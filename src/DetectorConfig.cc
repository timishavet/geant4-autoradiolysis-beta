#include "DetectorConfig.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <algorithm>

DetectorConfig& DetectorConfig::Instance()
{
    static DetectorConfig instance;
    return instance;
}

DetectorConfig::DetectorConfig()
    : fGeometryType(GeometryType::Cylinder),
      fRadius(2.0 * cm),
      fHalfHeight(3.0 * cm),
      fMaterialName("G4_WATER"),
      fCustomDensity(-1.0),
      fGlassWall(1.5 * mm),
      fRim(0.0),
      fGlassMaterialName("G4_SILICON_DIOXIDE"),
      fGlassCustomDensity(-1.0)
{}

void DetectorConfig::SetGeometry(const G4String& name)
{
    G4String n = G4String(name).strip();
    if (n == "cylinder" || n == "plain") {
        fGeometryType = GeometryType::Cylinder;
    } else if (n == "beaker") {
        fGeometryType = GeometryType::Beaker;
    } else if (n == "tube") {
        fGeometryType = GeometryType::Tube;
    } else {
        G4cerr << "Warning: unknown geometry type '" << name
               << "' (expected cylinder|beaker|tube). Ignored." << G4endl;
        return;
    }
    G4cout << "Geometry type set to: " << GetGeometryName() << G4endl;
}

G4String DetectorConfig::GetGeometryName() const
{
    switch (fGeometryType) {
        case GeometryType::Cylinder: return "cylinder";
        case GeometryType::Beaker:   return "beaker";
        case GeometryType::Tube:     return "tube";
    }
    return "cylinder";
}

void DetectorConfig::SetRadius(G4double val)
{
    fRadius = val;
}

void DetectorConfig::SetHalfHeight(G4double val)
{
    fHalfHeight = val;
}

void DetectorConfig::SetMaterial(const G4String& name)
{
    G4NistManager* nist = G4NistManager::Instance();
    if (!nist->FindOrBuildMaterial(name)) {
        G4cerr << "Warning: material '" << name
               << "' not found in NIST manager. Ignored, keeping '"
               << fMaterialName << "'." << G4endl;
        return;
    }
    fMaterialName = name;
    G4cout << "Cylinder material set to: " << name << G4endl;
}

void DetectorConfig::SetCustomDensity(G4double density)
{
    fCustomDensity = density;
    if (density > 0.0) {
        G4cout << "Cylinder custom density: " << density / (g / cm3)
               << " g/cm3" << G4endl;
    } else {
        G4cout << "Cylinder custom density cleared; NIST default used."
               << G4endl;
    }
}

void DetectorConfig::SetGlassWall(G4double val)
{
    fGlassWall = val;
}

void DetectorConfig::SetRim(G4double val)
{
    fRim = val;
}

void DetectorConfig::SetGlassMaterial(const G4String& name)
{
    G4NistManager* nist = G4NistManager::Instance();
    if (!nist->FindOrBuildMaterial(name)) {
        G4cerr << "Warning: glass material '" << name
               << "' not found in NIST manager. Ignored, keeping '"
               << fGlassMaterialName << "'." << G4endl;
        return;
    }
    fGlassMaterialName = name;
    G4cout << "Glass material set to: " << name << G4endl;
}

void DetectorConfig::SetGlassCustomDensity(G4double density)
{
    fGlassCustomDensity = density;
    if (density > 0.0) {
        G4cout << "Glass custom density: " << density / (g / cm3)
               << " g/cm3" << G4endl;
    } else {
        G4cout << "Glass custom density cleared; NIST default used." << G4endl;
    }
}

G4double DetectorConfig::GetEffectiveDensity() const
{
    if (fCustomDensity > 0.0) return fCustomDensity;
    G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial(fMaterialName);
    return mat ? mat->GetDensity() : 0.0;
}

G4double DetectorConfig::GetGlassEffectiveDensity() const
{
    if (fGlassCustomDensity > 0.0) return fGlassCustomDensity;
    G4Material* mat = G4NistManager::Instance()->FindOrBuildMaterial(fGlassMaterialName);
    return mat ? mat->GetDensity() : 0.0;
}

G4double DetectorConfig::GetOuterRadius() const
{
    if (fGeometryType == GeometryType::Cylinder) return fRadius;
    return fRadius + fGlassWall;
}

G4double DetectorConfig::GetContainerHeight() const
{
    switch (fGeometryType) {
        case GeometryType::Cylinder:
            return 2.0 * fHalfHeight;
        case GeometryType::Beaker:
            return fGlassWall + 2.0 * fHalfHeight + fRim;
        case GeometryType::Tube:
            return fGlassWall + fRadius + 2.0 * fHalfHeight + fRim;
    }
    return 2.0 * fHalfHeight;
}

G4double DetectorConfig::LiquidVolumeCm3() const
{
    const G4double r = GetRadius();
    const G4double h = 2.0 * GetHalfHeight();
    const G4double vCyl = CLHEP::pi * r * r * h;
    if (fGeometryType == GeometryType::Tube) {
        const G4double vHemi = (2.0 / 3.0) * CLHEP::pi * r * r * r;
        return (vCyl + vHemi) / (cm * cm * cm);
    }
    return vCyl / (cm * cm * cm);
}

G4RotationMatrix DetectorConfig::ContainerRotation()
{
    // Rotate the whole container +90 deg about X: local +Z (construction
    // frame, where every solid is built, cap at -Z) becomes world +Y. So
    // the vessel stands upright along +Y, rim up, rounded bottom down,
    // which matches the visualizer convention.
    G4RotationMatrix rot;
    rot.rotateX(90.0 * deg);
    return rot;
}

G4ThreeVector DetectorConfig::WorldToContainerLocal(const G4ThreeVector& pos)
{
    // Geometry is placed with ContainerRotation() passed to G4PVPlacement,
    // which applies the INVERSE of that matrix to the solids. So a world
    // point back maps to the construction frame with C itself.
    return ContainerRotation() * pos;
}

G4ThreeVector DetectorConfig::ContainerLocalToWorld(const G4ThreeVector& pos)
{
    // See WorldToContainerLocal: construction frame -> world uses C^{-1}.
    return ContainerRotation().inverse() * pos;
}