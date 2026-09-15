#ifndef MYCYLINDER_OUTPUTCONFIG_HH
#define MYCYLINDER_OUTPUTCONFIG_HH

#include "G4Types.hh"
#include "G4String.hh"
#include <map>

/// Singleton controlling which result fields are printed/saved at the end
/// of a run. Modified via /myOutput/* commands (set in PreInit state before
/// /run/initialize), read by RunAction at end of master run.
///
/// The list of fields is enumerated in Field. FieldName() returns the
/// canonical short id used in macros and HTML; keep it in sync with the
/// `outputFields` table in program_b.html.
class OutputConfig
{
public:
    enum class Field {
        // User-requested core fields
        EdepEnergy,         // absorbed energy (MeV) - Path A
        EdepDose,           // absorbed dose (Gy) - Path A
        MaterialName,
        MaterialDensity,
        Mass,
        ThreadingMode,      // single- / multi-threaded
        // Suggested additional fields
        NumEvents,
        Volume,
        Dimensions,         // cylinder radius + height
        DecayEnergy,        // Path B "in"
        EscapedEnergy,      // Path B "out"
        TransferredEnergy,  // Path B net
        PathBDose,          // Path B absorbed dose (Gy)
        CrossCheck,         // A - B
        Radionuclide,       // Z, A, name
        NumThreads,
        Timestamp,
        Activity,
        SimWallTime,
        AvgTimePerEvent,
        EnergyPerDecay,
        // Geometry variant fields (cylinder / beaker / tube)
        GeometryVariant,    // variant id string
        WallThickness,      // glass wall thickness (cm)
        Rim,                // dry rim height above the liquid (cm)
        GlassMaterial       // glass material name + density
    };

    static OutputConfig& Instance();

    void Enable(Field f);
    void Disable(Field f);
    void EnableAll();
    void DisableAll();
    G4bool IsEnabled(Field f) const;

    /// Lookup field by canonical id string (used by OutputMessenger).
    /// Returns true and sets *out on success.
    static G4bool FieldFromName(const G4String& name, Field& out);
    static G4String FieldName(Field f);

    G4bool IsMT() const { return fIsMT; }
    void   SetMT(G4bool isMT) { fIsMT = isMT; }

    G4int GetNumThreads() const { return fNumThreads; }
    void  SetNumThreads(G4int n) { fNumThreads = n; }

private:
    OutputConfig();

    std::map<Field, G4bool> fFlags;
    G4bool fIsMT;
    G4int  fNumThreads;
};

#endif
