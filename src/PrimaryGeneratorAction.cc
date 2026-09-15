#include "PrimaryGeneratorAction.hh"
#include "SourceConfig.hh"
#include "DetectorConfig.hh"

#include "G4IonTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "G4ios.hh"
#include "G4Exception.hh"

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <atomic>

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : fParticleGun(new G4ParticleGun(1)),
      fIonDefinition(nullptr)
{}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

namespace {
/// Writes an error message to results.txt with a timestamp, so the user
/// can see why the program aborted even if the console was closed.
void WriteErrorToResults(const G4String& message)
{
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
        outFile << "=== ERROR @ " << ts << " ===\n";
        outFile << message << "\n\n";
        outFile.close();
    }
}

// In MT mode each worker thread has its own PrimaryGeneratorAction instance,
// so fIonDefinition is cached per-thread. The "ion not found" check below
// must only fire ONCE across all threads, otherwise the error message and
// the G4Exception call would be duplicated N times. This atomic flag
// guarantees that only the first thread to reach the check proceeds.
std::atomic<G4bool> gIonCheckDone{false};
} // namespace

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // Build the ion definition once, using Z, A from SourceConfig (set
    // via /mySource/setRadionuclide before /run/initialize). Caching is
    // safe because SourceConfig is read-only during the run.
    if (!fIonDefinition) {
        const G4int    Z   = SourceConfig::Instance().GetZ();
        const G4int    A   = SourceConfig::Instance().GetA();
        const G4double exc = SourceConfig::Instance().GetExcitation();
        fIonDefinition = G4IonTable::GetIonTable()->GetIon(Z, A, exc);

        // GetIon() returns nullptr only for completely invalid Z,A
        // combinations (e.g. Z=200, A=500). For most "non-existent"
        // nuclides it actually returns a generic ion placeholder, which
        // we cannot distinguish from a real nuclide at this point —
        // G4RadioactiveDecay uses external data files, not the particle's
        // DecayTable, so there is no reliable upfront way to check
        // whether decay data exists. The "ion exists but never decays"
        // case is detected post-run in RunAction (zero secondaries).
        if (!fIonDefinition) {
            // Only the first thread to reach here writes the error and
            // raises the fatal exception. Other threads skip silently.
            G4bool expected = false;
            if (gIonCheckDone.compare_exchange_strong(expected, true)) {
                std::ostringstream err;
                err << "FATAL: Ion with Z=" << Z << ", A=" << A
                    << ", excitation=" << exc / keV << " keV"
                    << " was not found in the Geant4 ion database.\n"
                    << "Possible causes:\n"
                    << "  - This Z,A combination does not correspond to a known nuclide.\n"
                    << "  - The Geant4 particle data files are not installed.\n"
                    << "Please choose a different radionuclide.";
                G4String errMsg = err.str();
                G4cerr << errMsg << G4endl;
                WriteErrorToResults(errMsg);
                G4Exception(
                    "PrimaryGeneratorAction::GeneratePrimaries",
                    "ION_NOT_FOUND",
                    FatalException,
                    "Requested ion is not in the Geant4 database. See results.txt.");
            }
            return;
        }

        // Force the ion to be unstable. Some long-lived ions (e.g. Co-60
        // with T1/2 = 5.27 y) are marked as stable in G4IonTable by
        // default and would never decay during the simulation. Marking
        // them unstable ensures G4RadioactiveDecay applies its decay
        // scheme. The thresholdForVeryLongDecayTime command in main.cc
        // lifts the half-life threshold so that long-lived nuclides
        // actually decay.
        fIonDefinition->SetPDGStable(false);

        G4cout << "Ion created: " << fIonDefinition->GetParticleName()
               << " (forced unstable)" << G4endl;
        fParticleGun->SetParticleDefinition(fIonDefinition);
        fParticleGun->SetParticleCharge(0.0);
        fParticleGun->SetParticleEnergy(0.0);
    }

    // Sample the primary uniformly over the whole liquid (sensitive)
    // volume. The liquid always keeps the logical volume name "Cylinder"
    // regardless of the geometry variant; for the "tube" variant its solid
    // is a union of a cylinder and a hemisphere. Rejection sampling over
    // the solid's bounding box works for every variant. The bounding box
    // and Inside() check are in the container construction frame; the
    // world position is obtained by applying the container rotation.
    G4double x0 = 0.0, y0 = 0.0, z0 = 0.0;   // fallback: axis centre
    if (G4LogicalVolume* liqLV = G4LogicalVolumeStore::GetInstance()->GetVolume("Cylinder")) {
        G4ThreeVector pMin, pMax;
        liqLV->GetSolid()->BoundingLimits(pMin, pMax);
        z0 = (pMin.z() + pMax.z()) / 2.0;
        for (int i = 0; i < 10000; ++i) {
            const G4double x = pMin.x() + (pMax.x() - pMin.x()) * G4UniformRand();
            const G4double y = pMin.y() + (pMax.y() - pMin.y()) * G4UniformRand();
            const G4double z = pMin.z() + (pMax.z() - pMin.z()) * G4UniformRand();
            if (liqLV->GetSolid()->Inside(G4ThreeVector(x, y, z)) != kOutside) {
                x0 = x; y0 = y; z0 = z;
                break;
            }
        }
    }

    fParticleGun->SetParticlePosition(
        DetectorConfig::ContainerLocalToWorld(G4ThreeVector(x0, y0, z0)));
    fParticleGun->SetParticleTime(0.0);
    fParticleGun->GeneratePrimaryVertex(event);
}
