#include "EventAction.hh"
#include "MyRun.hh"

#include "G4Event.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

EventAction::EventAction()
    : fDecayEnergy(0.0),
      fBoundaryLeakage(0.0)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fDecayEnergy     = 0.0;
    fBoundaryLeakage = 0.0;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    // Push per-event totals into the per-run accumulator (thread-local MyRun).
    auto* run = static_cast<MyRun*>(
        const_cast<G4Run*>(G4RunManager::GetRunManager()->GetCurrentRun()));
    if (!run) return;

    run->AddDecayEnergy(fDecayEnergy);
    run->AddEscapedEnergy(fBoundaryLeakage);
}

void EventAction::AddDecayEnergy(G4double energy)
{
    if (energy > 0.0) {
        fDecayEnergy += energy;
    }
}

void EventAction::AddEscapedEnergy(G4double energy)
{
    if (energy > 0.0) {
        fBoundaryLeakage += energy;
    }
}
