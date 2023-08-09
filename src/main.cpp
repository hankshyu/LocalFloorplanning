#include <iostream>
#include <iomanip>
#include "LFUnits.h"
#include <algorithm>
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
#include "parser.h"
#include "ppsolver.h"
#include "maxflowLegaliser.h"

void printCord(Cord cord) {
    std::cout << "(" << cord.x << ", " << cord.y << ")";
}
int main(int argc, char const *argv[]) {
    Parser parser(argv[1]);
    int pushForceList[8] = { 5, 10, 15, 20, 25, 30, 40, 50 };
    int pushScale = 0;
    PPSolver *solver, *bestSolution;
    LFLegaliser *legaliser;
    float minHPWL = 1e100;
    int iteration = 1500;
    std::cout << std::fixed;

    for ( pushScale = 0; pushScale < 8; pushScale++ ) {
        solver = new PPSolver;
        solver->readFromParser(parser);
        solver->setupPushForce(pushForceList[pushScale]);
        for ( int phase = 1; phase <= 50; phase++ ) {
            solver->setRadiusRatio(phase * 0.02);
            for ( int i = 0; i < iteration; i++ ) {
                solver->calcModuleForce();
                solver->moveModule();
            }
        }
        std::cout << "Estimated HPWL: " << std::setprecision(2) << solver->calcEstimatedHPWL() << std::endl;
        if ( solver->calcEstimatedHPWL() < minHPWL ) {
            minHPWL = solver->calcEstimatedHPWL();
            bestSolution = solver;
        }
        else {
            delete solver;
        }
    }

    legaliser = new LFLegaliser((len_t) parser.getDieWidth(), (len_t) parser.getDieHeight());
    legaliser->translateGlobalFloorplanning(*bestSolution);
    legaliser->detectfloorplanningOverlaps();



    std::cout << "Estimated HPWL in Global Phase: " << std::setprecision(2) << bestSolution->calcEstimatedHPWL() << std::endl;
    bestSolution->currentPosition2txt("outputs/global_test.txt");
    std::cout << "has 3 overlapped? " << legaliser->has3overlap() << std::endl;

    // visualiseArtPieceCYY is integratd into visualiseArtpiece fnc + false option.
    legaliser->visualiseArtpiece("outputs/transform_test.txt", false);

    std::cout << "Performing split..."<< std::endl;
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
    for(MFL::MFLTileFlowInfo tf : blockTileFlows){
        // tf.tile->show(std::cout);

        for(MFL::MFLSingleFlowInfo s : tf.fromFlows){

            s.sourceTile->show(std::cout);
            std::cout << "------" << s.flowAmount << "------";
            std::cout << "[";
            switch (s.direction)
            {
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

    //TODO: add physical allocation of blocks.


}