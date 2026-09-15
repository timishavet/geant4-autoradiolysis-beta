#include "DoseDetector.hh"
#include "MyRun.hh"

#include "G4Step.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

DoseDetector::DoseDetector(const G4String& name)
    : G4VSensitiveDetector(name)
{
    collectionName.insert("DoseCollection");
}

G4bool DoseDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    const G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.0) return false;

    // Path A accumulator: write into the per-run MyRun instance.
    // In MT mode this is thread-local, so no synchronisation is needed.
    auto* run = static_cast<MyRun*>(
        const_cast<G4Run*>(G4RunManager::GetRunManager()->GetCurrentRun()));
    if (run) {
        run->AddEdep(edep);
    }
    return true;
}
