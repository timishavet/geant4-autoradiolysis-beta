#ifndef MYCYLINDER_MYRUN_HH
#define MYCYLINDER_MYRUN_HH

#include "G4Run.hh"
#include "G4Types.hh"

/// Per-run accumulators for the two energy-accounting paths.
///
/// In Geant4 MT mode each worker thread owns its own G4Run instance and
/// updates its accumulators without any synchronisation. At the end of the
/// run Geant4 automatically calls Merge() on every worker run into the
/// master run, summing the per-thread values. This is the idiomatic,
/// lock-free alternative to std::atomic accumulators in user actions.
///
/// Two paths converge here:
///  - Path A (full deposit): sum of step->GetTotalEnergyDeposit() over all
///    steps inside the cylinder, accumulated by DoseDetector.
///  - Path B (inject - outflow + inflow): kinetic energy of particles
///    created by radioactive decay (injected), minus kinetic energy carried
///    out across the cylinder boundary, plus kinetic energy carried back
///    in by returning tracks. The net value is the energy transferred to
///    the volume.
///
/// Comparing the two at run end is a built-in validation: with exact
/// boundary-flow bookkeeping both must agree up to tracking leftovers;
/// a significant divergence indicates a bookkeeping bug.
class MyRun : public G4Run
{
public:
    MyRun() = default;
    ~MyRun() override = default;

    // Path A: total energy deposited inside the cylinder.
    void AddEdep(G4double e) { fEdep += e; }
    G4double GetEdep() const { return fEdep; }

    // Path B "in": kinetic energy of all decay products.
    void AddDecayEnergy(G4double e) { fDecayEnergy += e; }
    G4double GetDecayEnergy() const { return fDecayEnergy; }

    // Path B "out": kinetic energy carried out across the cylinder boundary.
    void AddEscapedEnergy(G4double e) { fEscapedEnergy += e; }
    G4double GetEscapedEnergy() const { return fEscapedEnergy; }

    // Path B "in-flow": kinetic energy carried back in by returning tracks.
    void AddInflowEnergy(G4double e) { fInflowEnergy += e; }
    G4double GetInflowEnergy() const { return fInflowEnergy; }

    // Path B net: energy transferred to the cylinder.
    G4double GetTransferredEnergy() const
    {
        return fDecayEnergy - fEscapedEnergy + fInflowEnergy;
    }

    // Called by Geant4 to merge a worker run into the master run.
    void Merge(const G4Run* otherRun) override;

private:
    G4double fEdep          = 0.0;
    G4double fDecayEnergy   = 0.0;
    G4double fEscapedEnergy = 0.0;
    G4double fInflowEnergy  = 0.0;
};

#endif
