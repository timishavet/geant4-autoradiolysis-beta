#include "BetaSpectrumConfig.hh"

BetaSpectrumConfig& BetaSpectrumConfig::Instance()
{
    static BetaSpectrumConfig instance;
    return instance;
}

BetaSpectrumConfig::BetaSpectrumConfig()
    : fEnabled(false),
      fFileMinus("beta_minus.txt"),
      fFilePlus("beta_plus.txt"),
      fAtomicCut(0.0),
      fPrimaryOnly(true)
{}
