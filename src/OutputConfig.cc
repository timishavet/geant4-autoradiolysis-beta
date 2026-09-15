#include "OutputConfig.hh"

OutputConfig& OutputConfig::Instance()
{
    static OutputConfig instance;
    return instance;
}

OutputConfig::OutputConfig()
    : fIsMT(true), fNumThreads(1)
{
    // Default: enable every field. User disables specific ones via
    // /myOutput/disable <name>. This keeps the default behaviour
    // backward-compatible with the previous all-fields output.
    fFlags[Field::EdepEnergy]        = true;
    fFlags[Field::EdepDose]          = true;
    fFlags[Field::MaterialName]      = true;
    fFlags[Field::MaterialDensity]   = true;
    fFlags[Field::Mass]              = true;
    fFlags[Field::ThreadingMode]     = true;
    fFlags[Field::NumEvents]         = true;
    fFlags[Field::Volume]            = true;
    fFlags[Field::Dimensions]        = true;
    fFlags[Field::DecayEnergy]       = true;
    fFlags[Field::EscapedEnergy]     = true;
    fFlags[Field::TransferredEnergy] = true;
    fFlags[Field::PathBDose]         = true;
    fFlags[Field::CrossCheck]        = true;
    fFlags[Field::Radionuclide]      = true;
    fFlags[Field::NumThreads]        = true;
    fFlags[Field::Timestamp]         = true;
    fFlags[Field::Activity]          = true;
    fFlags[Field::SimWallTime]       = true;
    fFlags[Field::AvgTimePerEvent]   = true;
    fFlags[Field::EnergyPerDecay]    = true;
    fFlags[Field::GeometryVariant]   = true;
    fFlags[Field::WallThickness]     = true;
    fFlags[Field::Rim]               = true;
    fFlags[Field::GlassMaterial]     = true;
}

void OutputConfig::Enable(Field f)  { fFlags[f] = true; }
void OutputConfig::Disable(Field f) { fFlags[f] = false; }

void OutputConfig::EnableAll()
{
    for (auto& kv : fFlags) kv.second = true;
}

void OutputConfig::DisableAll()
{
    for (auto& kv : fFlags) kv.second = false;
}

G4bool OutputConfig::IsEnabled(Field f) const
{
    auto it = fFlags.find(f);
    return it != fFlags.end() ? it->second : false;
}

G4String OutputConfig::FieldName(Field f)
{
    switch (f) {
        case Field::EdepEnergy:        return "edep_energy";
        case Field::EdepDose:          return "edep_dose";
        case Field::MaterialName:      return "material_name";
        case Field::MaterialDensity:   return "material_density";
        case Field::Mass:              return "mass";
        case Field::ThreadingMode:     return "threading_mode";
        case Field::NumEvents:         return "num_events";
        case Field::Volume:            return "volume";
        case Field::Dimensions:        return "dimensions";
        case Field::DecayEnergy:       return "decay_energy";
        case Field::EscapedEnergy:     return "escaped_energy";
        case Field::TransferredEnergy: return "transferred_energy";
        case Field::PathBDose:         return "path_b_dose";
        case Field::CrossCheck:        return "cross_check";
        case Field::Radionuclide:      return "radionuclide";
        case Field::NumThreads:        return "num_threads";
        case Field::Timestamp:         return "timestamp";
        case Field::Activity:          return "activity";
        case Field::SimWallTime:       return "sim_wall_time";
        case Field::AvgTimePerEvent:   return "avg_time_per_event";
        case Field::EnergyPerDecay:    return "energy_per_decay";
        case Field::GeometryVariant:   return "geometry_variant";
        case Field::WallThickness:     return "wall_thickness";
        case Field::Rim:               return "rim";
        case Field::GlassMaterial:     return "glass_material";
    }
    return "unknown";
}

G4bool OutputConfig::FieldFromName(const G4String& name, Field& out)
{
    for (int i = 0; i <= static_cast<int>(Field::GlassMaterial); ++i) {
        Field f = static_cast<Field>(i);
        if (FieldName(f) == name) {
            out = f;
            return true;
        }
    }
    return false;
}
