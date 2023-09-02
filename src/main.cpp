#include <iostream>
#include <iomanip>
#include <algorithm>
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

#define MAX_ITER 11

int main(int argc, char const *argv[]) {

    RGParser rgparser(argv[1]);
    RGSolver solver;
    solver.readFromParser(rgparser);
    
    std::vector<double> punishmentValues{
        50000.0, 100000.0, 200000.0, 
        5000.0, 10000.0, 50000.0,
        1.0, 100.0, 1000.0,
        0.03, 1  };
    std::vector<double> toleranceLengthValues(MAX_ITER);
    std::fill(toleranceLengthValues.begin(), toleranceLengthValues.begin()+3, 0);
    std::fill(toleranceLengthValues.begin()+3, toleranceLengthValues.begin()+6, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 1600);
    std::fill(toleranceLengthValues.begin()+6, toleranceLengthValues.begin()+9, (rgparser.getDieWidth() + rgparser.getDieHeight()) / 800);
    std::fill(toleranceLengthValues.begin()+9, toleranceLengthValues.end(), (rgparser.getDieWidth() + rgparser.getDieHeight()) / 400);

    mnt::Monitor monitor;
    LFLegaliser *legaliser = nullptr;
    monitor.printCopyRight();
    for (int iter = 0; iter < MAX_ITER; iter++){
        if (legaliser != nullptr){
            delete legaliser;
        }
        double toleranceValue = toleranceLengthValues[iter];
        double punishmentValue = punishmentValues[iter];
        std::cout << "ITERATION: " << iter;
        std::cout << "\nTolerance: " << toleranceValue;
        std::cout << "\nPunishment: " << punishmentValue << std::endl; 

        /* Phase 1: Global Floorplanning */
        std::cout << std::endl << std::endl;
        monitor.printPhase("Global Floorplanning Phase");
        auto clockCounterbegin = std::chrono::steady_clock::now();

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

        solver.currentPosition2txt("outputs/global_test.txt");
        std::cout << std::fixed;
        std::cout << "Estimated HPWL: " << std::setprecision(2) << solver.calcEstimatedHPWL() << std::endl;

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


        /* Phase 3: Overlap distribution via DFS */
        std::cout << std::endl << std::endl;
        monitor.printPhase("Overlap distribution");

        DFSL::DFSLegalizer *dfsl;
        dfsl = new DFSL::DFSLegalizer();
        dfsl->initDFSLegalizer(legaliser);
        DFSL::RESULT legalResult = dfsl->legalize();
        if (legalResult == DFSL::RESULT::SUCCESS){
            std::cout << "DSFL DONE\n" << std::endl;
            std::cout << "Checking legality..." << std::endl;
            bool legal = true;
            for (Tessera* tess: legaliser->softTesserae){
                if (!tess->isLegal()){
                    std::cout << tess->getName() << " is not legal!" << std::endl;
                    legal = false;
                }
            }
            
            if (!legal){
                std::cout << "Restarting process...\n" << std::endl;
            }
            else {
                legaliser->visualiseArtpiece("outputs/legal.txt", true);
                break;
            }
        }
        else if (legalResult == DFSL::RESULT::CONSTRAINT_FAIL ) {
            std::cout << "Constraints FAIL, restarting process...\n" << std::endl;
        }
        else {
            std::cout << "Impossible to solve, restarting process...\n" << std::endl;
        }
    }


    // Phase 3 Reports
    std::cout << std::endl;
    monitor.printPhaseReport();

    /* Phase 4: Physical Overlap distribution */
    // std::cout << std::endl << std::endl;
    // monitor.printPhase("Physical Overlap distribution");
    // std::cout << "4.1 Analyse Overlap distribution tiles" << std::endl;
    // legaliser->visualiseRemoveAllmark();

    // // Phase 4 Reports
    // std::cout << std::endl;
    // monitor.printPhaseReport();

}