#ifndef MYCYLINDER_RUNACTION_HH
#define MYCYLINDER_RUNACTION_HH

#include "G4UserRunAction.hh"
#include "G4String.hh"
#include "G4Types.hh"

#include <chrono>

class G4Run;
class EventAction;

/// Run-level action: creates a MyRun instance per thread, prints and saves
/// the final dose results at the end of the master run. Which fields are
/// emitted is controlled by OutputConfig (set via /myOutput/* commands).
class RunAction : public G4UserRunAction
{
public:
    explicit RunAction(EventAction* eventAction = nullptr);
    ~RunAction() override = default;

    G4Run* GenerateRun() override;
    void BeginOfRunAction(const G4Run* run) override;
    void EndOfRunAction(const G4Run* run) override;

private:
    static G4String FormatValue(G4double value, const G4String& unit);
    void PrintAndSaveResults(const G4Run* run) const;

    EventAction* fEventAction;  // unused in MT master, kept for compatibility

    // Wall-clock timer, recorded on the master thread only.
    std::chrono::steady_clock::time_point fStartTime;
    G4bool fTimerActive = false;
};

#endif
