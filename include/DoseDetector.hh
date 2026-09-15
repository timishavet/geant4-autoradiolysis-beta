#ifndef MYCYLINDER_DOSEDETECTOR_HH
#define MYCYLINDER_DOSEDETECTOR_HH

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4HCofThisEvent;

/// Sensitive detector for the water cylinder.
///
/// Implements Path A of energy accounting: sums the total energy deposited
/// inside the cylinder by all particles. The result is stored in the
/// per-run MyRun instance, which Geant4 automatically merges across worker
/// threads at the end of the run.
class DoseDetector : public G4VSensitiveDetector
{
public:
    explicit DoseDetector(const G4String& name);
    ~DoseDetector() override = default;

    void Initialize(G4HCofThisEvent*) override {}
    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
    void EndOfEvent(G4HCofThisEvent*) override {}
};

#endif
