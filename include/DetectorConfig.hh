#ifndef MYCYLINDER_DETECTORCONFIG_HH
#define MYCYLINDER_DETECTORCONFIG_HH

#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

/// Singleton holding the detector (container) configuration: liquid
/// cylinder size, medium material, and the optional glass shell for the
/// beaker / test-tube variants. Set once in PreInit state via
/// /myDetector/* commands, then read while building the geometry in
/// DetectorConstruction and when printing results in RunAction.
///
/// Thread safety: state is read-only after /run/initialize, so concurrent
/// reads from worker threads are safe.
class DetectorConfig
{
public:
    enum class GeometryType {
        Cylinder,  // plain water cylinder, no shell
        Beaker,    // cylinder of medium inside an open cylindrical glass
        Tube       // cylinder + hemisphere (rounded bottom) inside glass
    };

    static DetectorConfig& Instance();

    void SetGeometry(const G4String& name);
    void SetRadius(G4double val);
    void SetHalfHeight(G4double val);
    void SetMaterial(const G4String& name);
    void SetCustomDensity(G4double density);
    void SetGlassWall(G4double val);
    void SetRim(G4double val);
    void SetGlassMaterial(const G4String& name);
    void SetGlassCustomDensity(G4double density);

    GeometryType GetGeometryType() const { return fGeometryType; }
    G4String     GetGeometryName() const;
    G4double     GetRadius()     const { return fRadius; }
    G4double     GetHalfHeight() const { return fHalfHeight; }
    G4String     GetMaterialName()          const { return fMaterialName; }
    G4double     GetCustomDensity()         const { return fCustomDensity; }
    G4double     GetGlassWall()             const { return fGlassWall; }
    G4double     GetRim()                   const { return fRim; }
    G4String     GetGlassMaterialName()     const { return fGlassMaterialName; }
    G4double     GetGlassCustomDensity()    const { return fGlassCustomDensity; }

    /// Effective density of the medium: custom value if set, or the NIST
    /// default density of the chosen material. Returns 0 if unknown.
    G4double GetEffectiveDensity() const;

    /// Effective density of the glass material (custom override or NIST).
    G4double GetGlassEffectiveDensity() const;

    /// Radius of the outermost glass surface (0 if no shell).
    G4double GetOuterRadius() const;

    /// Total vertical extent of the container: bottom (wall for beaker,
    /// R+wall for tube) .. top (liquid height + rim). Returns 2*halfHeight
    /// for the plain cylinder variant.
    G4double GetContainerHeight() const;

    /// Liquid (sensitive, medium) volume in cm3.
    G4double LiquidVolumeCm3() const;

    /// Rotation applied to the whole container so that its local +Z axis
    /// (up in construction frame) maps to world +Y. This is purely a
    /// visual/convenience convention: physics must not assume any fixed
    /// axis orientation.
    static G4RotationMatrix ContainerRotation();

    /// Transforms a world position into the container construction frame.
    static G4ThreeVector WorldToContainerLocal(const G4ThreeVector& pos);

    /// Transforms a container construction-frame position into the world.
    static G4ThreeVector ContainerLocalToWorld(const G4ThreeVector& pos);

private:
    DetectorConfig();

    GeometryType fGeometryType;  // default Cylinder
    G4double     fRadius;        // liquid cylinder radius (cm)
    G4double     fHalfHeight;    // liquid cylinder half-height (cm)
    G4String     fMaterialName;  // NIST medium name, e.g. "G4_WATER"
    G4double     fCustomDensity; // < 0 = use NIST default for medium
    G4double     fGlassWall;     // glass wall thickness (all sides), cm
    G4double     fRim;           // dry (air) part of the container above liquid
    G4String     fGlassMaterialName;   // NIST name, e.g. "G4_SILICON_DIOXIDE"
    G4double     fGlassCustomDensity;  // < 0 = use NIST default for glass
};

#endif