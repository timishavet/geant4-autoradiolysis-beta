#ifndef MYCYLINDER_SOURCECONFIG_HH
#define MYCYLINDER_SOURCECONFIG_HH

#include "G4Types.hh"
#include "G4String.hh"

/// Singleton holding the source configuration: radionuclide (Z, A,
/// excitation energy) and source activity. Set once in PreInit state via
/// /mySource/* commands, then read during the run by
/// PrimaryGeneratorAction, TrackingAction and RunAction.
///
/// Thread safety: state is read-only after /run/initialize, so concurrent
/// reads from worker threads are safe.
class SourceConfig
{
public:
    static SourceConfig& Instance();

    void SetRadionuclide(G4int Z, G4int A, G4double excitationEnergy = 0.0);
    void SetActivity(G4double activityBq);

    G4int    GetZ()          const { return fZ; }
    G4int    GetA()          const { return fA; }
    G4double GetExcitation() const { return fExcitation; }
    G4double GetActivity()   const { return fActivity; }

    /// Human-readable name ("Lu-177") for the currently set Z,A. Falls back
    /// to "Z-A" notation if not in the curated list.
    G4String GetRadionuclideName() const;

private:
    SourceConfig();

    G4int    fZ;           // default 71 (Lu)
    G4int    fA;           // default 177
    G4double fExcitation;  // default 0.0 (ground state)
    G4double fActivity;    // default 0.0 (Bq); informational only
};

#endif
