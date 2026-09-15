#include "MyRun.hh"

void MyRun::Merge(const G4Run* otherRun)
{
    const auto* other = static_cast<const MyRun*>(otherRun);
    fEdep          += other->fEdep;
    fDecayEnergy   += other->fDecayEnergy;
    fEscapedEnergy += other->fEscapedEnergy;

    G4Run::Merge(otherRun);
}
