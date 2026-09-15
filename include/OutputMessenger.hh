#ifndef MYCYLINDER_OUTPUTMESSENGER_HH
#define MYCYLINDER_OUTPUTMESSENGER_HH

#include "G4UImessenger.hh"

class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithoutParameter;
class G4UIdirectory;

/// /myOutput/* UI commands for selecting which result fields to print and
/// save at the end of a run.
class OutputMessenger : public G4UImessenger
{
public:
    OutputMessenger();
    ~OutputMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    G4UIdirectory*           fDir;
    G4UIcmdWithAString*      fEnableCmd;
    G4UIcmdWithAString*      fDisableCmd;
    G4UIcmdWithoutParameter* fEnableAllCmd;
    G4UIcmdWithoutParameter* fDisableAllCmd;
};

#endif
