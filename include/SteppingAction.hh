#ifndef MYCYLINDER_STEPPINGACTION_HH
#define MYCYLINDER_STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"

class G4Step;
class G4LogicalVolume;
class EventAction;

/// Per-step action: boundary-flow bookkeeping for Path B.
///
/// For every step whose pre- and post-step points lie in different
/// logical volumes relative to the liquid ("Cylinder"):
///  - step leaving the liquid  -> outflow  += post-step kinetic energy;
///  - step entering the liquid -> inflow   += post-step kinetic energy.
///
/// The post-step kinetic energy is exactly the energy at the crossing
/// point: the Geant4 navigator limits every step at geometry boundaries,
/// so a boundary-crossing step ends ON the boundary and its whole energy
/// deposit lies on the starting side. A null physical volume (step ending
/// outside the world) counts as "outside".
///
/// Pointer comparison of logical volumes keeps this cheap (two pointer
/// fetches and two compares per step); the liquid volume pointer is
/// resolved lazily once per worker thread.
class SteppingAction : public G4UserSteppingAction
{
public:
    explicit SteppingAction(EventAction* eventAction);
    ~SteppingAction() override = default;

    void UserSteppingAction(const G4Step*) override;

private:
    EventAction*      fEventAction;
    G4LogicalVolume*  fLiquidLV = nullptr;  // resolved lazily, cached per thread
};

#endif
