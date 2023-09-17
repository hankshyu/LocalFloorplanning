#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cfloat>
#include <stdio.h>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
#include "parser.h"
#include "ppsolver.h"
#include "rgsolver.h"
#include "DFSLegalizer.h"
#include "monitor.h"
#include "paletteKnife.h"

// #define MAX_ITER 12
#define LEGAL_MAX_ITER 4

#define MAX_MINUTE_RUNTIME 26

namespace rg = RectGrad;

int main(int argc, char const *argv[]) {

    bool legalSolutionFound = false;

    rg::Parser parser(argv[1]);

    
    // std::vector<double> punishmentValues{
    //     0.000001, 0.00001, 0.0001 ,0.001, 0.01, 0.1, 1.0 ,10.0, 100.0 ,1000.0, 10000.0, 100000.0, 1000000
    // };

    std::vector<double> punishmentValues{
        10E-5,
        10E-4, 2.5E-4,
        10E-3, 8.75E-2, 7.5E-3, 6.25E-3, 5E-3, 3.75E-3, 2.5E-3, 1.25E-3,
        10E-2, 8.75E-2, 7.5E-2, 6.25E-2, 5E-2, 3.75E-2, 2.5E-2, 1.25E-2,
        10E-1, 8.75E-2, 7.5E-1, 6.25E-1, 5E-1, 3.75E-1, 2.5E-1, 1.25E-1,
        1.0,
        10E+1, 10E+2, 10E+4
    };

    std::vector<double> toleranceLengthValues;
    for(int i = 0; i < punishmentValues.size(); ++i){
        toleranceLengthValues.push_back(0);
    }
    double pushValue = 1;
    while(pushValue < ((parser.getDieWidth() + parser.getDieHeight()) * 0.5 * 0.125)){
        for(int i = 0; i < punishmentValues.size(); ++i){
            toleranceLengthValues.push_back(pushValue);
        }
        pushValue = pushValue * 2;

    }
    const int MAX_ITER = toleranceLengthValues.size();
    std::cout << "MAX_ITER: " << MAX_ITER << std::endl;

    // toleranceLengthValues.push_back()

    // std::fill(toleranceLengthValues.begin(), toleranceLengthValues.begin()+4, 0);
    // std::fill(toleranceLengthValues.begin()+4, toleranceLengthValues.begin()+7, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 12800);
    // std::fill(toleranceLengthValues.begin()+4, toleranceLengthValues.begin()+7, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 1600);
    // std::fill(toleranceLengthValues.begin()+7, toleranceLengthValues.begin()+10, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 800);
    // std::fill(toleranceLengthValues.begin()+10, toleranceLengthValues.end(), (rgparser.getDieWidth() + rgparser.getDieHeight()) / 400);
    // std::fill(toleranceLengthValues.begin()+10, toleranceLengthValues.end(), (rgparser.getDieWidth() + rgparser.getDieHeight()) / 200);
    // std::fill(toleranceLengthValues.begin()+10, toleranceLengthValues.end(), (rgparser.getDieWidth() + rgparser.getDieHeight()) / 50);
    // std::fill(toleranceLengthValues.begin()+10, toleranceLengthValues.end(), (rgparser.getDieWidth() + rgparser.getDieHeight()) / 200);

    
    mnt::Monitor monitor;
    LFLegaliser *legaliser = nullptr;
    double bestHpwl = DBL_MAX;
    monitor.printCopyRight();
    for (int iter = 0; iter < MAX_ITER; iter++){
        
        try {
            monitor.startIteratrion();
            int etMin;
            double etSec;
            double elapsedTime = monitor.getElapsedSeconds(etMin, etSec);
            double toleranceValue = toleranceLengthValues[iter];
            double punishmentValue = punishmentValues[iter%punishmentValues.size()];
            // double punishmentValue = punishmentValues[iter % punishmentValues.size()];
            // double toleranceValue = toleranceLengthValues[iter / toleranceLengthValues.size()];
            
            std::cout << "Starting Iteration " << iter << " Currrent clock time: " << etMin << "(min) " << etSec <<"(s)";
            std::cout << ", current best HPWL = " << bestHpwl << std::endl;
            if(etMin >= MAX_MINUTE_RUNTIME){
                std::cout << "Too late to start anothe iteration, terminate Program." << std::endl;
                // This is final Report
                std::cout << "Final Report:" << std::endl << std::endl;
                monitor.finalReport(legalSolutionFound, bestHpwl, true);
                exit(0);
            }else{
                printf("Running with parameter: Tolerance: %14.6f, Punishment: %14.6f\n", toleranceValue, punishmentValue);
            }

            if (legaliser != nullptr){
                delete legaliser;
            }
            rg::GlobalSolver solver;
            solver.readFromParser(parser);

            /* Phase 1: Global Floorplanning */
            std::cout << std::endl << std::endl;
            monitor.printPhase("Global Floorplanning Phase", iter);
            // auto clockCounterbegin = std::chrono::steady_clock::now();

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

            legaliser = new LFLegaliser((len_t) parser.getDieWidth(), (len_t) parser.getDieHeight());
            legaliser->translateGlobalFloorplanning(solver);
            legaliser->detectfloorplanningOverlaps();

            // Phase 1 Reports
            std::cout << std::endl;
            monitor.printPhaseReport();
            // std::cout << "Estimated HPWL in Global Phase: " << std::setprecision(2) << solver->calcEstimatedHPWL() << std::endl;
            std::cout << "Multiple Tile overlap (>3) count: " << legaliser->has3overlap() << std::endl;
            // legaliser->visualiseArtpiece("outputs/phase1.txt", false);
            
            
            /* Phase 2: Processing Corner Stiching */
            std::cout << std::endl << std::endl;
            monitor.printPhase("Extracting geographical information to IR");


            std::cout << "2.1 Performing split ...";
            legaliser->splitTesseraeOverlaps();
            std::cout << "done!" << std::endl;

            std::cout << "2.2 Painting All Tesserae to Canvas" << std::endl;
            legaliser->arrangeTesseraetoCanvas();
            
            std::cout << "2.3 Start combinable tile search, ";
            std::vector <std::pair<Tile *, Tile *>> detectMergeTile;
            legaliser->detectCombinableBlanks(detectMergeTile);
            std::cout << detectMergeTile.size() << " candidates found" << std::endl << std::endl;
            for(std::pair<Tile *, Tile *> tp : detectMergeTile){
                legaliser->visualiseAddMark(tp.first);
                legaliser->visualiseAddMark(tp.second);
            }
            // legaliser->visualiseArtpiece("outputs/phase2_1.txt", true);

            legaliser->visualiseRemoveAllmark();
            while(!detectMergeTile.empty()){
                std::cout << "Merging Pair: #" << detectMergeTile.size() << std::endl;
                detectMergeTile[0].first->show(std::cout);
                detectMergeTile[0].second->show(std::cout);

                legaliser->combineVerticalMergeableBlanks(detectMergeTile[0].first, detectMergeTile[0].second);
                detectMergeTile.clear();
                legaliser->detectCombinableBlanks(detectMergeTile);
            }


            // Phase 2 Reports
            std::cout << std::endl;
            monitor.printPhaseReport();
            // legaliser->visualiseArtpiece("outputs/phase2.txt", true);

            
            // // Phase 3: Primitive Overlap Reduction
            // std::cout << std::endl << std::endl;
            // monitor.printPhase("Primitive removal/breaking-down Overlaps");
            // std::vector <RGConnStruct> connectionList = rgparser.getConnectionList();
            // paletteKnife spatula(legaliser, &connectionList);
            // spatula.disperseViaMargin();
            // // Phase 3 reports

            // std::cout << std::endl << "Overlap Report:" << std::endl;
            // spatula.printpaintClusters();
            // monitor.printPhaseReport();

            /* Phase 4: Overlap distribution via DFS */
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
                            monitor.recordInteration(iter, legalIter * 3 + legalizeMode, punishmentValue, toleranceValue,
                                storeOBAreaWeight, storeOBUtilWeight, storeOBAspWeight, storeBWUtilWeight, storeBWAspWeight,
                                itm, its, true, false, false, -1);

                        }else {
                            double finalScore = calculateHPWL(&(legalizedFloorplan), parser.getConnectionList(), false);
                            printf("Final Score = %12.6f\n", finalScore);
                            if (finalScore < bestHpwl){
                                bestHpwl = finalScore;
                                std::cout << "Best Hpwl found\n";
                                outputFinalAnswer(&(legalizedFloorplan), parser, argv[2]);
                                solver.currentPosition2txt("outputs/global_test.txt");
                            }
                            // legalizedFloorplan.visualiseArtpiece("outputs/legal.txt", true);
                            monitor.recordInteration(iter, legalIter * 3 + legalizeMode, punishmentValue, toleranceValue,
                                storeOBAreaWeight, storeOBUtilWeight, storeOBAspWeight, storeBWUtilWeight, storeBWAspWeight,
                                itm, its, true, true, false, finalScore);
                            std::cout << std::endl;
                        }
                    } else if (legalResult == DFSL::RESULT::CONSTRAINT_FAIL ) {
                        std::cout << "Constraints FAIL, restarting process...\n" << std::endl;
                        monitor.recordInteration(iter, legalIter * 3 + legalizeMode, punishmentValue, toleranceValue,
                            storeOBAreaWeight, storeOBUtilWeight, storeOBAspWeight, storeBWUtilWeight, storeBWAspWeight,
                            itm, its, false, false, false, -1);
                    } else {
                        std::cout << "Impossible to solve, restarting process...\n" << std::endl;
                        monitor.recordInteration(iter, legalIter * 3 + legalizeMode, punishmentValue, toleranceValue,
                            storeOBAreaWeight, storeOBUtilWeight, storeOBAspWeight, storeBWUtilWeight, storeBWAspWeight,
                            itm, its, false, false,false, -1);
                    }
                }
            }
        } catch (char const *errMsg) {
            std::cout << errMsg << std::endl;
            std::cout << "[ERROR] Caught an exception, skip to next iteration" << std::endl;

        }

    }

    // This is final Report
    std::cout << "Final Report:" << std::endl << std::endl;
    monitor.finalReport(legalSolutionFound, bestHpwl, true);

}