#include "SourceConfig.hh"

#include <map>
#include <utility>
#include <string>

SourceConfig& SourceConfig::Instance()
{
    static SourceConfig instance;
    return instance;
}

SourceConfig::SourceConfig()
    : fZ(71), fA(177), fExcitation(0.0), fActivity(0.0)
{}

void SourceConfig::SetRadionuclide(G4int Z, G4int A, G4double excitationEnergy)
{
    fZ = Z;
    fA = A;
    fExcitation = excitationEnergy;
}

void SourceConfig::SetActivity(G4double activityBq)
{
    fActivity = activityBq;
}

G4String SourceConfig::GetRadionuclideName() const
{
    // Curated list of common therapy / diagnostics radionuclides.
    // Keep in sync with the table in program_b.html.
    static const std::map<std::pair<G4int, G4int>, std::string> names = {
        {{71, 177}, "Lu-177"},
        {{53, 131}, "I-131"},
        {{39,  90}, "Y-90"},
        {{88, 223}, "Ra-223"},
        {{89, 225}, "Ac-225"},
        {{43,  99}, "Tc-99m"},
        {{ 9,  18}, "F-18"},
        {{53, 125}, "I-125"},
        {{62, 153}, "Sm-153"},
        {{75, 186}, "Re-186"},
        {{75, 188}, "Re-188"},
        {{27,  60}, "Co-60"},
        {{55, 137}, "Cs-137"},
        {{11,  22}, "Na-22"},
        {{63, 152}, "Eu-152"}
    };
    auto it = names.find({fZ, fA});
    if (it != names.end()) return G4String(it->second);
    return G4String(std::to_string(fZ) + "-" + std::to_string(fA));
}
