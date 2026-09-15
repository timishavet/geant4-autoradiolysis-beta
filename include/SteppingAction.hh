#ifndef MYCYLINDER_STEPPINGACTION_HH
#define MYCYLINDER_STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"

class G4Step;
class EventAction;

/// Per-step action (currently empty).
///
/// Previously this action reported the kinetic energy of a track when it
/// crossed the cylinder boundary outward ("first exit" rule). That rule
/// was replaced by the final-leakage rule in
/// TrackingAction::PostUserTrackingAction (energy measured at the moment
/// the track terminates outside the cylinder). This class is kept only to
/// preserve the registered action list.
class SteppingAction : public G4UserSteppingAction
{
public:
    explicit SteppingAction(EventAction* eventAction);
    ~SteppingAction() override = default;

    void UserSteppingAction(const G4Step*) override;

private:
    EventAction* fEventAction;
};

#endif
