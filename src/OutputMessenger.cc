#include "OutputMessenger.hh"
#include "OutputConfig.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4ios.hh"

OutputMessenger::OutputMessenger()
{
    fDir = new G4UIdirectory("/myOutput/");
    fDir->SetGuidance("Select which result fields are printed and saved at end of run");

    fEnableCmd = new G4UIcmdWithAString("/myOutput/enable", this);
    fEnableCmd->SetGuidance("Enable a specific output field by id");
    fEnableCmd->SetGuidance("Example: /myOutput/enable edep_dose");
    fEnableCmd->SetParameterName("field", false);
    fEnableCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fEnableCmd->SetToBeBroadcasted(false);

    fDisableCmd = new G4UIcmdWithAString("/myOutput/disable", this);
    fDisableCmd->SetGuidance("Disable a specific output field by id");
    fDisableCmd->SetGuidance("Example: /myOutput/disable num_threads");
    fDisableCmd->SetParameterName("field", false);
    fDisableCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fDisableCmd->SetToBeBroadcasted(false);

    fEnableAllCmd = new G4UIcmdWithoutParameter("/myOutput/enableAll", this);
    fEnableAllCmd->SetGuidance("Enable all output fields (default state)");
    fEnableAllCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fEnableAllCmd->SetToBeBroadcasted(false);

    fDisableAllCmd = new G4UIcmdWithoutParameter("/myOutput/disableAll", this);
    fDisableAllCmd->SetGuidance("Disable all output fields (use then /myOutput/enable to opt in)");
    fDisableAllCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fDisableAllCmd->SetToBeBroadcasted(false);
}

OutputMessenger::~OutputMessenger()
{
    delete fEnableCmd;
    delete fDisableCmd;
    delete fEnableAllCmd;
    delete fDisableAllCmd;
    delete fDir;
}

void OutputMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    auto& cfg = OutputConfig::Instance();

    if (command == fEnableCmd) {
        OutputConfig::Field f;
        if (OutputConfig::FieldFromName(newValue, f)) {
            cfg.Enable(f);
            G4cout << "Output field enabled: " << newValue << G4endl;
        } else {
            G4cerr << "Unknown output field: " << newValue << G4endl;
        }
    }
    else if (command == fDisableCmd) {
        OutputConfig::Field f;
        if (OutputConfig::FieldFromName(newValue, f)) {
            cfg.Disable(f);
            G4cout << "Output field disabled: " << newValue << G4endl;
        } else {
            G4cerr << "Unknown output field: " << newValue << G4endl;
        }
    }
    else if (command == fEnableAllCmd) {
        cfg.EnableAll();
        G4cout << "All output fields enabled." << G4endl;
    }
    else if (command == fDisableAllCmd) {
        cfg.DisableAll();
        G4cout << "All output fields disabled. Use /myOutput/enable <id> to opt in." << G4endl;
    }
}
