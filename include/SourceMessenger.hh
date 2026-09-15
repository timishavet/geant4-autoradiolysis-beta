#ifndef MYCYLINDER_SOURCEMESSENGER_HH
#define MYCYLINDER_SOURCEMESSENGER_HH

#include "G4UImessenger.hh"

class G4UIcommand;
class G4UIcmdWithADoubleAndUnit;
class G4UIdirectory;

/// /mySource/* UI commands for runtime source configuration before
/// /run/initialize: radionuclide (by Z, A, optional excitation) and
/// source activity (informational; recorded in results output).
class SourceMessenger : public G4UImessenger
{
public:
    SourceMessenger();
    ~SourceMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    G4UIdirectory*             fDir;
    G4UIcommand*               fSetRadionuclideCmd;  // Z A [excitation_keV]
    G4UIcmdWithADoubleAndUnit* fSetActivityCmd;
};

#endif
