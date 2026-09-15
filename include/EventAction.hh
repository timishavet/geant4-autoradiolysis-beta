#ifndef MYCYLINDER_EVENTACTION_HH
#define MYCYLINDER_EVENTACTION_HH

#include "G4Types.hh"
#include "G4UserEventAction.hh"

class G4Event;

/// Per-event action that accounts for decay energy and boundary leakage.
///
/// Two energy paths are tracked here:
///  - Path B "in"  (decay energy): kinetic energy of the particles that
///    carry NEW energy into the material volume - the products of the
///    radioactive decay itself (beta/positron, de-excitation gammas,
///    internal-conversion/Auger electrons) and, for beta+ emitters, the
///    two 511 keV annihilation gammas. Reported by TrackingAction at track
///    creation (creator process = RadioactiveDecay or annihil). In-volume
///    cascade secondaries (Compton recoils, delta rays, bremsstrahlung,
///    scattered photons) are NOT counted here: their kinetic energy is a
///    re-cycle of the parent's already-counted energy, not new injection.
///  - Path B "out" (boundary leakage): the remaining kinetic energy of a
///    track at the moment it terminates OUTSIDE the cylinder - i.e. energy
///    that left the material for good and will never be deposited in it.
///    Reported by TrackingAction at track end. This replaces the previous
///    "first boundary crossing" rule, which miscounted round-trip tracks.
///
/// With both rules, Path B becomes a true conservation balance on the
/// cylinder volume:
///     E_transferred = E_injected - E_final_leakage = E_deposited (Path A)
/// and the cross-check (A - B) reduces to negligible numerical noise.
class EventAction : public G4UserEventAction
{
public:
    EventAction();
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddDecayEnergy(G4double energy);
    void AddEscapedEnergy(G4double energy);

private:
    G4double fDecayEnergy;        // kinetic energy of energy-injecting particles this event
    G4double fBoundaryLeakage;    // kinetic energy of tracks terminating outside the cylinder
};

#endif
