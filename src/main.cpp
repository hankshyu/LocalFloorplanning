#include <iostream>
#include <iomanip>
#include "LFUnits.h"
#include <algorithm>
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
#include "parser.h"
#include "ppsolver.h"
#include "rgparser.h"
#include "rgsolver.h"
#include "maxflowLegaliser.h"

void printCord(Cord cord) {
    std::cout << "(" << cord.x << ", " << cord.y << ")";
}
int main(int argc, char const *argv[]) {
    // * You can still use it if 3 overlaps are too many
    // Parser parser(argv[1]);
    // int pushForceList[8] = { 5, 10, 15, 20, 25, 30, 40, 50 };
    // int pushScale = 0;
    // PPSolver *solver, *bestSolution;
    // LFLegaliser *legaliser;
    // float minHPWL = 1e100;
    // int iteration = 1500;
    // std::cout << std::fixed;
    // for ( pushScale = 0; pushScale < 8; pushScale++ ) {
    //     solver = new PPSolver;
    //     solver->readFromParser(parser);
    //     solver->setupPushForce(pushForceList[pushScale]);
    //     for ( int phase = 1; phase <= 50; phase++ ) {
    //         solver->setRadiusRatio(phase * 0.02);
    //         for ( int i = 0; i < iteration; i++ ) {
    //             solver->calcModuleForce();
    //             solver->moveModule();
    //         }
    //     }
    //     std::cout << "Estimated HPWL: " << std::setprecision(2) << solver->calcEstimatedHPWL() << std::endl;
    //     if ( solver->calcEstimatedHPWL() < minHPWL ) {
    //         minHPWL = solver->calcEstimatedHPWL();
    //         bestSolution = solver;
    //     }
    //     else {
    //         delete solver;
    //     }
    // }
    // legaliser = new LFLegaliser((len_t) parser.getDieWidth(), (len_t) parser.getDieHeight());
    // legaliser->translateGlobalFloorplanning(*bestSolution);
    // legaliser->detectfloorplanningOverlaps();
    // std::cout << "Estimated HPWL in Global Phase: " << std::setprecision(2) << bestSolution->calcEstimatedHPWL() << std::endl;
    // bestSolution->currentPosition2txt("outputs/global_test.txt");
    // std::cout << "has 3 overlapped? " << legaliser->has3overlap() << std::endl;



    RGParser rgparser(argv[1]);
    RGSolver solver;
    solver.readFromParser(rgparser);
    LFLegaliser *legaliser;
    int iteration = 6000;
    double lr = 1. / iteration;

    // ! These parameters can be modified to meet your needs
    solver.setPunishment(0.03);
    solver.setOverlapTolaranceLen(( rgparser.getDieWidth() + rgparser.getDieHeight() ) / 200);

    for ( int phase = 1; phase <= 50; phase++ ) {
        solver.setSizeScalar(phase * 0.02);
        for ( int i = 0; i < iteration; i++ ) {
            solver.calcGradient();
            solver.gradientDescent(lr);
        }
    }

    solver.currentPosition2txt("outputs/global_test.txt");
    std::cout << std::fixed;
    std::cout << "Estimated HPWL: " << std::setprecision(2) << solver.calcEstimatedHPWL() << std::endl;

    legaliser = new LFLegaliser((len_t) rgparser.getDieWidth(), (len_t) rgparser.getDieHeight());
    legaliser->translateGlobalFloorplanning(solver);
    legaliser->detectfloorplanningOverlaps();


    std::cout << "has 3 overlapped? " << legaliser->has3overlap() << std::endl;

    // visualiseArtPieceCYY is integratd into visualiseArtpiece fnc + false option.
    legaliser->visualiseArtpiece("outputs/transform_test.txt", false);

    std::cout << "Performing split..." << std::endl;
    legaliser->splitTesseraeOverlaps();

    legaliser->arrangeTesseraetoCanvas();
    legaliser->visualiseArtpiece("outputs/cornerStiching.txt", true);

    MFL::MaxflowLegaliser MFL;
    MFL.initMFL(legaliser);
    MFL.legaliseByMaxflow();
    std::vector<MFL::MFLTileFlowInfo> overlapTileFlows, blockTileFlows, blankTileFlows;
    MFL.outputFlows(overlapTileFlows, blockTileFlows, blankTileFlows);

    std::cout << " ======= MaxFlow Result Report ======= " << std::endl;
    std::cout << "OverlapTileFlows:" << std::endl;
    for ( MFL::MFLTileFlowInfo tf : blockTileFlows ) {
        // tf.tile->show(std::cout);

        for ( MFL::MFLSingleFlowInfo s : tf.fromFlows ) {

            s.sourceTile->show(std::cout);
            std::cout << "------" << s.flowAmount << "------";
            std::cout << "[";
            switch ( s.direction ) {
            case MFL::TOP:
                std::cout << "TOP";
                break;
            case MFL::RIGHT:
                std::cout << "RIGHT";
                break;
            case MFL::DOWN:
                std::cout << "DOWN";
                break;
            case MFL::LEFT:
                std::cout << "LEFT";
                break;
            default:
                break;
            }
            std::cout << "]------------->";

            s.destTile->show(std::cout);

        }
    }


}