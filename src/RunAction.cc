#include "RunAction.hh"
#include "EventAction.hh"
#include "MyRun.hh"
#include "SourceConfig.hh"

#include "DetectorConfig.hh"
#include "BetaSpectrumConfig.hh"

#include "G4Material.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <utility>

RunAction::RunAction(EventAction* eventAction)
    : fEventAction(eventAction)
{
    G4RunManager::GetRunManager()->SetPrintProgress(1000);
}

G4Run* RunAction::GenerateRun()
{
    return new MyRun();
}

void RunAction::BeginOfRunAction(const G4Run*)
{
    G4RunManager::GetRunManager()->SetRandomNumberStore(false);

    // Start the wall-clock timer on the master thread only.
    auto rmType = G4RunManager::GetRunManager()->GetRunManagerType();
    if (rmType != G4RunManager::workerRM &&
        rmType != G4RunManager::subEventWorkerRM) {
        fStartTime = std::chrono::steady_clock::now();
        fTimerActive = true;
    } else {
        fTimerActive = false;
    }
}

G4String RunAction::FormatValue(G4double value, const G4String& unit)
{
    const G4double absValue = std::abs(value);
    std::ostringstream stream;
    const G4bool useScientific =
        absValue > 0.0 && (absValue < 1.0e-3 || absValue >= 1.0e4);

    if (useScientific) {
        stream << std::scientific << std::setprecision(4) << value;
    } else {
        stream << std::fixed << std::setprecision(4) << value;
    }
    return stream.str() + " " + unit;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    if (run->GetNumberOfEvent() == 0) return;

    auto rmType = G4RunManager::GetRunManager()->GetRunManagerType();
    if (rmType == G4RunManager::workerRM ||
        rmType == G4RunManager::subEventWorkerRM) {
        return;
    }

    // Post-run sanity check: if events ran but no secondaries were produced
    // (zero energy deposit AND zero decay energy), the ion most likely did
    // not decay. This happens when:
    //   - the nuclide is stable (or treated as stable by Geant4)
    //   - G4RADIOACTIVEDATA is not installed / not in PATH
    //   - the half-life exceeds thresholdForVeryLongDecayTime
    // We cannot detect this upfront (G4RadioactiveDecay uses external data
    // files, not the particle's DecayTable), so we warn here instead.
    const auto* myRun = static_cast<const MyRun*>(run);
    const G4int nofEvents = run->GetNumberOfEvent();
    if (myRun->GetEdep() <= 0.0 && myRun->GetDecayEnergy() <= 0.0) {
        auto& src = SourceConfig::Instance();
        std::ostringstream warn;
        warn << "WARNING: " << nofEvents << " events were simulated but NO SECONDARIES were produced.\n"
             << "The ion " << src.GetRadionuclideName()
             << " (Z=" << src.GetZ() << ", A=" << src.GetA() << ")"
             << " most likely did not decay.\n\n"
             << "Possible causes:\n"
             << "  - This nuclide is stable or treated as stable by Geant4.\n"
             << "  - G4RADIOACTIVEDATA environment variable is not set or the data\n"
             << "    files for this nuclide are missing.\n"
             << "  - The half-life exceeds /process/had/rdm/thresholdForVeryLongDecayTime.\n\n"
             << "Check the Geant4 data installation and try a different radionuclide.";
        G4String warnMsg = warn.str();
        G4cout << "\n" << warnMsg << "\n" << G4endl;

        // Also append to results.txt
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm tm_local{};
#if defined(_WIN32)
        localtime_s(&tm_local, &now_c);
#else
        localtime_r(&now_c, &tm_local);
#endif
        char ts[32];
        std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_local);
        std::ofstream outFile("run_result.txt", std::ios::out | std::ios::app);
        if (outFile.is_open()) {
            outFile << "=== WARNING @ " << ts << " ===\n";
            outFile << warnMsg << "\n\n";
            outFile.close();
        }
    }

    PrintAndSaveResults(run);
    WriteBetaSpectra(run);
}

