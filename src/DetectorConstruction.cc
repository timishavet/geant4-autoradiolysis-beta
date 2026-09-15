#include "DetectorConstruction.hh"
#include "DetectorConfig.hh"
#include "DoseDetector.hh"

#include "G4SDManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4UnionSolid.hh"
#include "G4Transform3D.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4ios.hh"

namespace {

/// Builds a NIST material, optionally cloned with a custom density.
G4Material* ResolveMaterial(const G4String& nistName,
                            G4double customDensity,
                            const G4String& cloneName,
                            const G4String& fallback)
{
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* base = nist->FindOrBuildMaterial(nistName);
    if (!base) {
        G4cerr << "Error: material '" << nistName
               << "' not found; falling back to '" << fallback << "'." << G4endl;
        base = nist->FindOrBuildMaterial(fallback);
        if (!base) return nullptr;
    }
    if (customDensity > 0.0) {
        G4Material* clone = nist->BuildMaterialWithNewDensity(
            cloneName, base->GetName(), customDensity);
        if (!clone) {
            G4cerr << "Warning: could not build custom-density material; "
                   << "using NIST default." << G4endl;
            return base;
        }
        return clone;
    }
    return base;
}

} // namespace

DetectorConstruction::DetectorConstruction() = default;

DetectorConstruction::~DetectorConstruction() = default;

void DetectorConstruction::SetGeometry(const G4String& name)
{
    DetectorConfig::Instance().SetGeometry(name);
}

void DetectorConstruction::SetRadius(G4double val)
{
    DetectorConfig::Instance().SetRadius(val);
}

void DetectorConstruction::SetHalfHeight(G4double val)
{
    DetectorConfig::Instance().SetHalfHeight(val);
}

void DetectorConstruction::SetMaterial(const G4String& name)
{
    DetectorConfig::Instance().SetMaterial(name);
}

void DetectorConstruction::SetCustomDensity(G4double density)
{
    DetectorConfig::Instance().SetCustomDensity(density);
}

void DetectorConstruction::SetGlassWall(G4double val)
{
    DetectorConfig::Instance().SetGlassWall(val);
}

void DetectorConstruction::SetRim(G4double val)
{
    DetectorConfig::Instance().SetRim(val);
}

void DetectorConstruction::SetGlassMaterial(const G4String& name)
{
    DetectorConfig::Instance().SetGlassMaterial(name);
}

void DetectorConstruction::SetGlassCustomDensity(G4double density)
{
    DetectorConfig::Instance().SetGlassCustomDensity(density);
}

G4double DetectorConstruction::GetRadius() const
{
    return DetectorConfig::Instance().GetRadius();
}

G4double DetectorConstruction::GetHalfHeight() const
{
    return DetectorConfig::Instance().GetHalfHeight();
}

