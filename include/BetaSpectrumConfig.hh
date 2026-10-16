#ifndef MYCYLINDER_BETASPECTRUMCONFIG_HH
#define MYCYLINDER_BETASPECTRUMCONFIG_HH

#include "G4Types.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"

/// Singleton controlling per-decay beta-energy recording for spectrum
/// plots. Disabled by default so ordinary dose runs are unaffected
/// (no memory overhead, no extra files).
///
/// When enabled (via /mySpectrum/enable), TrackingAction records the
/// birth kinetic energy of every electron (beta-) and positron (beta+)
/// created by G4RadioactiveDecay, separately. At end of run RunAction
/// writes the two plain-text files (one energy per line, MeV).
///
/// Thread safety: state is read-only after /run/initialize, so
/// concurrent reads from worker threads are safe.
class BetaSpectrumConfig
{
public:
    static BetaSpectrumConfig& Instance();

    void SetEnabled(G4bool on) { fEnabled = on; }
    G4bool IsEnabled() const { return fEnabled; }

    void SetFileMinus(const G4String& name) { fFileMinus = name; }
    void SetFilePlus(const G4String& name)  { fFilePlus  = name; }
    const G4String& GetFileMinus() const { return fFileMinus; }
    const G4String& GetFilePlus()  const { return fFilePlus; }

    /// Low-energy cut for spectrum recording: decay electrons below this
    /// kinetic energy are not stored (default 0 = off, everything is
    /// recorded). Rationale for enabling it (e.g. 10 keV via
    /// /mySpectrum/setAtomicCut): below ~10 keV the RadioactiveDecay
    /// electron output is dominated by atomic effects (Auger cascade,
    /// shake-off, eV-scale relaxation electrons, ~1 per nuclear decay),
    /// while the true beta continuum below 10 keV carries a negligible
    /// fraction (<<1%) for the hundreds-keV/MeV beta emitters this
    /// program targets.
    void SetAtomicCut(G4double e) { fAtomicCut = e; }
    G4double GetAtomicCut() const { return fAtomicCut; }

    /// Primary-only mode (default true): record only electrons whose
    /// parent is the primary ion (ParentID == 1), i.e. the direct
    /// emission of the simulated nuclide. This drops internal-conversion
    /// and atomic electrons emitted in the de-excitation of daughter
    /// levels, leaving the pure beta continuum (plus conversion only if
    /// the primary itself was excited, e.g. Tc-99m).
    /// With false ("full chain"), electrons from all chain members are
    /// recorded: correct beta spectra of every daughter, but mixed in
    /// one file together with their conversion lines.
    void SetPrimaryOnly(G4bool on) { fPrimaryOnly = on; }
    G4bool IsPrimaryOnly() const { return fPrimaryOnly; }

private:
    BetaSpectrumConfig();

    G4bool   fEnabled;
    G4String fFileMinus;
    G4String fFilePlus;
    G4double fAtomicCut;
    G4bool   fPrimaryOnly;
};

#endif
