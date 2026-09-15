#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UImanager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <sstream>
#include <iomanip>

DetectorMessenger::DetectorMessenger(DetectorConstruction* detector)
    : fDetector(detector),
      fDir(new G4UIdirectory("/myDetector/"))
{
    fDir->SetGuidance(
        "Commands to set the container geometry before /run/initialize.\n"
        "Variants: cylinder (plain), beaker (open glass cup), tube (test tube\n"
        "with rounded bottom). The liquid (sensitive) volume is the medium;\n"
        "the glass shell is never sensitive. The hemisphere of the tube is\n"
        "always filled; 'rim' extends the empty (dry) cylindrical wall above\n"
        "the liquid level.");

    fGeometryCmd = new G4UIcmdWithAString("/myDetector/setGeometry", this);
    fGeometryCmd->SetGuidance("Set geometry variant: cylinder | beaker | tube");
    fGeometryCmd->SetGuidance("Example: /myDetector/setGeometry tube");
    fGeometryCmd->SetParameterName("geometry", false);
    fGeometryCmd->AvailableForStates(G4State_PreInit);
    fGeometryCmd->SetToBeBroadcasted(false);

    fRadiusCmd = new G4UIcmdWithADoubleAndUnit("/myDetector/setRadius", this);
    fRadiusCmd->SetGuidance("Set radius of the liquid cylinder");
    fRadiusCmd->SetGuidance("Example: /myDetector/setRadius 2.5 cm");
    fRadiusCmd->SetParameterName("radius", false);
    fRadiusCmd->SetRange("radius>0.");
    fRadiusCmd->SetUnitCategory("Length");
    fRadiusCmd->SetDefaultUnit("cm");
    fRadiusCmd->AvailableForStates(G4State_PreInit);
    fRadiusCmd->SetToBeBroadcasted(false);

    fHalfHeightCmd = new G4UIcmdWithADoubleAndUnit("/myDetector/setHalfHeight", this);
    fHalfHeightCmd->SetGuidance("Set half-height of the liquid cylinder");
    fHalfHeightCmd->SetGuidance("Example: /myDetector/setHalfHeight 3.0 cm");
    fHalfHeightCmd->SetParameterName("halfHeight", false);
    fHalfHeightCmd->SetRange("halfHeight>0.");
    fHalfHeightCmd->SetUnitCategory("Length");
    fHalfHeightCmd->SetDefaultUnit("cm");
    fHalfHeightCmd->AvailableForStates(G4State_PreInit);
    fHalfHeightCmd->SetToBeBroadcasted(false);

    fMaterialCmd = new G4UIcmdWithAString("/myDetector/setMaterial", this);
    fMaterialCmd->SetGuidance("Set liquid (medium) material by NIST name");
    fMaterialCmd->SetGuidance("Examples: G4_WATER, G4_TISSUE_SOFT_ICRP, G4_BONE_COMPACT_ICRU, G4_PLEXIGLASS");
    fMaterialCmd->SetParameterName("material", false);
    fMaterialCmd->AvailableForStates(G4State_PreInit);
    fMaterialCmd->SetToBeBroadcasted(false);

    fMaterialDensityCmd = new G4UIcmdWithADoubleAndUnit("/myDetector/setMaterialDensity", this);
    fMaterialDensityCmd->SetGuidance("Override the liquid material density");
    fMaterialDensityCmd->SetGuidance("Pass 0 or a negative value to revert to NIST default");
    fMaterialDensityCmd->SetGuidance("Example: /myDetector/setMaterialDensity 1.05 g/cm3");
    fMaterialDensityCmd->SetParameterName("density", false);
    fMaterialDensityCmd->SetUnitCategory("Volumic Mass");
    fMaterialDensityCmd->SetDefaultUnit("g/cm3");
    fMaterialDensityCmd->AvailableForStates(G4State_PreInit);
    fMaterialDensityCmd->SetToBeBroadcasted(false);

    fGlassWallCmd = new G4UIcmdWithADoubleAndUnit("/myDetector/setGlassWall", this);
    fGlassWallCmd->SetGuidance("Set wall thickness of the glass shell (all sides)");
    fGlassWallCmd->SetGuidance("Example: /myDetector/setGlassWall 1.5 mm");
    fGlassWallCmd->SetParameterName("wall", false);
    fGlassWallCmd->SetRange("wall>0.");
    fGlassWallCmd->SetUnitCategory("Length");
    fGlassWallCmd->SetDefaultUnit("mm");
    fGlassWallCmd->AvailableForStates(G4State_PreInit);
    fGlassWallCmd->SetToBeBroadcasted(false);

    fRimCmd = new G4UIcmdWithADoubleAndUnit("/myDetector/setRim", this);
    fRimCmd->SetGuidance("Set height of the dry (empty) cylindrical wall above the liquid");
    fRimCmd->SetGuidance("Example: /myDetector/setRim 5.0 mm");
    fRimCmd->SetParameterName("rim", false);
    fRimCmd->SetRange("rim>=0.");
    fRimCmd->SetUnitCategory("Length");
    fRimCmd->SetDefaultUnit("mm");
    fRimCmd->AvailableForStates(G4State_PreInit);
    fRimCmd->SetToBeBroadcasted(false);

    fGlassMaterialCmd = new G4UIcmdWithAString("/myDetector/setGlassMaterial", this);
    fGlassMaterialCmd->SetGuidance("Set glass (shell) material by NIST name");
    fGlassMaterialCmd->SetGuidance("Examples: G4_SILICON_DIOXIDE, G4_GLASS_LEAD");
    fGlassMaterialCmd->SetParameterName("material", false);
    fGlassMaterialCmd->AvailableForStates(G4State_PreInit);
    fGlassMaterialCmd->SetToBeBroadcasted(false);

    fGlassMaterialDensityCmd = new G4UIcmdWithADoubleAndUnit("/myDetector/setGlassMaterialDensity", this);
    fGlassMaterialDensityCmd->SetGuidance("Override the glass material density");
    fGlassMaterialDensityCmd->SetGuidance("Pass 0 or a negative value to revert to NIST default");
    fGlassMaterialDensityCmd->SetGuidance("Example: /myDetector/setGlassMaterialDensity 2.2 g/cm3");
    fGlassMaterialDensityCmd->SetParameterName("density", false);
    fGlassMaterialDensityCmd->SetUnitCategory("Volumic Mass");
    fGlassMaterialDensityCmd->SetDefaultUnit("g/cm3");
    fGlassMaterialDensityCmd->AvailableForStates(G4State_PreInit);
    fGlassMaterialDensityCmd->SetToBeBroadcasted(false);

    fDrawAxesCmd = new G4UIcmdWithoutParameter("/myDetector/drawAxes", this);
    fDrawAxesCmd->SetGuidance("Draw axes with size depending on cylinder dimensions");
    fDrawAxesCmd->SetToBeBroadcasted(false);
}

