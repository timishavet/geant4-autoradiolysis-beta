#include "MyRun.hh"

void MyRun::Merge(const G4Run* otherRun)
{
    const auto* other = static_cast<const MyRun*>(otherRun);
    fEdep          += other->fEdep;
    fDecayEnergy   += other->fDecayEnergy;
    fEscapedEnergy += other->fEscapedEnergy;
    fInflowEnergy  += other->fInflowEnergy;
    fBetaMinus.insert(fBetaMinus.end(),
                      other->fBetaMinus.begin(), other->fBetaMinus.end());
    fBetaPlus.insert(fBetaPlus.end(),
                     other->fBetaPlus.begin(), other->fBetaPlus.end());

    G4Run::Merge(otherRun);
}
