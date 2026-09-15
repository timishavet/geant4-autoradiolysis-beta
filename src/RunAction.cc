#include "RunAction.hh"
#include "EventAction.hh"
#include "MyRun.hh"
#include "SourceConfig.hh"
#include "OutputConfig.hh"
#include "DetectorConfig.hh"

#include "G4Material.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
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
        std::ofstream outFile("results.txt", std::ios::out | std::ios::app);
        if (outFile.is_open()) {
            outFile << "=== WARNING @ " << ts << " ===\n";
            outFile << warnMsg << "\n\n";
            outFile.close();
        }
    }

    PrintAndSaveResults(run);
}

void RunAction::PrintAndSaveResults(const G4Run* run) const
{
    using Field = OutputConfig::Field;
    auto& out = OutputConfig::Instance();

    const auto* myRun = static_cast<const MyRun*>(run);
    const G4int    nofEvents     = run->GetNumberOfEvent();
    const G4double edep          = myRun->GetEdep();
    const G4double decayEnergy   = myRun->GetDecayEnergy();
    const G4double escapedEnergy = myRun->GetEscapedEnergy();
    const G4double transferred   = myRun->GetTransferredEnergy();
    const G4double diff          = edep - transferred;

    // Resolve geometry and material from DetectorConfig (single source of
    // truth; the liquid solid may be a union, so reading it back from the
    // logical volume store is unreliable).
    auto& cfg = DetectorConfig::Instance();
    const G4double radius     = cfg.GetRadius();
    const G4double halfHeight = cfg.GetHalfHeight();
    const G4double volume     = cfg.LiquidVolumeCm3() * cm3;
    const G4double density    = cfg.GetEffectiveDensity();
    const G4double mass       = volume * density;
    const G4String materialName = cfg.GetMaterialName();

    const G4double doseA = (mass > 0.0) ? edep        / mass : 0.0;
    const G4double doseB = (mass > 0.0) ? transferred / mass : 0.0;

    // Derived timing and per-decay values.
    double wallSeconds = 0.0;
    if (fTimerActive) {
        auto elapsed = std::chrono::steady_clock::now() - fStartTime;
        wallSeconds = std::chrono::duration<double>(elapsed).count();
    }
    const double avgPerEventMs = (nofEvents > 0) ? (wallSeconds * 1000.0 / nofEvents) : 0.0;
    const G4double energyPerDecay = (nofEvents > 0) ? (edep / nofEvents) : 0.0;

    // Source configuration.
    auto& src = SourceConfig::Instance();
    const G4int    srcZ   = src.GetZ();
    const G4int    srcA   = src.GetA();
    const G4String srcName = src.GetRadionuclideName();
    const G4double activityBq = src.GetActivity();

    // Timestamp.
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

    // Build the list of (label, formatted_value) pairs, filtered by
    // OutputConfig. Each field appears here exactly once.
    std::vector<std::pair<G4String, G4String>> lines;
    auto add = [&](Field f, const G4String& label, const G4String& value) {
        if (out.IsEnabled(f)) lines.emplace_back(label, value);
    };

    add(Field::Timestamp,         "Date/time:           ", G4String(ts));
    add(Field::Radionuclide,      "Radionuclide:        ",
        srcName + " (Z=" + std::to_string(srcZ) + ", A=" + std::to_string(srcA) + ")");
    add(Field::NumEvents,         "Number of decays:    ", std::to_string(nofEvents));
    add(Field::Activity,          "Source activity:     ",
        (activityBq > 0.0 ? FormatValue(activityBq / CLHEP::becquerel, "Bq") : G4String("not set")));
    add(Field::GeometryVariant,   "Geometry variant:    ", cfg.GetGeometryName());
    add(Field::Dimensions,        "Cylinder radius:     ", FormatValue(radius / cm, "cm"));
    add(Field::Dimensions,        "Cylinder height:     ", FormatValue((2.0 * halfHeight) / cm, "cm"));
    add(Field::Volume,            "Volume:              ", FormatValue(volume / cm3, "cm3"));
    add(Field::MaterialName,      "Material:            ", materialName);
    add(Field::MaterialDensity,   "Material density:    ", FormatValue(density / (g / cm3), "g/cm3"));
    add(Field::Mass,              "Material mass:       ", FormatValue(mass / g, "g"));
    if (cfg.GetGeometryType() != DetectorConfig::GeometryType::Cylinder) {
        add(Field::WallThickness, "Glass wall:          ", FormatValue(cfg.GetGlassWall() / cm, "cm"));
        add(Field::Rim,           "Dry rim above:       ", FormatValue(cfg.GetRim() / cm, "cm"));
        add(Field::GlassMaterial, "Glass material:      ",
            cfg.GetGlassMaterialName() + " (" +
            FormatValue(cfg.GetGlassEffectiveDensity() / (g / cm3), "g/cm3") + ")");
    }
    add(Field::EdepEnergy,        "Energy deposit (A):  ", FormatValue(edep / MeV, "MeV"));
    add(Field::EdepDose,          "Absorbed dose (A):   ", FormatValue(doseA / gray, "Gy"));
    add(Field::DecayEnergy,       "Decay energy (B):    ", FormatValue(decayEnergy / MeV, "MeV"));
    add(Field::EscapedEnergy,     "Escaped energy (B):  ", FormatValue(escapedEnergy / MeV, "MeV"));
    add(Field::TransferredEnergy, "Transferred (B):     ", FormatValue(transferred / MeV, "MeV"));
    add(Field::PathBDose,         "Absorbed dose (B):   ", FormatValue(doseB / gray, "Gy"));
    add(Field::CrossCheck,        "Cross-check (A-B):   ", FormatValue(diff / MeV, "MeV"));
    add(Field::EnergyPerDecay,    "Energy per decay:    ", FormatValue(energyPerDecay / MeV, "MeV"));
    add(Field::ThreadingMode,     "Threading mode:      ",
        G4String(out.IsMT() ? "Multi-threaded" : "Single-threaded"));
    add(Field::NumThreads,        "Number of threads:   ", std::to_string(out.GetNumThreads()));
    add(Field::SimWallTime,       "Wall time:           ", FormatValue(wallSeconds, "s"));
    add(Field::AvgTimePerEvent,   "Avg time per event:  ", FormatValue(avgPerEventMs, "ms"));

    // ----- Console output -----
    G4cout << "\n========================================" << G4endl;
    G4cout << "  SIMULATION RESULTS"                     << G4endl;
    G4cout << "========================================" << G4endl;
    for (const auto& [label, value] : lines) {
        G4cout << label << value << G4endl;
    }
    G4cout << "========================================\n" << G4endl;

    // ----- Append to results.txt with timestamp -----
    std::ofstream outFile("results.txt", std::ios::out | std::ios::app);
    if (outFile.is_open()) {
        outFile << "=== Run @ " << ts << " ===\n";
        for (const auto& [label, value] : lines) {
            outFile << label << value << "\n";
        }
        outFile << "\n";
        outFile.close();
    }
}
