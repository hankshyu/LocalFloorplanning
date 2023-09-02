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

int main(int argc, char const *argv[]) {

    RGParser rgparser(argv[1]);
    RGSolver solver;
    solver.readFromParser(rgparser);
    
    mnt::Monitor monitor;
    monitor.printCopyRight();
    
    /* Phase 1: Global Floorplanning */
    std::cout << std::endl << std::endl;
    monitor.printPhase("Global Floorplanning Phase");
    auto clockCounterbegin = std::chrono::steady_clock::now();

    LFLegaliser *legaliser;
    int iteration = 20000;
    double lr = 5. / iteration;
    solver.setMaxMovement(0.001);

    // ! These parameters can be modified to meet your needs
    solver.setPunishment(100000.);
    // double tolaranceLen = ( rgparser.getDieWidth() + rgparser.getDieHeight() ) / 200;
    double tolaranceLen = 0;

    for ( int phase = 1; phase <= 50; phase++ ) {
        solver.setSizeScalar(phase * 0.02);
        solver.setOverlapTolaranceLen(tolaranceLen * phase * 0.02);
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
    dfsl->legalize();
    std::cout << "DSFL DONE\n";

    legaliser->visualiseArtpiece("outputs/legal.txt", true);

    // Phase 3 Reports
    // std::cout << std::endl;
    // monitor.printPhaseReport();

    /* Phase 4: Physical Overlap distribution */
    // std::cout << std::endl << std::endl;
    // monitor.printPhase("Physical Overlap distribution");
    // std::cout << "4.1 Analyse Overlap distribution tiles" << std::endl;
    // legaliser->visualiseRemoveAllmark();

    // // Phase 4 Reports
    // std::cout << std::endl;
    // monitor.printPhaseReport();

}