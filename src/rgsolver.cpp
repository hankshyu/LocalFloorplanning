#include "rgsolver.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

RGSolver::RGSolver() {
    std::srand(std::time(NULL));
    softModuleNum = 0;
    fixedModuleNum = 0;
    sizeScalar = 1;
    punishment = 1e4;
    overlapTolaranceLen = 0;
}

RGSolver::~RGSolver() {
    for ( int i = 0; i < modules.size(); i++ ) {
        delete modules[i];
    }
}

void RGSolver::setOutline(int width, int height) {
    DieWidth = (double) width;
    DieHeight = (double) height;
    xMaxMovement = DieWidth / 4000.;
    yMaxMovement = DieHeight / 4000.;
}

void RGSolver::setSoftModuleNum(int num) {
    softModuleNum = num;
    moduleNum = softModuleNum + fixedModuleNum;
    xGradient.resize(moduleNum);
    yGradient.resize(moduleNum);
}

void RGSolver::setFixedModuleNum(int num) {
    fixedModuleNum = num;
    moduleNum = softModuleNum + fixedModuleNum;
    xGradient.resize(moduleNum);
    yGradient.resize(moduleNum);
}

void RGSolver::setConnectionNum(int num) {
    connectionNum = num;
}

void RGSolver::addModule(RGModule *in_module) {
    modules.push_back(in_module);
}

void RGSolver::addConnection(std::string ma, std::string mb, double value) {
    RGModule *m0;
    RGModule *m1;
    for ( int i = 0; i < modules.size(); i++ ) {
        if ( modules[i]->name == ma )
            m0 = modules[i];
        else if ( modules[i]->name == mb )
            m1 = modules[i];
    }
    m0->addConnection(m1, value);
    m1->addConnection(m0, value);
}

void RGSolver::readFromParser(RGParser parser) {
    setOutline(parser.getDieWidth(), parser.getDieHeight());
    setSoftModuleNum(parser.getSoftModuleNum());
    setFixedModuleNum(parser.getFixedModuleNum());
    setConnectionNum(parser.getConnectionNum());
    for ( int i = 0; i < this->modules.size(); i++ ) {
        delete this->modules[i];
    }
    this->modules.clear();
    for ( int i = 0; i < this->moduleNum; i++ ) {
        RGModule copy = parser.getModule(i);
        RGModule *newModule;
        if ( copy.fixed ) {
            newModule = new RGModule(copy.name, copy.x, copy.y, copy.width, copy.height, copy.area, copy.fixed);
        }
        else {
            // double turbX = (double) ( std::rand() - RAND_MAX / 2 ) / RAND_MAX * 1e-8 * DieWidth;
            // double turbY = (double) ( std::rand() - RAND_MAX / 2 ) / RAND_MAX * 1e-8 * DieHeight;
            newModule = new RGModule(copy.name, copy.centerX, copy.centerY, copy.area, copy.fixed);
        }
        this->modules.push_back(newModule);
    }
    double scalar = -1.;
    for ( int i = 0; i < connectionNum; i++ ) {
        RGConnStruct conn = parser.getConnection(i);
        if ( (double) conn.value > scalar ) {
            scalar = (double) conn.value;
        }
    }
    connectNormalize = 1. / scalar;
    for ( int i = 0; i < connectionNum; i++ ) {
        RGConnStruct conn = parser.getConnection(i);

        RGModule *m0;
        RGModule *m1;
        for ( int i = 0; i < modules.size(); i++ ) {
            if ( modules[i]->name == conn.m0 )
                m0 = modules[i];
            else if ( modules[i]->name == conn.m1 )
                m1 = modules[i];
        }
        m0->addConnection(m1, (double) conn.value);
        m1->addConnection(m0, (double) conn.value);
    }
}

