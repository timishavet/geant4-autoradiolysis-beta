#ifndef MYCYLINDER_TRACKINGACTION_HH
#define MYCYLINDER_TRACKINGACTION_HH

#include "G4UserTrackingAction.hh"

class G4Track;
class EventAction;

/// Per-track action implementing the two Path-B bookkeeping rules:
///  - PreUserTrackingAction: energy IN. Counts the kinetic energy only of
///    tracks that inject NEW energy into the cylinder - decay products
///    (creator process RadioactiveDecay) and positron-annihilation gammas
///    (creator process annihil). Cascade secondaries are excluded: their
///    energy is a re-cycle of already-counted energy.
///  - PostUserTrackingAction: energy OUT. Counts the remaining kinetic
///    energy of tracks that terminate OUTSIDE the cylinder (final leakage:
///    energy that has left the material and cannot be deposited in it any
///    more). This replaces the old "first boundary crossing" rule.
class TrackingAction : public G4UserTrackingAction
{
public:
    explicit TrackingAction(EventAction* eventAction);
    ~TrackingAction() override = default;

    void PreUserTrackingAction(const G4Track*) override;
    void PostUserTrackingAction(const G4Track*) override;

private:
    EventAction* fEventAction;
};

#endif
