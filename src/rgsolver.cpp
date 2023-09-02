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
    momentum = 0;
    pullWhileOverlap = false;
    overlapped = true;
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
    xGradientPrev.resize(moduleNum);
    yGradientPrev.resize(moduleNum);
}

void RGSolver::setFixedModuleNum(int num) {
    fixedModuleNum = num;
    moduleNum = softModuleNum + fixedModuleNum;
    xGradient.resize(moduleNum);
    yGradient.resize(moduleNum);
    xGradientPrev.resize(moduleNum);
    yGradientPrev.resize(moduleNum);
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
            overlappedWidth = ( curWidth + pushWidth ) / 2.0 - std::abs(x_diff);
            overlappedHeight = ( curHeight + pushHeight ) / 2.0 - std::abs(y_diff);

            if ( overlappedWidth > overlapTolaranceLen && overlappedHeight > overlapTolaranceLen && !pullWhileOverlap ) {
                continue;
            }

            double x_sign = ( x_diff == 0 ) ? 0. : ( x_diff > 0 ) ? 1. : -1.;
            double y_sign = ( y_diff == 0 ) ? 0. : ( y_diff > 0 ) ? 1. : -1.;
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
            overlappedWidth = ( curWidth + pushWidth ) / 2.0 - std::abs(x_diff);
            overlappedHeight = ( curHeight + pushHeight ) / 2.0 - std::abs(y_diff);
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
    // saturated = true;
    // move soft modules
    RGModule *curModule;
    for ( int i = 0; i < moduleNum; i++ ) {
        if ( modules[i]->fixed == true )
            continue;

        curModule = modules[i];

        // double xMovement = ( xGradient[i] + xGradientPrev[i] ) * lr * DieWidth;
        // double yMovement = ( yGradient[i] + yGradientPrev[i] ) * lr * DieHeight;

        double xMovement = ( xGradient[i] ) * lr * DieWidth;
        double yMovement = ( yGradient[i] ) * lr * DieHeight;

        xGradientPrev[i] = momentum * xGradient[i];
        yGradientPrev[i] = momentum * yGradient[i];

        // if ( xGradient[i] > 1. || yGradient[i] > 1. ) {
        //     saturated = false;
        // }

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

bool RGSolver::isSaturated() {
    // return saturated;
    return false;
}

void RGSolver::setMomentum(double momentum) {
    this->momentum = momentum;
}

void RGSolver::setPullWhileOverlap(bool onoff) {
    this->pullWhileOverlap = onoff;
}

bool RGSolver::hasOverlap() {
    for ( auto &mod : modules ) {
        mod->updateCord(DieWidth, DieHeight, 1.);
    }
    overlapped = false;
    for ( int i = 0; i < moduleNum - 1; i++ ) {
        for ( int j = i + 1; j < moduleNum; j++ ) {
            RGModule *mod1 = modules[i];
            RGModule *mod2 = modules[j];
            double overlappedWidth, overlappedHeight, x_diff, y_diff;

            x_diff = mod1->centerX - mod2->centerX;
            y_diff = mod1->centerY - mod2->centerY;

            overlappedWidth = ( mod1->width + mod2->width ) / 2.0 - std::abs(x_diff);
            overlappedHeight = ( mod1->height + mod2->height ) / 2.0 - std::abs(y_diff);

            if ( overlappedWidth > 0. && overlappedHeight > 0. ) {
                overlapped = true;
            }
        }
    }
    return overlapped;
}

void RGSolver::reportOverlap() {
    for ( int i = 0; i < moduleNum - 1; i++ ) {
        for ( int j = i + 1; j < moduleNum; j++ ) {
            RGModule *mod1 = modules[i];
            RGModule *mod2 = modules[j];
            double overlappedWidth, overlappedHeight, x_diff, y_diff;

            x_diff = mod1->centerX - mod2->centerX;
            y_diff = mod1->centerY - mod2->centerY;

            overlappedWidth = ( mod1->width + mod2->width ) / 2.0 - std::abs(x_diff);
            overlappedHeight = ( mod1->height + mod2->height ) / 2.0 - std::abs(y_diff);

            if ( overlappedWidth > 0. && overlappedHeight > 0. ) {
                std::cout << "Overlap: " << mod1->name << " & " << mod2->name << " : " << overlappedWidth * overlappedHeight << std::endl;
            }
        }
    }
}

void RGSolver::squeezeToFit() {
    for ( auto &mod : modules ) {
        mod->updateCord(DieWidth, DieHeight, 1.);
    }

    std::vector<int> squeezeWidthVec(this->moduleNum, 0);
    std::vector<int> squeezeHeightVec(this->moduleNum, 0);

    for ( int i = 0; i < this->moduleNum; ++i ) {
        RGModule *curModule = modules[i];
        if ( curModule->fixed ) {
            continue;
        }
        int totalOverlapWidth = 0;
        int totalOverlapHeight = 0;
        for ( RGModule *tarModule : modules ) {
            if ( curModule == tarModule ) {
                continue;
            }
            int overlappedWidth, overlappedHeight, x_diff, y_diff;

            x_diff = curModule->centerX - tarModule->centerX;
            y_diff = curModule->centerY - tarModule->centerY;

            overlappedWidth = (int) (( curModule->width + tarModule->width ) / 2.0 - std::abs(x_diff));
            overlappedHeight = (int) (( curModule->height + tarModule->height ) / 2.0 - std::abs(y_diff));

            if ( overlappedWidth > curModule->width ) {
                overlappedWidth = curModule->width;
            }
            else if ( overlappedWidth > tarModule->width ) {
                overlappedWidth = tarModule->width;
            }

            if ( overlappedHeight > curModule->height ) {
                overlappedHeight = curModule->height;
            }
            else if ( overlappedHeight > tarModule->height ) {
                overlappedHeight = tarModule->height;
            }

            if ( overlappedWidth > 0. && overlappedHeight > 0. ) {
                totalOverlapWidth += overlappedWidth;
                totalOverlapHeight += overlappedHeight;
            }
        }
        if ( totalOverlapWidth > 0. && totalOverlapHeight > 0. ) {
            double aspectRatio = (double) totalOverlapHeight / totalOverlapWidth;
            if ( aspectRatio > 5. ) {
                squeezeWidthVec[i] += totalOverlapWidth;
            }
            else if ( aspectRatio < 0.2 ) {
                squeezeHeightVec[i] += totalOverlapHeight;
            }
        }
    }
    for ( int i = 0; i < this->moduleNum; ++i ) {
        RGModule *curModule = modules[i];
        if ( curModule->fixed ) {
            continue;
        }
        if ( squeezeWidthVec[i] > 0. ) {
            curModule->width -= squeezeWidthVec[i];
            curModule->height = std::ceil((double) curModule->area / curModule->width);
        }
        else if ( squeezeHeightVec[i] > 0. ) {
            curModule->height -= squeezeHeightVec[i];
            curModule->width = std::ceil((double) curModule->area / curModule->height);
        }
        curModule->updateCord(DieWidth, DieHeight, 1.);
    }
}