void RGSolver::currentPosition2txt(std::string file_name) {
    for ( auto &mod : modules ) {
        mod->updateCord((int) this->DieWidth, (int) this->DieHeight, sizeScalar);
    }
    std::ofstream ostream(file_name);
    ostream << "BLOCK " << moduleNum << " CONNECTOIN " << connectionNum << std::endl;
    ostream << DieWidth << " " << DieHeight << std::endl;
    for ( int i = 0; i < moduleNum; i++ ) {
        ostream << modules[i]->name << " ";
        ostream << ( ( modules[i]->fixed ) ? "FIXED" : "SOFT" ) << " ";
        ostream << modules[i]->x << " " << modules[i]->y << " ";
        if ( modules[i]->fixed ) {
            ostream << modules[i]->width << " " << modules[i]->height << std::endl;
        }
        else {
            ostream << modules[i]->width * sizeScalar << " " << modules[i]->height * sizeScalar << std::endl;
        }
    }
    std::vector<RGModule *> added;
    for ( int i = 0; i < moduleNum; i++ ) {
        added.push_back(modules[i]);
        for ( int j = 0; j < modules[i]->connections.size(); j++ ) {
            if ( std::find(added.begin(), added.end(), modules[i]->connections[j]->module) != added.end() )
                continue;
            ostream << modules[i]->name << " ";
            ostream << modules[i]->connections[j]->module->name << " ";
            ostream << modules[i]->connections[j]->value << std::endl;
        }
    }
    ostream.close();
}

double RGSolver::calcDeadspace() {
    double dieArea = DieWidth * DieHeight;
    double moduleArea = 0;
    for ( int i = 0; i < moduleNum; i++ ) {
        moduleArea += modules[i]->area;
    }
    return 1. - moduleArea / dieArea;
}

void RGSolver::calcGradient() {
    RGModule *curModule;
    for ( int i = 0; i < moduleNum; i++ ) {
        if ( modules[i]->fixed == true ) {
            continue;
        }

        curModule = modules[i];
        double x_grad = 0;
        double y_grad = 0;

        // gradient for HPWL
        for ( int j = 0; j < curModule->connections.size(); j++ ) {
            RGModule *pullModule = curModule->connections[j]->module;
            double pullValue = curModule->connections[j]->value * connectNormalize;
            double x_diff, y_diff;

            x_diff = curModule->centerX - pullModule->centerX;
            y_diff = curModule->centerY - pullModule->centerY;
            if ( x_diff == 0 && y_diff == 0 ) {
                continue;
            }

            double curWidth = curModule->width * sizeScalar;
            double pushWidth = ( pullModule->fixed ) ? pullModule->width : pullModule->width * sizeScalar;
            double curHeight = curModule->height * sizeScalar;
            double pushHeight = ( pullModule->fixed ) ? pullModule->height : pullModule->height * sizeScalar;
            double overlappedWidth, overlappedHeight;
            overlappedWidth = ( curWidth + pushWidth ) / 2. - std::abs(x_diff);
            overlappedHeight = ( curHeight + pushHeight ) / 2. - std::abs(y_diff);

            if ( overlappedWidth > overlapTolaranceLen && overlappedHeight > overlapTolaranceLen ) {
                continue;
            }

            double x_sign = ( x_diff == 0 ) ? 0 : ( x_diff > 0 ) ? 1. : -1.;
            double y_sign = ( y_diff == 0 ) ? 0 : ( y_diff > 0 ) ? 1. : -1.;
            x_grad += pullValue * x_sign;
            y_grad += pullValue * y_sign;
            // x_grad += pullValue * x_diff;
            // y_grad += pullValue * y_diff;
        }

        // gradient for overlapped area
        for ( int j = 0; j < moduleNum; j++ ) {
            if ( j == i )
                continue;
            RGModule *pushModule = modules[j];
            double overlappedWidth, overlappedHeight, x_diff, y_diff;

            x_diff = curModule->centerX - pushModule->centerX;
            y_diff = curModule->centerY - pushModule->centerY;
            if ( x_diff == 0 && y_diff == 0 ) {
                continue;
            }

            double curWidth = curModule->width * sizeScalar;
            double pushWidth = ( pushModule->fixed ) ? pushModule->width : pushModule->width * sizeScalar;
            double curHeight = curModule->height * sizeScalar;
            double pushHeight = ( pushModule->fixed ) ? pushModule->height : pushModule->height * sizeScalar;
            overlappedWidth = ( curWidth + pushWidth ) / 2. - std::abs(x_diff);
            overlappedHeight = ( curHeight + pushHeight ) / 2. - std::abs(y_diff);
            if ( overlappedWidth <= overlapTolaranceLen || overlappedHeight <= overlapTolaranceLen ) {
                continue;
            }

            // if ( overlappedWidth > curModule->width ) {
            //     overlappedWidth = (double) curModule->width;
            // }
            // else if ( overlappedWidth > pushModule->width ) {
            //     overlappedWidth = (double) pushModule->width;
            // }

            // if ( overlappedHeight > curModule->height ) {
            //     overlappedHeight = (double) curModule->height;
            // }
            // else if ( overlappedHeight > pushModule->height ) {
            //     overlappedHeight = (double) pushModule->height;
            // }

            double x_sign = ( x_diff == 0 ) ? 0 : ( x_diff > 0 ) ? 1. : -1.;
            double y_sign = ( y_diff == 0 ) ? 0 : ( y_diff > 0 ) ? 1. : -1.;
            x_grad += -punishment * x_sign * overlappedHeight;
            y_grad += -punishment * y_sign * overlappedWidth;
            // x_grad += -punishment * x_sign * std::abs(10);
            // y_grad += -punishment * y_sign * std::abs(10);

        }

        xGradient[i] = x_grad;
        yGradient[i] = y_grad;
    }

    //for ( int i = 0; i < moduleNum; i++ ) {
    //    std::cout << modules[i]->name << ": " << xGradient[i] << " " << yGradient[i] << std::endl;
    //}
}

