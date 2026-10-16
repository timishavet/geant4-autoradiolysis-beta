#include "SteppingAction.hh"
#include "EventAction.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"

// Path B boundary flow: net outflow across the liquid boundary.
// See SteppingAction.hh for the full rationale.
SteppingAction::SteppingAction(EventAction* eventAction)
    : fEventAction(eventAction)
{}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!fEventAction || !step) return;

    // Resolve the liquid logical volume once (geometry exists by the time
    // the first step is processed; each worker thread has its own action
    // instance, so caching the pointer in a member is thread-safe).
    if (!fLiquidLV) {
        fLiquidLV = G4LogicalVolumeStore::GetInstance()->GetVolume("Cylinder");
        if (!fLiquidLV) return;
    }

    const G4StepPoint* prePoint  = step->GetPreStepPoint();
    const G4StepPoint* postPoint = step->GetPostStepPoint();
    if (!prePoint || !postPoint) return;

    const G4VPhysicalVolume* prePV  = prePoint->GetPhysicalVolume();
    const G4VPhysicalVolume* postPV = postPoint->GetPhysicalVolume();
    const G4LogicalVolume* preLV  = prePV  ? prePV->GetLogicalVolume()  : nullptr;
    const G4LogicalVolume* postLV = postPV ? postPV->GetLogicalVolume() : nullptr;
    if (preLV == postLV) return;

    const G4bool preIn  = (preLV  == fLiquidLV);
    const G4bool postIn = (postLV == fLiquidLV);
    if (preIn == postIn) return;

    // Energy at the crossing point (see class documentation).
    const G4double energy = step->GetTrack()->GetKineticEnergy();
    if (energy <= 0.0) return;

    if (preIn) {
        fEventAction->AddEscapedEnergy(energy);  // outflow
    } else {
        fEventAction->AddInflowEnergy(energy);   // returning track
    }
}