void RunAction::PrintAndSaveResults(const G4Run* run) const
{
    const auto* myRun = static_cast<const MyRun*>(run);
    const G4int    nofEvents     = run->GetNumberOfEvent();
    const G4double edep          = myRun->GetEdep();
    const G4double decayEnergy   = myRun->GetDecayEnergy();
    const G4double escapedEnergy = myRun->GetEscapedEnergy();
    const G4double transferred   = myRun->GetTransferredEnergy();
    const G4double diff          = edep - transferred;

    auto& cfg = DetectorConfig::Instance();
    const G4double volume     = cfg.LiquidVolumeCm3() * cm3;
    const G4double density    = cfg.GetEffectiveDensity();
    const G4double mass       = volume * density;

    const G4double doseA = (mass > 0.0) ? edep        / mass : 0.0;
    const G4double doseB = (mass > 0.0) ? transferred / mass : 0.0;

    double wallSeconds = 0.0;
    if (fTimerActive) {
        auto elapsed = std::chrono::steady_clock::now() - fStartTime;
        wallSeconds = std::chrono::duration<double>(elapsed).count();
    }
    const double avgPerEventMs = (nofEvents > 0) ? (wallSeconds * 1000.0 / nofEvents) : 0.0;
    const G4double energyPerDecay = (nofEvents > 0) ? (edep / nofEvents) : 0.0;

    std::ofstream outFile("run_result.txt", std::ios::out | std::ios::trunc);
    if (outFile.is_open()) {
        outFile << "Number of decays:    " << nofEvents << "\n";
        outFile << "Energy deposit (A):  " << FormatValue(edep / MeV, "MeV") << "\n";
        outFile << "Absorbed dose (A):   " << FormatValue(doseA / gray, "Gy") << "\n";
        outFile << "Decay energy (B):    " << FormatValue(decayEnergy / MeV, "MeV") << "\n";
        outFile << "Escaped energy (B):  " << FormatValue(escapedEnergy / MeV, "MeV") << "\n";
        outFile << "Transferred (B):     " << FormatValue(transferred / MeV, "MeV") << "\n";
        outFile << "Absorbed dose (B):   " << FormatValue(doseB / gray, "Gy") << "\n";
        outFile << "Cross-check (A-B):   " << FormatValue(diff / MeV, "MeV") << "\n";
        outFile << "Energy per decay:    " << FormatValue(energyPerDecay / MeV, "MeV") << "\n";
        outFile << "Wall time:           " << FormatValue(wallSeconds, "s") << "\n";
        outFile << "Avg time per event:  " << FormatValue(avgPerEventMs, "ms") << "\n";
        G4int nThreads = 1;
        if (auto* mt = G4MTRunManager::GetMasterRunManager()) nThreads = mt->GetNumberOfThreads();
        outFile << "Number of threads:   " << nThreads << "\n";
        outFile.close();
    }

    G4cout << "\n========================================" << G4endl;
    G4cout << "  SIMULATION RESULTS"                     << G4endl;
    G4cout << "========================================" << G4endl;
    G4cout << "Number of decays:    " << nofEvents << G4endl;
    G4cout << "Energy deposit (A):  " << FormatValue(edep / MeV, "MeV") << G4endl;
    G4cout << "Absorbed dose (A):   " << FormatValue(doseA / gray, "Gy") << G4endl;
    G4cout << "Cross-check (A-B):   " << FormatValue(diff / MeV, "MeV") << G4endl;
    G4cout << "========================================\n" << G4endl;
}

void RunAction::WriteBetaSpectra(const G4Run* run) const
{
    if (!BetaSpectrumConfig::Instance().IsEnabled()) return;
    const auto* myRun = static_cast<const MyRun*>(run);
    if (!myRun) return;

    auto& spec = BetaSpectrumConfig::Instance();
    auto& src = SourceConfig::Instance();
    const G4int nofEvents = run->GetNumberOfEvent();

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_local{};
#if defined(_WIN32)
    localtime_s(&tm_local, &now_c);
#else
    localtime_r(&now_c, &tm_local);
#endif
    char ts[32];
    std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_local);

    const G4String nuclideInfo =
        src.GetRadionuclideName() +
        " (Z=" + std::to_string(src.GetZ()) +
        ", A=" + std::to_string(src.GetA()) + ")";

    auto writeOne = [&](const G4String& path, const G4String& title,
                        const std::vector<G4double>& values) {
        std::ofstream f(path.c_str(), std::ios::out | std::ios::trunc);
        if (!f.is_open()) {
            G4cerr << "Beta spectrum: cannot open " << path << " for writing."
                   << G4endl;
            return;
        }
        f << "# " << title << " (birth kinetic energy, MeV, one per line)\n";
        f << "# Radionuclide: " << nuclideInfo << "\n";
        f << "# Number of events: " << nofEvents << "\n";
        f << "# Low-energy cut: ";
        if (spec.GetAtomicCut() > 0.0) {
            f << spec.GetAtomicCut() / keV
              << " keV (atomic electrons below are skipped)\n";
        } else {
            f << "off (all decay electrons recorded)\n";
        }
        f << "# Mode: "
          << (spec.IsPrimaryOnly() ? "primary ion only" : "full decay chain")
          << "\n";
        f << "# Date: " << ts << "\n";
        f << std::fixed << std::setprecision(6);
        for (G4double e : values) {
            f << e << "\n";
        }
        f.close();
    };

    writeOne(spec.GetFileMinus(), "Beta- spectrum", myRun->GetBetaMinus());
    writeOne(spec.GetFilePlus(), "Beta+ spectrum", myRun->GetBetaPlus());

    G4cout << "Beta spectrum: " << myRun->GetBetaMinus().size()
           << " beta- -> " << spec.GetFileMinus() << ", "
           << myRun->GetBetaPlus().size()
           << " beta+ -> " << spec.GetFilePlus() << G4endl;

    std::ofstream outFile("run_result.txt", std::ios::out | std::ios::app);
    if (outFile.is_open()) {
        outFile << "Beta- entries: " << myRun->GetBetaMinus().size()
                << " -> " << spec.GetFileMinus() << "\n";
        outFile << "Beta+ entries: " << myRun->GetBetaPlus().size()
                << " -> " << spec.GetFilePlus() << "\n\n";
        outFile.close();
    }
}
