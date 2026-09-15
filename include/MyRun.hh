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
///  - Path B (decay - escape): kinetic energy of particles created by
///    radioactive decay (Path B "in"), minus kinetic energy of particles
///    crossing the cylinder boundary outward, counted once per track
///    (Path B "out"). The net value is the energy transferred to the volume.
///
/// Comparing the two at run end is a built-in validation: a significant
/// divergence (A > B) indicates that some particles left the cylinder and
/// returned, depositing part of their energy on the way back — exactly the
/// situation that Path B cannot see by construction.
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

    // Path B "out": kinetic energy of particles leaving the cylinder (once per track).
    void AddEscapedEnergy(G4double e) { fEscapedEnergy += e; }
    G4double GetEscapedEnergy() const { return fEscapedEnergy; }

    // Path B net: energy transferred to the cylinder.
    G4double GetTransferredEnergy() const { return fDecayEnergy - fEscapedEnergy; }

    // Called by Geant4 to merge a worker run into the master run.
    void Merge(const G4Run* otherRun) override;

private:
    G4double fEdep         = 0.0;
    G4double fDecayEnergy  = 0.0;
    G4double fEscapedEnergy = 0.0;
};

#endif
