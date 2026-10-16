#include "SpectrumMessenger.hh"
#include "BetaSpectrumConfig.hh"

#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

SpectrumMessenger::SpectrumMessenger()
{
    fDir = new G4UIdirectory("/mySpectrum/");
    fDir->SetGuidance("Record per-decay beta energies for spectrum plots");

    fEnableCmd = new G4UIcmdWithoutParameter("/mySpectrum/enable", this);
    fEnableCmd->SetGuidance("Enable recording of beta- and beta+ birth energies");
    fEnableCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fEnableCmd->SetToBeBroadcasted(false);

    fDisableCmd = new G4UIcmdWithoutParameter("/mySpectrum/disable", this);
    fDisableCmd->SetGuidance("Disable recording of beta energies (default)");
    fDisableCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fDisableCmd->SetToBeBroadcasted(false);

    fFileMinusCmd = new G4UIcmdWithAString("/mySpectrum/setFileMinus", this);
    fFileMinusCmd->SetGuidance("Output file for beta- energies (one per line, MeV)");
    fFileMinusCmd->SetGuidance("Example: /mySpectrum/setFileMinus beta_minus.txt");
    fFileMinusCmd->SetParameterName("file", false);
    fFileMinusCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fFileMinusCmd->SetToBeBroadcasted(false);

    fFilePlusCmd = new G4UIcmdWithAString("/mySpectrum/setFilePlus", this);
    fFilePlusCmd->SetGuidance("Output file for beta+ energies (one per line, MeV)");
    fFilePlusCmd->SetGuidance("Example: /mySpectrum/setFilePlus beta_plus.txt");
    fFilePlusCmd->SetParameterName("file", false);
    fFilePlusCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fFilePlusCmd->SetToBeBroadcasted(false);

    fAtomicCutCmd = new G4UIcmdWithADoubleAndUnit("/mySpectrum/setAtomicCut", this);
    fAtomicCutCmd->SetGuidance("Low-energy cut for spectrum recording: decay electrons");
    fAtomicCutCmd->SetGuidance("below this energy are not stored (atomic Auger background).");
    fAtomicCutCmd->SetGuidance("Example: /mySpectrum/setAtomicCut 10 keV");
    fAtomicCutCmd->SetParameterName("energy", false);
    fAtomicCutCmd->SetRange("energy >= 0.");
    fAtomicCutCmd->SetUnitCategory("Energy");
    fAtomicCutCmd->SetDefaultUnit("keV");
    fAtomicCutCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fAtomicCutCmd->SetToBeBroadcasted(false);

    fPrimaryOnlyCmd = new G4UIcmdWithABool("/mySpectrum/setPrimaryOnly", this);
    fPrimaryOnlyCmd->SetGuidance("true: record only direct emission of the primary ion");
    fPrimaryOnlyCmd->SetGuidance("(no daughter conversion lines); false: record the full chain.");
    fPrimaryOnlyCmd->SetGuidance("Example: /mySpectrum/setPrimaryOnly true");
    fPrimaryOnlyCmd->SetParameterName("primaryOnly", false);
    fPrimaryOnlyCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
    fPrimaryOnlyCmd->SetToBeBroadcasted(false);
}

SpectrumMessenger::~SpectrumMessenger()
{
    delete fEnableCmd;
    delete fDisableCmd;
    delete fFileMinusCmd;
    delete fFilePlusCmd;
    delete fAtomicCutCmd;
    delete fPrimaryOnlyCmd;
    delete fDir;
}

void SpectrumMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    auto& cfg = BetaSpectrumConfig::Instance();

    if (command == fEnableCmd) {
        cfg.SetEnabled(true);
        G4cout << "Beta spectrum recording enabled." << G4endl;
    }
    else if (command == fDisableCmd) {
        cfg.SetEnabled(false);
        G4cout << "Beta spectrum recording disabled." << G4endl;
    }
    else if (command == fFileMinusCmd) {
        cfg.SetFileMinus(newValue);
        G4cout << "Beta- spectrum file: " << newValue << G4endl;
    }
    else if (command == fFilePlusCmd) {
        cfg.SetFilePlus(newValue);
        G4cout << "Beta+ spectrum file: " << newValue << G4endl;
    }
    else if (command == fAtomicCutCmd) {
        const G4double e = fAtomicCutCmd->GetNewDoubleValue(newValue);
        cfg.SetAtomicCut(e);
        G4cout << "Beta spectrum low-energy cut: " << e / CLHEP::keV << " keV" << G4endl;
    }
    else if (command == fPrimaryOnlyCmd) {
        const G4bool on = fPrimaryOnlyCmd->GetNewBoolValue(newValue);
        cfg.SetPrimaryOnly(on);
        G4cout << "Beta spectrum mode: "
               << (on ? "primary ion only" : "full decay chain") << G4endl;
    }
}
