#ifndef MYCYLINDER_EVENTACTION_HH
#define MYCYLINDER_EVENTACTION_HH

#include "G4Types.hh"
#include "G4UserEventAction.hh"

#include <vector>

class G4Event;

/// Per-event action that accounts for decay energy and boundary flow.
///
/// Three energy quantities are tracked here (all per event):
///  - Path B "injected": kinetic energy of the particles that carry NEW
///    energy into the material volume - the products of the radioactive
///    decay itself (beta/positron, de-excitation gammas,
///    internal-conversion/Auger electrons) and, for beta+ emitters, the
///    two 511 keV annihilation gammas. Reported by TrackingAction at track
///    creation (creator process = RadioactiveDecay or annihil). In-volume
///    cascade secondaries (Compton recoils, delta rays, bremsstrahlung,
///    scattered photons) are NOT counted here: their kinetic energy is a
///    re-cycle of the parent's already-counted energy, not new injection.
///  - Path B "outflow": kinetic energy carried OUT of the cylinder across
///    its boundary, measured at the crossing point. Reported by
///    SteppingAction whenever a step leaves the cylinder volume.
///  - Path B "inflow": kinetic energy carried back INTO the cylinder by
///    returning tracks, measured at the crossing point. Reported by
///    SteppingAction whenever a step enters the cylinder volume.
///
/// With all three, Path B becomes an exact conservation balance on the
/// cylinder volume:
///     E_transferred = E_injected - E_outflow + E_inflow = E_deposited (Path A)
/// Energy dumped outside the cylinder (glass shell, air) cancels out by
/// construction: it is counted in the outflow at the exit crossing but
/// never returns, so it stays subtracted. Round-trip tracks contribute
/// their net loss (outflow minus inflow), i.e. exactly what they left
/// outside.
class EventAction : public G4UserEventAction
{
public:
    EventAction();
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddDecayEnergy(G4double energy);
    void AddEscapedEnergy(G4double energy);
    void AddInflowEnergy(G4double energy);

    // Per-track beta energies (MeV) recorded this event by TrackingAction.
    // Flushed into MyRun at EndOfEventAction.
    void AddBetaMinus(G4double eMeV);
    void AddBetaPlus(G4double eMeV);

private:
    G4double fDecayEnergy;        // kinetic energy of energy-injecting particles this event
    G4double fBoundaryLeakage;    // kinetic energy carried out across the boundary this event
    G4double fInflowEnergy;       // kinetic energy carried back in across the boundary this event
    std::vector<G4double> fBetaMinus;  // beta- birth energies this event (MeV)
    std::vector<G4double> fBetaPlus;   // beta+ birth energies this event (MeV)
};

#endif
