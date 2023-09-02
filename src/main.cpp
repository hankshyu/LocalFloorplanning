#include <iostream>
#include <iomanip>
#include <algorithm>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
// #include "parser.h"
#include "ppsolver.h"
#include "rgparser.h"
#include "rgsolver.h"
#include "maxflowLegaliser.h"
#include "monitor.h"
#include "paletteKnife.h"
#include "tensor.h"

int main(int argc, char const *argv[]) {

    Tile t1 = Tile(tileType::OVERLAP, Cord(7, 99), 38, 95);
    t1.OverlapFixedTesseraeIdx.push_back(1);
    t1.OverlapFixedTesseraeIdx.push_back(3);
    t1.OverlapFixedTesseraeIdx.push_back(5);
    t1.OverlapSoftTesseraeIdx.push_back(2);
    t1.OverlapSoftTesseraeIdx.push_back(4);
    t1.OverlapSoftTesseraeIdx.push_back(6);
    Tile *t2 = new Tile(tileType::BLOCK, Cord(34, 12), 14, 42);
    Tile *t33 = new Tile(tileType::BLOCK, Cord(10, 20), 3, 40);
    t2->bl = t33;
    t33->tr = t2;
    t2->OverlapFixedTesseraeIdx.push_back(11);
    t2->OverlapFixedTesseraeIdx.push_back(13);
    t2->OverlapFixedTesseraeIdx.push_back(15);
    t2->OverlapSoftTesseraeIdx.push_back(12);
    t2->OverlapSoftTesseraeIdx.push_back(14);
    t2->OverlapSoftTesseraeIdx.push_back(16);

    std::cout << "t1: " << t1 <<std::endl;
    t1.showLink(std::cout);
    std::cout << "t2: " << *t2 <<std::endl;
    t2->showLink(std::cout);
    Tile t3(t1);
    Tile t4 = Tile(*t2);
    Tile t5 = *t2;
    Tile *t6 = new Tile(t1);

    std::cout << "t3: " << t3 <<std::endl;
    t3.showLink(std::cout);
    std::cout << "t4: " << t4 <<std::endl;
    t4.showLink(std::cout);
    std::cout << "t5: " << t5 <<std::endl;
    t5.showLink(std::cout);
    std::cout << "t6: " << t6 <<std::endl;
    t6->showLink(std::cout);

    Tile t7;
    t7 = t1;
    std::cout << "t7: " << t7 << std::endl;
    t7 = *t2;
    std::cout << "t7: " << t7 << std::endl;
    t7.showLink(std::cout);
    delete(t2);
    std::cout << "t7(delete): " << t7 << std::endl;
    t7.showLink(std::cout);

    

    // RGParser rgparser(argv[1]);
    // RGSolver solver;
    // solver.readFromParser(rgparser);
    
    // mnt::Monitor monitor;
    // monitor.printCopyRight();
    
    // /* Phase 1: Global Floorplanning */
    // std::cout << std::endl << std::endl;
    // monitor.printPhase("Global Floorplanning Phase");
    // auto clockCounterbegin = std::chrono::steady_clock::now();

    // // Parser parser(argv[1]);
    // int pushForceList[8] = { 5, 10, 15, 20, 25, 30, 40, 50 };
    // int pushScale = 0;

    // LFLegaliser *legaliser;
    // int iteration = 6000;
    // double lr = 1. / iteration;
    

    // // ! These parameters can be modified to meet your needs
    // solver.setPunishment(0.03);
    // double tolaranceLen = ( rgparser.getDieWidth() + rgparser.getDieHeight() ) / 200;

    // for ( int phase = 1; phase <= 50; phase++ ) {
    //     solver.setSizeScalar(phase * 0.02);
    //     solver.setOverlapTolaranceLen(tolaranceLen * phase * 0.02);
    //     for ( int i = 0; i < iteration; i++ ) {
    //         solver.calcGradient();
    //         solver.gradientDescent(lr);
    //     }
    // }

    // solver.currentPosition2txt("outputs/global_test.txt");
    // std::cout << std::fixed;
    // std::cout << "Estimated HPWL: " << std::setprecision(2) << solver.calcEstimatedHPWL() << std::endl;

    // legaliser = new LFLegaliser((len_t) rgparser.getDieWidth(), (len_t) rgparser.getDieHeight());
    // legaliser->translateGlobalFloorplanning(solver);
    // legaliser->detectfloorplanningOverlaps();

    // // Phase 1 Reports
    // std::cout << std::endl;
    // monitor.printPhaseReport();
    // // std::cout << "Estimated HPWL in Global Phase: " << std::setprecision(2) << solver->calcEstimatedHPWL() << std::endl;
    // std::cout << "Multiple Tile overlap (>3) count: " << legaliser->has3overlap() << std::endl;
    // // solver->currentPosition2txt("outputs/ppmoduleResult.txt");
    // legaliser->visualiseArtpiece("outputs/phase1.txt", false);
    
    
    // /* Phase 2: Processing Corner Stiching */
    // std::cout << std::endl << std::endl;
    // monitor.printPhase("Extracting geographical information to IR");


    // std::cout << "2.1 Performing split ...";
    // legaliser->splitTesseraeOverlaps();
    // std::cout << "done!" << std::endl;

    // std::cout << "2.2 Painting All Tesserae to Canvas" << std::endl;
    // legaliser->arrangeTesseraetoCanvas();
    
    // std::cout << "2.3 Start combinable tile search, ";
    // std::vector <std::pair<Tile *, Tile *>> detectMergeTile;
    // legaliser->detectCombinableBlanks(detectMergeTile);
    // std::cout << detectMergeTile.size() << " candidates found" << std::endl << std::endl;
    // for(std::pair<Tile *, Tile *> tp : detectMergeTile){
    //     legaliser->visualiseAddMark(tp.first);
    //     legaliser->visualiseAddMark(tp.second);
    // }
    // legaliser->visualiseArtpiece("outputs/phase2_1.txt", true);

    // legaliser->visualiseRemoveAllmark();
    // while(!detectMergeTile.empty()){
    //     std::cout << "Merging Pair: #" << detectMergeTile.size() << std::endl;
    //     detectMergeTile[0].first->show(std::cout);
    //     detectMergeTile[0].second->show(std::cout);

    //     legaliser->combineVerticalMergeableBlanks(detectMergeTile[0].first, detectMergeTile[0].second);
    //     detectMergeTile.clear();
    //     legaliser->detectCombinableBlanks(detectMergeTile);
    // }


    // // Phase 2 Reports
    // std::cout << std::endl;
    // monitor.printPhaseReport();
    
    // legaliser->visualiseArtpiece("outputs/phase2.txt", true);


    // /* Phase 3: Promitive Overlap Removal */
    // std::cout << std::endl << std::endl;
    // monitor.printPhase("Primitive removal/break-down of Overlaps");

    // std::vector <RGConnStruct> connectionList = rgparser.getConnectionList();
    // paletteKnife spatula(legaliser, &connectionList);
    
    // spatula.disperseViaMargin();
    
    // // Phase 3 Reports
    // std::cout << std::endl;
    // std::cout << "Overlap Report: " << std::endl;
    // spatula.collectOverlaps();
    // spatula.printpaintClusters();
    // monitor.printPhaseReport();
    // legaliser->visualiseArtpiece("outputs/phase3.txt", true);

    // /* Phase 4: Solve 2-Overlaps */
    // std::cout << std::endl << std::endl;
    // monitor.printPhase("Solve Level-2 Overlaps");
    // spatula.eatCakesLevel2();



}