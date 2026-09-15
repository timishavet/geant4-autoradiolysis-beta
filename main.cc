#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "SourceMessenger.hh"
#include "OutputMessenger.hh"
#include "OutputConfig.hh"
#include "FTFP_BERT.hh"

#include "G4RunManagerFactory.hh"
#include "G4MTRunManager.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4SteppingVerbose.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4IonTable.hh"
#include "G4HadronicParameters.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4Threading.hh"

#include <thread>

int main(int argc, char** argv)
{
    // Detect interactive mode (no arguments) and create a UI session.
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    G4int precision = 4;
    G4SteppingVerbose::UseBestUnit(precision);

    // MT run manager. Each worker thread runs its own subset of events;
    // per-thread accumulators in MyRun are merged automatically via Merge().
    auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::MT);

    // Default to the maximum number of hardware threads. Geant4's own
    // default may be 2 on some systems; we want full CPU utilisation.
    // The user can still override at runtime via /run/numberOfThreads N.
    if (auto* mtRM = G4MTRunManager::GetMasterRunManager()) {
        unsigned int hwThreads = std::thread::hardware_concurrency();
        if (hwThreads == 0) hwThreads = G4Threading::G4GetNumberOfCores();
        if (hwThreads == 0) hwThreads = 2;  // ultimate fallback
        mtRM->SetNumberOfThreads(hwThreads);
        OutputConfig::Instance().SetMT(true);
        OutputConfig::Instance().SetNumThreads(static_cast<G4int>(hwThreads));
    } else {
        OutputConfig::Instance().SetMT(false);
        OutputConfig::Instance().SetNumThreads(1);
    }

    // Detector + geometry/material messenger
    auto* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);
    new DetectorMessenger(detector);

    // Source (radionuclide + activity) and output-selection messengers.
    // Stateless singletons; safe to construct before /run/initialize.
    new SourceMessenger();
    new OutputMessenger();

    // Lift the G4RadioactiveDecay "very long decay time" threshold so that
    // long-lived nuclides (e.g. Co-60 T1/2=5.27 y, Cs-137 T1/2=30 y) will
    // still decay during the simulation. Starting with Geant4 11.2 the
    // default threshold is 1 year, which silently suppresses the at-rest
    // decays of such nuclides (no daughters, no deposited energy).
    // 1e+60 year is effectively "no threshold".
    //
    // The UI command /process/had/rdm/thresholdForVeryLongDecayTime cannot
    // be used here: the RDM messenger is created only at /run/initialize,
    // so a PreInit ApplyCommand would silently fail. Use the C++ interface
    // (Geant4 user documentation, section 5.2.6), which must be placed in
    // main() before run initialization.
    G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay(
        1.0e+60 * CLHEP::year);

    // Physics: FTFP_BERT reference list + step limiter + radioactive decay.
    auto* physicsList = new FTFP_BERT;
    physicsList->RegisterPhysics(new G4StepLimiterPhysics());
    physicsList->RegisterPhysics(new G4RadioactiveDecayPhysics());
    runManager->SetUserInitialization(physicsList);

    runManager->SetUserInitialization(new ActionInitialization());

    auto* visManager = new G4VisExecutive(argc, argv);
    visManager->Initialize();

    auto* UImanager = G4UImanager::GetUIpointer();

    if (!ui) {
        // Batch mode: execute the macro file passed as the first argument.
        const G4String command  = "/control/execute ";
        const G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }
    else {
        // Interactive mode: load visualisation defaults and the GUI menu.
        UImanager->ApplyCommand("/control/execute vis.mac");
        UImanager->ApplyCommand("/control/execute gui.mac");
        ui->SessionStart();
        delete ui;
    }

    delete visManager;
    delete runManager;
}
