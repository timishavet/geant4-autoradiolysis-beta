#ifndef MYCYLINDER_ACTIONINITIALIZATION_HH
#define MYCYLINDER_ACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"

/// Action initialisation: builds per-thread user actions.
/// In MT mode Build() is called once per worker thread, BuildForMaster()
/// once for the master thread (which only needs RunAction for final output).
class ActionInitialization : public G4VUserActionInitialization
{
public:
    ActionInitialization() = default;
    ~ActionInitialization() override = default;

    void BuildForMaster() const override;
    void Build() const override;
};

#endif
