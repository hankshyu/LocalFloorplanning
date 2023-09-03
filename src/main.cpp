#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cfloat>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
#include "parser.h"
#include "ppsolver.h"
#include "rgparser.h"
#include "rgsolver.h"
#include "DFSLegalizer.h"
#include "monitor.h"
#include "paletteKnife.h"

#define MAX_ITER 26
#define LEGAL_MAX_ITER 4

int main(int argc, char const *argv[]) {


    RGParser rgparser(argv[1]);
    RGSolver solver;
    solver.readFromParser(rgparser);
    
    std::vector<double> punishmentValues{
        0.05, 10.0, 100.0, 1000.0, 100000.0, 200000.0, 
        0.05, 10.0, 100.0, 5000.0, 10000.0, 40000.0,
        1.0, 100.0, 1000.0, 10000.0,
        1.0, 100.0, 1000.0, 10000.0,
        0.03, 1.0, 100.0,
        0.03, 1.0, 100.0  };
    std::vector<double> toleranceLengthValues(MAX_ITER);
    std::fill(toleranceLengthValues.begin(), toleranceLengthValues.begin()+6, 0);
    std::fill(toleranceLengthValues.begin()+6, toleranceLengthValues.begin()+12, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 1600);
    std::fill(toleranceLengthValues.begin()+12, toleranceLengthValues.begin()+16, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 800);
    std::fill(toleranceLengthValues.begin()+16, toleranceLengthValues.begin()+20, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 600);
    std::fill(toleranceLengthValues.begin()+20, toleranceLengthValues.begin()+23, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 400);
    std::fill(toleranceLengthValues.begin()+23, toleranceLengthValues.end(), (rgparser.getDieWidth() + rgparser.getDieHeight()) / 200);

    mnt::Monitor monitor;
    LFLegaliser *legaliser = nullptr;
    double bestHpwl = DBL_MAX;
    monitor.printCopyRight();
    for (int iter = 0; iter < MAX_ITER; iter++){
        
        int etMin;
        double etSec;
        double elapsedTime = monitor.getElapsedSeconds(etMin, etSec);
        double toleranceValue = toleranceLengthValues[iter];
        double punishmentValue = punishmentValues[iter];
        
        std::cout << "Starting Iteration " << iter << " Currrent clock time: " << etMin << "(min) " << etSec <<"(s)" << std::endl;
        if(etMin >= 27){
            std::cout << "Too late to start anothe iteration, terminate Program." << std::endl;
            exit(0);
        }else{
            std::cout << "Running with parameter: Tolerance: " << toleranceValue << ", Punishment: " << punishmentValue << std::endl; 
        }

        if (legaliser != nullptr){
            delete legaliser;
        }

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
            std::cout << "[RGSolver] ERROR: Area Constraint Violated.\n";
        }
        else {
            std::cout << "[RGSolver] Note: Area Constraint Met.\n";
        }

        solver.currentPosition2txt("outputs/global_test.txt");
        std::cout << std::fixed;
        std::cout << "[RGSolver] Estimated HPWL: " << std::setprecision(2) << solver.calcEstimatedHPWL() << std::endl;

        legaliser = new LFLegaliser((len_t) rgparser.getDieWidth(), (len_t) rgparser.getDieHeight());
        legaliser->translateGlobalFloorplanning(solver);
        legaliser->detectfloorplanningOverlaps();

        // Phase 1 Reports
        std::cout << std::endl;
        monitor.printPhaseReport();
        // std::cout << "Estimated HPWL in Global Phase: " << std::setprecision(2) << solver->calcEstimatedHPWL() << std::endl;
        std::cout << "Multiple Tile overlap (>3) count: " << legaliser->has3overlap() << std::endl;
        // solver->currentPosition2txt("outputs/ppmoduleResult.txt");
        legaliser->visualiseArtpiece("outputs/phase1.txt", false);
        
        
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
        legaliser->visualiseArtpiece("outputs/phase2_1.txt", true);

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
        legaliser->visualiseArtpiece("outputs/phase2.txt", true);

        /* Phase 3: disperseViaMerge*/
        // std::cout << std::endl << "Overlap Report:" << std::endl;
        // spatula.collectOverlaps();
        // spatula.printpaintClusters();
        // monitor.printPhaseReport();

        /* Phase 4: Overlap distribution via DFS */
        std::cout << std::endl << std::endl;
        monitor.printPhase("Overlap distribution");
        DFSL::DFSLegalizer dfsl;

        for (int legalIter = 0; legalIter < LEGAL_MAX_ITER; legalIter++){
            LFLegaliser legalizedFloorplan(*(legaliser));
            dfsl.initDFSLegalizer(&(legalizedFloorplan));
            if (legalIter == 0){
                std::cout << "LegalIter = 0, using default configs\n";
                // default configs
            }
            else if (legalIter == 1){
                // prioritize area 
                std::cout << "LegalIter = 1, prioritizing area\n";
                dfsl.config.OBAreaWeight = 2000.0;
                dfsl.config.OBUtilWeight = 750.0;
                dfsl.config.OBAspWeight = 100.0;
                dfsl.config.BWUtilWeight = 750.0;
                dfsl.config.BWAspWeight = 100.0;
            }
            else if (legalIter == 2){
                // prioritize util
                std::cout << "LegalIter = 2, prioritizing utilization\n";
                dfsl.config.OBAreaWeight = 700.0;
                dfsl.config.OBUtilWeight = 900.0;
                dfsl.config.OBAspWeight = 100.0;
                dfsl.config.BWUtilWeight = 2100.0;
                dfsl.config.BWAspWeight = 100.0;
            }
            else if (legalIter == 3){
                // prioritize aspect ratio
                std::cout << "LegalIter = 3, prioritizing aspect ratio\n";
                dfsl.config.OBAreaWeight = 650.0;
                dfsl.config.OBUtilWeight = 700.0;
                dfsl.config.OBAspWeight = 1000.0;
                dfsl.config.BWUtilWeight = 800.0;
                dfsl.config.BWAspWeight = 1100.0;
            }

            DFSL::RESULT legalResult = dfsl.legalize();
            if (legalResult == DFSL::RESULT::SUCCESS){
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
                }
                else {
                    double finalScore = calculateHPWL(&(legalizedFloorplan), rgparser.getConnectionList(), false);
                    printf("Final Score = %12.6f\n", finalScore);
                    if (finalScore < bestHpwl){
                        bestHpwl = finalScore;
                        std::cout << "Best Hpwl found" << std::endl;
                        outputFinalAnswer(&(legalizedFloorplan), rgparser, argv[2]);
                    }
                    legalizedFloorplan.visualiseArtpiece("outputs/legal.txt", true);
                    std::cout << std::endl;
                }
            }
            else if (legalResult == DFSL::RESULT::CONSTRAINT_FAIL ) {
                std::cout << "Constraints FAIL, restarting process...\n" << std::endl;
            }
            else {
                std::cout << "Impossible to solve, restarting process...\n" << std::endl;
            }
        }
    }

    std::cout << "\nFinished. Thank you..." << std::endl;
}