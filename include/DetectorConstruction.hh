#ifndef MYCYLINDER_DETECTORCONSTRUCTION_HH
#define MYCYLINDER_DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4String.hh"
#include "G4Types.hh"

class G4VPhysicalVolume;

/// Constructs the geometry: an air world containing a liquid (medium)
/// volume and, optionally, a glass shell (beaker or test-tube variant).
///
/// Geometry variants (see DetectorConfig::GeometryType):
///   - Cylinder: plain cylinder of medium, no shell.
///   - Beaker: medium cylinder inside an open cylindrical glass cup
///     (side wall + bottom, both of thickness wall; dry "rim" above the
///     liquid is the empty upper extension of the side wall).
///   - Tube: medium = cylinder + hemisphere (rounded bottom) of the same
///     radius, inside a matching glass shell. The hemisphere is always
///     filled; the rim extends the cylindrical part above the liquid.
///
/// The liquid volume keeps the logical volume name "Cylinder" so that the
/// source sampling (PrimaryGeneratorAction), the leakage checks
/// (TrackingAction) and the result printing (RunAction) keep working for
/// all variants. Only the liquid volume is sensitive.
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // Configuration setters (thin wrappers updating DetectorConfig).
    void SetGeometry(const G4String& name);
    void SetRadius(G4double val);
    void SetHalfHeight(G4double val);
    void SetMaterial(const G4String& name);
    void SetCustomDensity(G4double density);
    void SetGlassWall(G4double val);
    void SetRim(G4double val);
    void SetGlassMaterial(const G4String& name);
    void SetGlassCustomDensity(G4double density);

    G4double  GetRadius()     const;
    G4double  GetHalfHeight() const;
    G4String  GetMaterialName() const;
};

#endif