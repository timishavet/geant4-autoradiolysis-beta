#include "SteppingAction.hh"
#include "EventAction.hh"

#include "G4Step.hh"

// Note: with the new Path B bookkeeping, the boundary-leakage term is no
// longer captured at the moment a track crosses the cylinder boundary
// outward. The old "first crossing, once per track" rule miscounted
// round-trip particles and was replaced by the "track terminates outside
// the cylinder" rule implemented in TrackingAction::PostUserTrackingAction
// (see TrackingAction.cc). This stepping action is therefore intentionally
// empty; it is kept registered to avoid touching the geometry/tracking
// setup.
SteppingAction::SteppingAction(EventAction* eventAction)
    : fEventAction(eventAction)
{}

void SteppingAction::UserSteppingAction(const G4Step*)
{}