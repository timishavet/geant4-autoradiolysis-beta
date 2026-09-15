#include "TrackingAction.hh"
#include "EventAction.hh"
#include "SourceConfig.hh"
#include "DetectorConfig.hh"

#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4ParticleDefinition.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"
#include "G4TouchableHandle.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"

TrackingAction::TrackingAction(EventAction* eventAction)
    : fEventAction(eventAction)
{}

namespace {
/// Returns true if the given WORLD position lies inside the cylinder volume.
/// We resolve this by looking up the cylinder's logical volume and
/// checking its solid. The solid is defined in the container construction
/// frame, so the world position is first mapped back to that frame
/// (the whole container is rotated by DetectorConfig::ContainerRotation()).
/// This avoids relying on the touchable hierarchy of the freshly created
/// track (which may not be set up yet).
G4bool IsInsideCylinder(const G4ThreeVector& pos)
{
    G4LogicalVolume* cylLV = G4LogicalVolumeStore::GetInstance()->GetVolume("Cylinder");
    if (!cylLV) return false;
    return cylLV->GetSolid()->Inside(
        DetectorConfig::WorldToContainerLocal(pos));
}
} // namespace

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    if (!fEventAction || !track) return;

    // Skip the primary ion (parentID = 0). Its kinetic energy is set to
    // zero by PrimaryGeneratorAction anyway, so it has nothing to add.
    if (track->GetParentID() == 0) return;

    // Path B "in" (decay energy): count ONLY the tracks that inject NEW
    // energy into the material:
    //   - products of the radioactive decay itself (RadioactiveDecay):
    //     beta/positron, nuclear de-excitation gammas, internal-conversion
    //     and Auger electrons, characteristic x-rays
    //   - the two 511 keV gammas from positron annihilation (annihil),
    //     which for beta+ emitters are a second genuine injection
    //
    // Every other secondary (Compton recoil electrons, delta rays,
    // bremsstrahlung photons, scattered photons, pair electrons, ...) is
    // deliberately NOT counted: its kinetic energy is a re-cycle of the
    // already-counted energy of a parent track, so counting it would
    // double-book energy and inflate Path B above the true absorbed dose
    // (this was the cause of the large (A - B) discrepancy for gamma
    // emitters like Co-60). With this restriction:
    //     DecayEnergy = E_injected  (true new energy entering the system)
    const G4VProcess* creator = track->GetCreatorProcess();
    if (!creator) return;
    const G4String& processName = creator->GetProcessName();
    if (processName != "RadioactiveDecay" && processName != "annihil") return;

    // Only energy injected inside the cylinder belongs to Path B.
    if (!IsInsideCylinder(track->GetPosition())) return;

    fEventAction->AddDecayEnergy(track->GetKineticEnergy());
}

void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
    if (!fEventAction || !track) return;

    // Path B "out" (final leakage): if a track terminates OUTSIDE the
    // cylinder, its remaining kinetic energy will never be deposited in
    // the material - it is the energy that has left the volume for good.
    // Track termination is the moment when the energy of a lineage stops
    // being redistributed into new daughter tracks, so counting it here
    // (once per terminating track, no "first exit" bookkeeping) yields a
    // clean single subtraction of the true leakage.
    //
    // Tracks that terminate INSIDE the cylinder are not counted: their
    // remaining energy is either deposited continuously (charged particles
    // at rest) or converted into daughter tracks that will be accounted
    // later, so keeping it out of "escaped" preserves the balance.
    //
    // Note: if a track scatters in the outside world (G4_AIR) and creates
    // daughters there, each in-air daughter terminating outside would be
    // counted too. In this application the world is low-density air and
    // the cylinder is the only dense material, so in-air generation of
    // secondaries is negligible and does not break the balance.
    if (!IsInsideCylinder(track->GetPosition())) {
        fEventAction->AddEscapedEnergy(track->GetKineticEnergy());
    }
}