DetectorMessenger::~DetectorMessenger()
{
    delete fGeometryCmd;
    delete fRadiusCmd;
    delete fHalfHeightCmd;
    delete fMaterialCmd;
    delete fMaterialDensityCmd;
    delete fGlassWallCmd;
    delete fRimCmd;
    delete fGlassMaterialCmd;
    delete fGlassMaterialDensityCmd;
    delete fDrawAxesCmd;
    delete fDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fGeometryCmd) {
        fDetector->SetGeometry(newValue);
    }
    else if (command == fRadiusCmd) {
        const G4double radius = fRadiusCmd->GetNewDoubleValue(newValue);
        fDetector->SetRadius(radius);
        G4cout << "Liquid radius set to: " << radius / cm << " cm" << G4endl;
    }
    else if (command == fHalfHeightCmd) {
        const G4double halfHeight = fHalfHeightCmd->GetNewDoubleValue(newValue);
        fDetector->SetHalfHeight(halfHeight);
        G4cout << "Liquid half-height set to: " << halfHeight / cm << " cm" << G4endl;
    }
    else if (command == fMaterialCmd) {
        fDetector->SetMaterial(newValue);
    }
    else if (command == fMaterialDensityCmd) {
        const G4double density = fMaterialDensityCmd->GetNewDoubleValue(newValue);
        fDetector->SetCustomDensity(density);
    }
    else if (command == fGlassWallCmd) {
        const G4double wall = fGlassWallCmd->GetNewDoubleValue(newValue);
        fDetector->SetGlassWall(wall);
        G4cout << "Glass wall thickness set to: " << wall / mm << " mm" << G4endl;
    }
    else if (command == fRimCmd) {
        const G4double rim = fRimCmd->GetNewDoubleValue(newValue);
        fDetector->SetRim(rim);
        G4cout << "Dry rim height set to: " << rim / mm << " mm" << G4endl;
    }
    else if (command == fGlassMaterialCmd) {
        fDetector->SetGlassMaterial(newValue);
    }
    else if (command == fGlassMaterialDensityCmd) {
        const G4double density = fGlassMaterialDensityCmd->GetNewDoubleValue(newValue);
        fDetector->SetGlassCustomDensity(density);
    }
    else if (command == fDrawAxesCmd) {
        DrawAxes();
    }
}

void DetectorMessenger::DrawAxes()
{
    const G4double radius = fDetector->GetRadius();
    const G4double halfHeight = fDetector->GetHalfHeight();

    const G4double minDim = std::min(2.0 * radius, 2.0 * halfHeight);
    G4double axisSize = minDim * 0.5;
    if (axisSize < 0.5 * cm) axisSize = 0.5 * cm;

    G4UImanager* UI = G4UImanager::GetUIpointer();
    UI->ApplyCommand("/vis/scene/remove/axes");

    std::ostringstream cmd;
    cmd << "/vis/scene/add/axes 0 0 0 "
        << std::fixed << std::setprecision(4)
        << axisSize / cm << " cm";
    UI->ApplyCommand(cmd.str());

    G4cout << "Axes drawn with size: " << axisSize / cm << " cm" << G4endl;
}