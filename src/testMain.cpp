#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cfloat>
#include <stdio.h>
#include "LFUnits.h"
// #include "Tile.h"
// #include "Tessera.h"
// #include "LFLegaliser.h"
#include "parser.h"
// #include "ppsolver.h"
#include "rgsolver.h"
// #include "DFSLegalizer.h"
#include "monitor.h"
// #include "paletteKnife.h"
#include "newTile.h"
#include "newTessera.h"
#include "FPManager.h"

// #define MAX_ITER 12
#define LEGAL_MAX_ITER 4

#define MAX_MINUTE_RUNTIME 26

namespace rg = RectGrad;

int main(int argc, char const *argv[]) {

    bool legalSolutionFound = false;

    rg::Parser parser(argv[1]);

    
    mnt::Monitor monitor;
    FPManager *FPM = nullptr;
    double bestHpwl = DBL_MAX;
        
    monitor.startIteratrion();
    int etMin;
    double etSec;
    double elapsedTime = monitor.getElapsedSeconds(etMin, etSec);
    double toleranceValue = 1;
    double punishmentValue = 10E-2;
    // double punishmentValue = punishmentValues[iter % punishmentValues.size()];
    // double toleranceValue = toleranceLengthValues[iter / toleranceLengthValues.size()];

    if (FPM != nullptr){
        delete FPM;
    }
    rg::GlobalSolver solver;
    solver.readFromParser(parser);

    int iteration = 20000;
    double lr = 5. / iteration;
    solver.setMaxMovement(0.001);


    // ! These parameters can be modified to meet your needs
    solver.setPunishment(punishmentValue);

    for ( int phase = 1; phase <= 50; phase++ ) {
        solver.setSizeScalar(phase * 0.02);
        solver.setOverlapTolaranceLen(toleranceValue * phase * 0.02);
        for ( int i = 0; i < iteration; i++ ) {
            solver.calcGradient();
            solver.gradientDescent(lr);
        }
    }

    solver.setPullWhileOverlap(false);
    solver.setMaxMovement(1e-6);
    solver.setPunishment(1e6);
    solver.setOverlapTolaranceLen(0.);
    solver.setSizeScalar(1.);
    lr = 1e-8;
    int count = 0;
    while ( solver.hasOverlap() ) {
        solver.squeezeToFit();
        for ( int i = 0; i < 5000; i++ ) {
            solver.calcGradient();
            solver.gradientDescent(lr);
        }

        if ( ++count >= 5 ) {
            break;
        }
    }

    if ( !solver.isAreaLegal() ) {
        std::cout << "[GlobalSolver] ERROR: Area Constraint Violated.\n";
    }
    else {
        std::cout << "[GlobalSolver] Note: Area Constraint Met.\n";
    }

    // solver.currentPosition2txt("outputs/global_test.txt");
    std::cout << std::fixed;
    std::cout << "[GlobalSolver] Estimated HPWL: " << std::setprecision(2) << solver.calcEstimatedHPWL() << std::endl;

    FPM = new FPManager((len_t) parser.getDieWidth(), (len_t) parser.getDieHeight());
    FPM->translateGlobalFloorplanning(solver);
    FPM->detectfloorplanningOverlaps();

    // Phase 1 Reports
    std::cout << std::endl;
    monitor.printPhaseReport();
    // std::cout << "Estimated HPWL in Global Phase: " << std::setprecision(2) << solver->calcEstimatedHPWL() << std::endl;
    std::cout << "Multiple Tile overlap (>3) count: " << FPM->has3overlap() << std::endl;
    // legaliser->visualiseArtpiece("outputs/phase1.txt", false);
    
    
    /* Phase 2: Processing Corner Stiching */
    std::cout << std::endl << std::endl;
    monitor.printPhase("Extracting geographical information to IR");


    std::cout << "2.1 Performing split ...";
    FPM->splitTesseraeOverlaps();
    std::cout << "done!" << std::endl;

    std::cout << "2.2 Painting All Tesserae to Canvas" << std::endl;
    FPM->arrangeTesseraetoCanvas();
    
    std::cout << "2.3 Start combinable tile search, ";
    std::vector <std::pair<Tile *, Tile *>> detectMergeTile;
    FPM->detectCombinableBlanks(detectMergeTile);
    std::cout << detectMergeTile.size() << " candidates found" << std::endl << std::endl;
    for(std::pair<Tile *, Tile *> tp : detectMergeTile){
        FPM->visualiseAddMark(tp.first);
        FPM->visualiseAddMark(tp.second);
    }
    // legaliser->visualiseArtpiece("outputs/phase2_1.txt", true);

    FPM->visualiseRemoveAllmark();
    while(!detectMergeTile.empty()){
        std::cout << "Merging Pair: #" << detectMergeTile.size() << std::endl;
        detectMergeTile[0].first->show(std::cout);
        detectMergeTile[0].second->show(std::cout);

        FPM->combineVerticalMergeableBlanks(detectMergeTile[0].first, detectMergeTile[0].second);
        detectMergeTile.clear();
        FPM->detectCombinableBlanks(detectMergeTile);
    }


    // Phase 2 Reports
    std::cout << std::endl;
    monitor.printPhaseReport();
    FPM->visualiseArtpiece("outputs/phase2.txt", true);

 /*

    std::cout << std::endl << std::endl;
    monitor.printPhase("Overlap distribution");
    DFSL::DFSLegalizer dfsl;

    for (int legalIter = 0; legalIter < LEGAL_MAX_ITER; legalIter++){
        for (int legalizeMode = 0; legalizeMode < 3; legalizeMode++){
            LFLegaliser legalizedFloorplan(*(legaliser));
            dfsl.initDFSLegalizer(&(legalizedFloorplan));

            double storeOBAreaWeight;
            double storeOBUtilWeight;
            double storeOBAspWeight;
            double storeBWUtilWeight;
            double storeBWAspWeight;
            
            
            if (legalIter == 0){
                std::cout << "LegalIter = 0, using default configs\n";
                // default configs
                storeOBAreaWeight = 750.0;
                storeOBUtilWeight = 1000.0;
                storeOBAspWeight = 100.0;
                storeBWUtilWeight = 1500.0;
                storeBWAspWeight = 100.0;
            }
            else if (legalIter == 1){
                // prioritize area 
                std::cout << "LegalIter = 1, prioritizing area\n";
                dfsl.config.OBAreaWeight = storeOBAreaWeight = 1400.0;
                dfsl.config.OBUtilWeight = storeOBUtilWeight = 750.0;
                dfsl.config.OBAspWeight = storeOBAspWeight = 100.0;
                dfsl.config.BWUtilWeight = storeBWUtilWeight = 750.0;
                dfsl.config.BWAspWeight = storeBWAspWeight = 100.0;
            }
            else if (legalIter == 2){
                // prioritize util
                std::cout << "LegalIter = 2, prioritizing utilization\n";
                dfsl.config.OBAreaWeight = storeOBAreaWeight  = 750.0;
                dfsl.config.OBUtilWeight = storeOBUtilWeight  = 900.0;
                dfsl.config.OBAspWeight = storeOBAspWeight = 100.0;
                dfsl.config.BWUtilWeight = storeBWUtilWeight = 2000.0;
                dfsl.config.BWAspWeight = storeBWAspWeight = 100.0;
            }
            else if (legalIter == 3){
                // prioritize aspect ratio
                std::cout << "LegalIter = 3, prioritizing aspect ratio\n";
                dfsl.config.OBAreaWeight = storeOBAreaWeight  = 750.0;
                dfsl.config.OBUtilWeight = storeOBUtilWeight  = 1100.0;
                dfsl.config.OBAspWeight = storeOBAspWeight = 1000.0;
                dfsl.config.BWUtilWeight = storeBWUtilWeight = 1000.0;
                dfsl.config.BWAspWeight = storeBWAspWeight = 1100.0;
            }
            std::cout << "Legalization mode: " << legalizeMode << std::endl;

            DFSL::RESULT legalResult = dfsl.legalize(legalizeMode);
            int itm;
            double its;
            monitor.getIterationSeconds(itm, its);

            if (legalResult == DFSL::RESULT::SUCCESS){
                legalSolutionFound = true;
                std::cout << "DSFL DONE\n";
                std::cout << "Checking legality..." << std::endl;
                bool legal = true;
                for (Tessera* tess: legalizedFloorplan.softTesserae){
                    if (!tess->isLegal()){
                        std::cout << tess->getName() << " is not legal!" << std::endl;
                        legal = false;
                    }
                }
                
                if (!legal){
                    std::cout << "Restarting process...\n" << std::endl;
                }else {
                    double finalScore = calculateHPWL(&(legalizedFloorplan), parser.getConnectionList(), false);
                    printf("Final Score = %12.6f\n", finalScore);
                    if (finalScore < bestHpwl){
                        bestHpwl = finalScore;
                        std::cout << "Best Hpwl found\n";
                        outputFinalAnswer(&(legalizedFloorplan), parser, argv[2]);
                        solver.currentPosition2txt("outputs/global_test.txt");
                    }
                    legalizedFloorplan.visualiseArtpiece("outputs/legal" + std::to_string(legalIter*3+legalizeMode) + ".txt", true);
                }
            } else if (legalResult == DFSL::RESULT::CONSTRAINT_FAIL ) {
                std::cout << "Constraints FAIL, restarting process...\n" << std::endl;
            } else {
                std::cout << "Impossible to solve, restarting process...\n" << std::endl;
            }
        }
    } 
    
*/
    
}