void RGSolver::gradientDescent(double lr) {
    // move soft modules
    RGModule *curModule;
    for ( int i = 0; i < moduleNum; i++ ) {
        if ( modules[i]->fixed == true )
            continue;

        curModule = modules[i];

        double xMovement = xGradient[i] * lr * DieWidth;
        double yMovement = yGradient[i] * lr * DieHeight;

        if ( std::abs(xMovement) > xMaxMovement ) {
            xMovement = ( xMovement > 0 ) ? xMaxMovement : -xMaxMovement;
        }
        if ( std::abs(yMovement) > yMaxMovement ) {
            yMovement = ( yMovement > 0 ) ? yMaxMovement : -yMaxMovement;
        }

        curModule->centerX -= xMovement;
        curModule->centerY -= yMovement;

        if ( curModule->centerX < curModule->width / 2. ) {
            curModule->centerX = curModule->width / 2.;
        }
        if ( curModule->centerY < curModule->height / 2. ) {
            curModule->centerY = curModule->height / 2.;
        }
        if ( curModule->centerX > DieWidth - curModule->width / 2. ) {
            curModule->centerX = DieWidth - curModule->width / 2.;
        }
        if ( curModule->centerY > DieHeight - curModule->height / 2. ) {
            curModule->centerY = DieHeight - curModule->height / 2.;
        }
    }
}

double RGSolver::calcEstimatedHPWL() {
    double HPWL = 0;
    for ( int i = 0; i < moduleNum; i++ ) {
        RGModule *curModule = modules[i];
        for ( int j = 0; j < curModule->connections.size(); j++ ) {
            RGModule *conModule = curModule->connections[j]->module;
            double value = curModule->connections[j]->value;
            double x_diff = std::abs(curModule->centerX - conModule->centerX);
            double y_diff = std::abs(curModule->centerY - conModule->centerY);
            HPWL += ( x_diff + y_diff ) * value;
        }
    }
    return HPWL / 2.;
}

void RGSolver::setSizeScalar(double scalar) {
    sizeScalar = scalar;
}

void RGSolver::setPunishment(double force) {
    punishment = force;
}

void RGSolver::setupPunishment(double amplification) {
    double maxConnection = 0;
    for ( int i = 0; i < moduleNum; i++ ) {
        RGModule *curModule = modules[i];
        double connection = 0;
        for ( int j = 0; j < curModule->connections.size(); j++ ) {
            connection += curModule->connections[j]->value;
        }
        if ( connection > maxConnection ) {
            maxConnection = connection;
        }
    }
    punishment = maxConnection * amplification;
    // std::cout << "Set push force = " << punishment << std::endl;
}

void RGSolver::setMaxMovement(double ratio) {
    xMaxMovement = DieWidth * ratio;
    yMaxMovement = DieHeight * ratio;
}

void RGSolver::setOverlapTolaranceLen(double len) {
    this->overlapTolaranceLen = len;
}