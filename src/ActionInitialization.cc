#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "SteppingAction.hh"

void ActionInitialization::BuildForMaster() const
{
    // In MT mode the master thread only needs RunAction for final output.
    // No event/stepping/tracking actions on the master — they are per-worker.
    SetUserAction(new RunAction(nullptr));
}

void ActionInitialization::Build() const
{
    // Per-worker actions. Each worker gets its own EventAction etc.
    auto* eventAction    = new EventAction;
    auto* trackingAction = new TrackingAction(eventAction);
    auto* steppingAction = new SteppingAction(eventAction);

    SetUserAction(new PrimaryGeneratorAction);
    SetUserAction(eventAction);
    SetUserAction(trackingAction);
    SetUserAction(steppingAction);
    SetUserAction(new RunAction(eventAction));
}