G4String DetectorConstruction::GetMaterialName() const
{
    return DetectorConfig::Instance().GetMaterialName();
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto& cfg = DetectorConfig::Instance();

    G4NistManager* nist = G4NistManager::Instance();
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");

    G4Material* liquidMat = ResolveMaterial(
        cfg.GetMaterialName(), cfg.GetCustomDensity(),
        "CustomLiquidMaterial", "G4_WATER");

    const G4double R   = cfg.GetRadius();
    const G4double Hh  = cfg.GetHalfHeight();   // half-height of liquid cylinder
    const G4double H   = 2.0 * Hh;              // liquid cylinder height
    const G4double wall = cfg.GetGlassWall();
    const G4double rim  = cfg.GetRim();

    const auto geoType = cfg.GetGeometryType();
    const bool hasGlass = (geoType != DetectorConfig::GeometryType::Cylinder);

    // Rotate the whole container so its construction-frame +Z points along
    // world +Y (stands upright for the visualizer). Every placement below
    // shares this single rotation.
    G4RotationMatrix* containerRot =
        new G4RotationMatrix(DetectorConfig::ContainerRotation());

    G4Material* glassMat = nullptr;
    if (hasGlass) {
        glassMat = ResolveMaterial(
            cfg.GetGlassMaterialName(), cfg.GetGlassCustomDensity(),
            "CustomGlassMaterial", "G4_SILICON_DIOXIDE");
    }

    // World box: half-extent = 3x the larger of (outer radius,
    // half of container height). Generous for every variant, including
    // the test-tube whose rounded bottom hangs below the origin.
    const G4double worldHalf = std::max(cfg.GetOuterRadius(),
                                        cfg.GetContainerHeight() / 2.0) * 3.0;

    G4Box* solidWorld = new G4Box("World", worldHalf, worldHalf, worldHalf);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    G4VPhysicalVolume* physWorld =
        new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    G4LogicalVolume* liquidLV = nullptr;

    if (geoType == DetectorConfig::GeometryType::Tube) {
        // Liquid = cylinder (z in [0, H]) + hemisphere (z in [-R, 0]),
        // united into a single solid so the medium is one volume.
        auto* hemiSolid = new G4Sphere("LiquidHemi", 0., R, 0., 360.*deg,
                                       90.*deg, 90.*deg);
        auto* cylSolid  = new G4Tubs("LiquidCyl", 0., R, Hh, 0., 360.*deg);
        auto* unionSolid = new G4UnionSolid(
            "Cylinder", hemiSolid, cylSolid,
            G4Transform3D(G4RotationMatrix(), G4ThreeVector(0., 0., Hh)));
        liquidLV = new G4LogicalVolume(unionSolid, liquidMat, "Cylinder");
        new G4PVPlacement(containerRot, G4ThreeVector(), liquidLV, "Cylinder",
                          logicWorld, false, 0);

        if (glassMat) {
            // Side wall above the hemisphere: G4PVPlacement applies the
            // transpose of pRot to the frame, so world tlate = pRot^T * tlocal.
            G4ThreeVector sideTlate =
                (*containerRot).inverse() * G4ThreeVector(0., 0., (H + rim) / 2.);
            auto* sideSolid = new G4Tubs("GlassSide", R, R + wall,
                                         (H + rim) / 2., 0., 360.*deg);
            auto* sideLV = new G4LogicalVolume(sideSolid, glassMat, "GlassSide");
            new G4PVPlacement(containerRot, sideTlate,
                              sideLV, "GlassSide", logicWorld, false, 0);
            // Rounded bottom shell: hemisphere of glass around the liquid cap.
            auto* hemiGlassSolid = new G4Sphere("GlassHemi", R, R + wall,
                                                0., 360.*deg, 90.*deg, 90.*deg);
            auto* hemiGlassLV = new G4LogicalVolume(hemiGlassSolid, glassMat, "GlassHemi");
            new G4PVPlacement(containerRot, G4ThreeVector(), hemiGlassLV, "GlassHemi",
                              logicWorld, false, 0);
            hemiGlassLV->SetVisAttributes(
                new G4VisAttributes(G4Colour(0.7, 0.75, 0.8, 0.4)));
            sideLV->SetVisAttributes(
                new G4VisAttributes(G4Colour(0.7, 0.75, 0.8, 0.4)));
        }
    } else {
        // Plain cylinder (no glass) or beaker: liquid is a single cylinder
        // centered at the origin in both cases.
        auto* solidCylinder = new G4Tubs("Cylinder", 0., R, Hh, 0., 360.*deg);
        liquidLV = new G4LogicalVolume(solidCylinder, liquidMat, "Cylinder");
        new G4PVPlacement(containerRot, G4ThreeVector(), liquidLV, "Cylinder",
                          logicWorld, false, 0);

        if (glassMat) {
            // Side wall: rises from the top of the bottom disk (z = -Hh)
            // up to z = Hh + rim; no overlap with the bottom.
            G4ThreeVector sideTlate =
                (*containerRot).inverse() * G4ThreeVector(0., 0., rim / 2.);
            auto* sideSolid = new G4Tubs("GlassSide", R, R + wall,
                                         Hh + rim / 2., 0., 360.*deg);
            auto* sideLV = new G4LogicalVolume(sideSolid, glassMat, "GlassSide");
            new G4PVPlacement(containerRot, sideTlate,
                              sideLV, "GlassSide", logicWorld, false, 0);
            // Bottom wall: solid disk under the liquid cylinder.
            G4ThreeVector bottomTlate =
                (*containerRot).inverse() * G4ThreeVector(0., 0., -Hh - wall / 2.);
            auto* bottomSolid = new G4Tubs("GlassBottom", 0., R + wall,
                                           wall / 2., 0., 360.*deg);
            auto* bottomLV = new G4LogicalVolume(bottomSolid, glassMat, "GlassBottom");
            new G4PVPlacement(containerRot, bottomTlate,
                              bottomLV, "GlassBottom", logicWorld, false, 0);
            sideLV->SetVisAttributes(
                new G4VisAttributes(G4Colour(0.7, 0.75, 0.8, 0.4)));
            bottomLV->SetVisAttributes(
                new G4VisAttributes(G4Colour(0.7, 0.75, 0.8, 0.4)));
        }
    }

    G4VisAttributes* visAtt = new G4VisAttributes(G4Colour(0.0, 0.5, 1.0, 0.3));
    liquidLV->SetVisAttributes(visAtt);

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    auto* doseDet = new DoseDetector("WaterDose");
    G4SDManager::GetSDMpointer()->AddNewDetector(doseDet);
    SetSensitiveDetector("Cylinder", doseDet);
}