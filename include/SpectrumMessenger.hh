#ifndef MYCYLINDER_SPECTRUMMESSENGER_HH
#define MYCYLINDER_SPECTRUMMESSENGER_HH

#include "G4UImessenger.hh"

class G4UIcommand;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithoutParameter;
class G4UIdirectory;

/// /mySpectrum/* UI commands for per-decay beta-energy recording.
/// Disabled by default; enable explicitly before /run/beamOn:
///   /mySpectrum/enable
///   /mySpectrum/setPrimaryOnly true
///   /mySpectrum/setFileMinus beta_minus.txt
///   /mySpectrum/setFilePlus  beta_plus.txt
///   /mySpectrum/setAtomicCut 10 keV
class SpectrumMessenger : public G4UImessenger
{
public:
    SpectrumMessenger();
    ~SpectrumMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    G4UIdirectory*           fDir;
    G4UIcmdWithoutParameter* fEnableCmd;
    G4UIcmdWithoutParameter* fDisableCmd;
    G4UIcmdWithAString*      fFileMinusCmd;
    G4UIcmdWithAString*      fFilePlusCmd;
    G4UIcmdWithADoubleAndUnit* fAtomicCutCmd;
    G4UIcmdWithABool*        fPrimaryOnlyCmd;
};

#endif
