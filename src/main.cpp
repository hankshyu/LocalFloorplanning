#include <iostream>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
#include "parser.h"
#include "ppsolver.h"

int main(int argc, char const *argv[]) {
    Parser parser(argv[1]);
    int pushForceList[8] = { 10, 20, 50, 100, 200, 300, 500, 1000 };
    int pushScale = 0;
    PPSolver *solver = new PPSolver;
    LFLegaliser *legaliser = new LFLegaliser((len_t) parser.getDieWidth(), (len_t) parser.getDieHeight());

    do {
        delete solver;
        delete legaliser;
        solver = new PPSolver;
        legaliser = new LFLegaliser((len_t) parser.getDieWidth(), (len_t) parser.getDieHeight());

        solver->readFromParser(parser);

        int iteration = 1000;
        solver->setupPushForce(pushForceList[pushScale++]);
        for ( int phase = 1; phase <= 50; phase++ ) {
            solver->setRadiusRatio(phase * 0.02);
            for ( int i = 0; i < iteration; i++ ) {
                solver->calcModuleForce();
                solver->moveModule();
            }
        }

        legaliser->translateGlobalFloorplanning(*solver);
        legaliser->detectfloorplanningOverlaps();
    } while ( legaliser->has3overlap() );



    solver->currentPosition2txt("global_test.txt");

    std::cout << "has 3 overlapped? " << legaliser->has3overlap() << std::endl;

    legaliser->visualiseArtpieceCYY("transform_test.txt");

    return 0;
}