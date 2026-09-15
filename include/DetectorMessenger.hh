#ifndef MYCYLINDER_DETECTORMESSENGER_HH
#define MYCYLINDER_DETECTORMESSENGER_HH

#include "G4UImessenger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4String.hh"

class DetectorConstruction;
class G4UIdirectory;

/// Exposes /myDetector/* UI commands for runtime geometry configuration
/// before /run/initialize: setGeometry, setRadius, setHalfHeight,
/// setMaterial, setMaterialDensity, setGlassWall, setRim,
/// setGlassMaterial, setGlassMaterialDensity, drawAxes.
class DetectorMessenger : public G4UImessenger
{
public:
    explicit DetectorMessenger(DetectorConstruction* detector);
    ~DetectorMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

    void DrawAxes();

private:
    DetectorConstruction* fDetector;

    G4UIdirectory*               fDir;
    G4UIcmdWithAString*          fGeometryCmd;
    G4UIcmdWithADoubleAndUnit*   fRadiusCmd;
    G4UIcmdWithADoubleAndUnit*   fHalfHeightCmd;
    G4UIcmdWithAString*          fMaterialCmd;
    G4UIcmdWithADoubleAndUnit*   fMaterialDensityCmd;
    G4UIcmdWithADoubleAndUnit*   fGlassWallCmd;
    G4UIcmdWithADoubleAndUnit*   fRimCmd;
    G4UIcmdWithAString*          fGlassMaterialCmd;
    G4UIcmdWithADoubleAndUnit*   fGlassMaterialDensityCmd;
    G4UIcmdWithoutParameter*     fDrawAxesCmd;
};

#endif
