#include "EventAction.hh"
#include "MyRun.hh"

#include "G4Event.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"

EventAction::EventAction()
    : fDecayEnergy(0.0),
      fBoundaryLeakage(0.0),
      fInflowEnergy(0.0)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fDecayEnergy     = 0.0;
    fBoundaryLeakage = 0.0;
    fInflowEnergy    = 0.0;
    fBetaMinus.clear();
    fBetaPlus.clear();
}

void EventAction::EndOfEventAction(const G4Event*)
{
    // Push per-event totals into the per-run accumulator (thread-local MyRun).
    auto* run = static_cast<MyRun*>(
        const_cast<G4Run*>(G4RunManager::GetRunManager()->GetCurrentRun()));
    if (!run) return;

    run->AddDecayEnergy(fDecayEnergy);
    run->AddEscapedEnergy(fBoundaryLeakage);
    run->AddInflowEnergy(fInflowEnergy);
    for (G4double e : fBetaMinus) run->AddBetaMinus(e);
    for (G4double e : fBetaPlus)  run->AddBetaPlus(e);
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

void EventAction::AddInflowEnergy(G4double energy)
{
    if (energy > 0.0) {
        fInflowEnergy += energy;
    }
}

void EventAction::AddBetaMinus(G4double eMeV)
{
    if (eMeV > 0.0) {
        fBetaMinus.push_back(eMeV);
    }
}

void EventAction::AddBetaPlus(G4double eMeV)
{
    if (eMeV > 0.0) {
        fBetaPlus.push_back(eMeV);
    }
}
