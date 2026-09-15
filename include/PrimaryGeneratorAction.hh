#ifndef MYCYLINDER_PRIMARYGENERATORACTION_HH
#define MYCYLINDER_PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;
class G4Event;
class G4ParticleDefinition;

/// Generates primary vertices: a Lu-177 ion placed at a uniformly random
/// point inside the water cylinder. Cylinder dimensions are read from the
/// geometry at the start of each event, so changes made via
/// /myDetector/setRadius etc. are picked up automatically.
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* event) override;

private:
    G4ParticleGun*        fParticleGun;
    G4ParticleDefinition* fIonDefinition;  // cached on first use
};

#endif
