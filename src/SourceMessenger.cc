#include "SourceMessenger.hh"
#include "SourceConfig.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIparameter.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <sstream>

SourceMessenger::SourceMessenger()
{
    fDir = new G4UIdirectory("/mySource/");
    fDir->SetGuidance("Commands to configure the radioactive source before /run/initialize");

    // /mySource/setRadionuclide Z A [excitation_keV]
    // Excitation is optional and defaults to 0 (ground state).
    fSetRadionuclideCmd = new G4UIcommand("/mySource/setRadionuclide", this);
    fSetRadionuclideCmd->SetGuidance("Set the radionuclide by Z, A and optional excitation energy (keV)");
    fSetRadionuclideCmd->SetGuidance("Example: /mySource/setRadionuclide 71 177        # Lu-177");
    fSetRadionuclideCmd->SetGuidance("          /mySource/setRadionuclide 43 99        # Tc-99m");

    auto* pZ = new G4UIparameter("Z", 'i', false);
    pZ->SetParameterRange("Z > 0 && Z < 120");
    fSetRadionuclideCmd->SetParameter(pZ);

    auto* pA = new G4UIparameter("A", 'i', false);
    pA->SetParameterRange("A > 0");
    fSetRadionuclideCmd->SetParameter(pA);

    auto* pExc = new G4UIparameter("Excitation_keV", 'd', true);
    pExc->SetDefaultValue(0.0);
    fSetRadionuclideCmd->SetParameter(pExc);

    fSetRadionuclideCmd->AvailableForStates(G4State_PreInit);
    fSetRadionuclideCmd->SetToBeBroadcasted(false);

    // /mySource/setActivity <value> <unit>  (Bq, kBq, MBq, Ci, ...)
    // Informational only: recorded in results output so the user can
    // trace which activity was assumed when computing N = A * t externally.
    fSetActivityCmd = new G4UIcmdWithADoubleAndUnit("/mySource/setActivity", this);
    fSetActivityCmd->SetGuidance("Set source activity (informational; recorded in results)");
    fSetActivityCmd->SetGuidance("Example: /mySource/setActivity 1e6 Bq");
    fSetActivityCmd->SetGuidance("          /mySource/setActivity 1 MBq");
    fSetActivityCmd->SetParameterName("activity", false);
    fSetActivityCmd->SetRange("activity >= 0.");
    fSetActivityCmd->SetUnitCategory("Activity");
    fSetActivityCmd->SetDefaultUnit("Bq");
    fSetActivityCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fSetActivityCmd->SetToBeBroadcasted(false);
}

SourceMessenger::~SourceMessenger()
{
    delete fSetRadionuclideCmd;
    delete fSetActivityCmd;
    delete fDir;
}

void SourceMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fSetRadionuclideCmd) {
        G4int Z = 0, A = 0;
        G4double exc = 0.0;
        std::istringstream iss(newValue);
        iss >> Z >> A >> exc;
        SourceConfig::Instance().SetRadionuclide(Z, A, exc * keV);
        G4cout << "Radionuclide set to Z=" << Z << " A=" << A
               << " (" << SourceConfig::Instance().GetRadionuclideName() << ")"
               << G4endl;
    }
    else if (command == fSetActivityCmd) {
        const G4double a = fSetActivityCmd->GetNewDoubleValue(newValue);
        SourceConfig::Instance().SetActivity(a);
        G4cout << "Source activity set to: " << a / CLHEP::becquerel << " Bq" << G4endl;
    }
}
