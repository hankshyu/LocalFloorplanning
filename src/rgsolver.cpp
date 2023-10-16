#include "rgsolver.h"

namespace RectGrad {
    GlobalSolver::GlobalSolver() {
        std::srand(std::time(NULL));
        softModuleNum = 0;
        fixedModuleNum = 0;
        sizeScalar = 1;
        punishment = 1e4;
        overlapTolaranceLen = 0;
        pullWhileOverlap = false;
        overlapped = true;
        timeStep = 0;
    }

    GlobalSolver::~GlobalSolver() {
        for ( int i = 0; i < modules.size(); i++ ) {
            delete modules[i];
        }
    }

    void GlobalSolver::setOutline(int width, int height) {
        DieWidth = (double) width;
        DieHeight = (double) height;
        xMaxMovement = DieWidth / 4000.;
        yMaxMovement = DieHeight / 4000.;
    }

    void GlobalSolver::setSoftModuleNum(int num) {
        softModuleNum = num;
        moduleNum = softModuleNum + fixedModuleNum;
        xGradient.resize(moduleNum);
        yGradient.resize(moduleNum);
        xFirstMoment.resize(moduleNum, 0.);
        yFirstMoment.resize(moduleNum, 0.);
        xSecondMoment.resize(moduleNum, 0.);
        ySecondMoment.resize(moduleNum, 0.);
    }

    void GlobalSolver::setFixedModuleNum(int num) {
        fixedModuleNum = num;
        moduleNum = softModuleNum + fixedModuleNum;
        xGradient.resize(moduleNum);
        yGradient.resize(moduleNum);
        xFirstMoment.resize(moduleNum, 0.);
        yFirstMoment.resize(moduleNum, 0.);
        xSecondMoment.resize(moduleNum, 0.);
        ySecondMoment.resize(moduleNum, 0.);
    }

    void GlobalSolver::setConnectionNum(int num) {
        connectionNum = num;
    }

    void GlobalSolver::addModule(GlobalModule *in_module) {
        modules.push_back(in_module);
    }

    void GlobalSolver::addConnection(std::string ma, std::string mb, double value) {
        GlobalModule *m0;
        GlobalModule *m1;
        for ( int i = 0; i < modules.size(); i++ ) {
            if ( modules[i]->name == ma )
                m0 = modules[i];
            else if ( modules[i]->name == mb )
                m1 = modules[i];
        }
        m0->addConnection(m1, value);
        m1->addConnection(m0, value);
    }

    void GlobalSolver::readFromParser(Parser parser) {
        setOutline(parser.getDieWidth(), parser.getDieHeight());
        setSoftModuleNum(parser.getSoftModuleNum());
        setFixedModuleNum(parser.getFixedModuleNum());
        setConnectionNum(parser.getConnectionNum());
        for ( int i = 0; i < this->modules.size(); i++ ) {
            delete this->modules[i];
        }
        this->modules.clear();
        for ( int i = 0; i < this->moduleNum; i++ ) {
            GlobalModule copy = parser.getModule(i);
            GlobalModule *newModule;
            if ( copy.fixed ) {
                newModule = new GlobalModule(copy.name, copy.x, copy.y, copy.width, copy.height, copy.area, copy.fixed);
            }
            else {
                newModule = new GlobalModule(copy.name, copy.centerX, copy.centerY, copy.area, copy.fixed);
            }
            this->modules.push_back(newModule);
        }
        double scalar = -1.;
        for ( int i = 0; i < connectionNum; i++ ) {
            ConnStruct conn = parser.getConnection(i);
            if ( (double) conn.value > scalar ) {
                scalar = (double) conn.value;
            }
        }
        connectNormalize = 1. / scalar;
        for ( int i = 0; i < connectionNum; i++ ) {
            ConnStruct conn = parser.getConnection(i);

            GlobalModule *m0;
            GlobalModule *m1;
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

    void GlobalSolver::currentPosition2txt(Parser parser, std::string file_name) {
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
        for ( int i = 0; i < connectionNum; i++ ) {
            ConnStruct conn = parser.getConnection(i);
            ostream << conn.m0 << " ";
            ostream << conn.m1 << " ";
            ostream << conn.value << std::endl;
        }
        ostream.close();
    }

    double GlobalSolver::calcDeadspace() {
        double dieArea = DieWidth * DieHeight;
        double moduleArea = 0;
        for ( int i = 0; i < moduleNum; i++ ) {
            moduleArea += modules[i]->area;
        }
        return 1. - moduleArea / dieArea;
    }

    void GlobalSolver::calcGradient() {
        GlobalModule *curModule;
        for ( int i = 0; i < moduleNum; i++ ) {
            if ( modules[i]->fixed == true ) {
                continue;
            }

            curModule = modules[i];
            double x_grad = 0;
            double y_grad = 0;

            // gradient for HPWL
            for ( int j = 0; j < curModule->connections.size(); j++ ) {
                GlobalModule *pullModule = curModule->connections[j]->module;
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
                GlobalModule *pushModule = modules[j];
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

                if ( overlappedWidth > curModule->width ) {
                    overlappedWidth = (double) curModule->width;
                }
                else if ( overlappedWidth > pushModule->width ) {
                    overlappedWidth = (double) pushModule->width;
                }

                if ( overlappedHeight > curModule->height ) {
                    overlappedHeight = (double) curModule->height;
                }
                else if ( overlappedHeight > pushModule->height ) {
                    overlappedHeight = (double) pushModule->height;
                }

                double x_sign = ( x_diff == 0 ) ? 0 : ( x_diff > 0 ) ? 1. : -1.;
                double y_sign = ( y_diff == 0 ) ? 0 : ( y_diff > 0 ) ? 1. : -1.;
                // x_grad += -punishment * x_sign * overlappedHeight / (curModule->width * curModule->height);
                // y_grad += -punishment * y_sign * overlappedWidth / (curModule->width * curModule->height);
                x_grad += -punishment * x_sign * overlappedHeight;
                y_grad += -punishment * y_sign * overlappedWidth;

            }

            xGradient[i] = x_grad;
            yGradient[i] = y_grad;
        }

        //for ( int i = 0; i < moduleNum; i++ ) {
        //    std::cout << modules[i]->name << ": " << xGradient[i] << " " << yGradient[i] << std::endl;
        //}
    }

    void GlobalSolver::gradientDescent(double lr) {
        timeStep += 1;
        // move soft modules
        GlobalModule *curModule;
        for ( int i = 0; i < moduleNum; i++ ) {
            if ( modules[i]->fixed == true )
                continue;

            curModule = modules[i];

            xFirstMoment[i] = 0.9 * xFirstMoment[i] + 0.1 * xGradient[i];
            yFirstMoment[i] = 0.9 * yFirstMoment[i] + 0.1 * yGradient[i];

            xSecondMoment[i] = 0.999 * xSecondMoment[i] + 0.001 * xGradient[i] * xGradient[i];
            ySecondMoment[i] = 0.999 * ySecondMoment[i] + 0.001 * yGradient[i] * yGradient[i];

            double xFirstMomentHat = xFirstMoment[i] / (1 - std::pow(0.9, timeStep));
            double yFirstMomentHat = yFirstMoment[i] / (1 - std::pow(0.9, timeStep));

            double xSecondMomentHat = xSecondMoment[i] / (1 - std::pow(0.999, timeStep));
            double ySecondMomentHat = ySecondMoment[i] / (1 - std::pow(0.999, timeStep));

            double xMovement = ( lr * xFirstMomentHat / ( std::sqrt(xSecondMomentHat) + 1e-8 )) * DieWidth;
            double yMovement = ( lr * yFirstMomentHat / ( std::sqrt(ySecondMomentHat) + 1e-8 )) * DieHeight;

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

    double GlobalSolver::calcEstimatedHPWL() {
        double HPWL = 0;
        for ( int i = 0; i < moduleNum; i++ ) {
            GlobalModule *curModule = modules[i];
            for ( int j = 0; j < curModule->connections.size(); j++ ) {
                GlobalModule *conModule = curModule->connections[j]->module;
                double value = curModule->connections[j]->value;
                double x_diff = std::abs(curModule->centerX - conModule->centerX);
                double y_diff = std::abs(curModule->centerY - conModule->centerY);
                HPWL += ( x_diff + y_diff ) * value;
            }
        }
        return HPWL / 2.;
    }

    void GlobalSolver::setSizeScalar(double scalar) {
        sizeScalar = scalar;
    }

    void GlobalSolver::setPunishment(double force) {
        punishment = force;
    }

    void GlobalSolver::setupPunishment(double amplification) {
        double maxConnection = 0;
        for ( int i = 0; i < moduleNum; i++ ) {
            GlobalModule *curModule = modules[i];
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

    void GlobalSolver::setMaxMovement(double ratio) {
        xMaxMovement = DieWidth * ratio;
        yMaxMovement = DieHeight * ratio;
    }

    void GlobalSolver::setOverlapTolaranceLen(double len) {
        this->overlapTolaranceLen = len;
    }

    void GlobalSolver::setPullWhileOverlap(bool onoff) {
        this->pullWhileOverlap = onoff;
    }

    bool GlobalSolver::hasOverlap() {
        for ( auto &mod : modules ) {
            mod->updateCord(DieWidth, DieHeight, 1.);
        }
        overlapped = false;
        for ( int i = 0; i < moduleNum - 1; i++ ) {
            for ( int j = i + 1; j < moduleNum; j++ ) {
                GlobalModule *mod1 = modules[i];
                GlobalModule *mod2 = modules[j];
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

    void GlobalSolver::reportOverlap() {
        for ( int i = 0; i < moduleNum - 1; i++ ) {
            for ( int j = i + 1; j < moduleNum; j++ ) {
                GlobalModule *mod1 = modules[i];
                GlobalModule *mod2 = modules[j];
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

    void GlobalSolver::squeezeToFit() {
        for ( auto &mod : modules ) {
            mod->updateCord(DieWidth, DieHeight, 1.);
        }

        std::vector<int> squeezeWidthVec(this->moduleNum, 0);
        std::vector<int> squeezeHeightVec(this->moduleNum, 0);

        for ( int i = 0; i < this->moduleNum; ++i ) {
            GlobalModule *curModule = modules[i];
            if ( curModule->fixed ) {
                continue;
            }
            int totalOverlapWidth = 0.0;
            int totalOverlapHeight = 0.0;
            for ( GlobalModule *tarModule : modules ) {
                if ( curModule == tarModule ) {
                    continue;
                }
                int overlappedWidth, overlappedHeight, x_diff, y_diff;

                x_diff = curModule->centerX - tarModule->centerX;
                y_diff = curModule->centerY - tarModule->centerY;

                overlappedWidth = ( curModule->width + tarModule->width ) / 2.0 - std::abs(x_diff);
                overlappedHeight = ( curModule->height + tarModule->height ) / 2.0 - std::abs(y_diff);

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
                    // std::cout << curModule->centerX << " " << tarModule->centerX << " ";
                    // std::cout << curModule->width << " " << tarModule->width << std::endl;
                    // std::cout << overlappedWidth << std::endl;
                    totalOverlapHeight += overlappedHeight;
                }
            }
            if ( totalOverlapWidth > 0. && totalOverlapHeight > 0. ) {
                double aspectRatio = (double) totalOverlapHeight / totalOverlapWidth;
                if ( aspectRatio > 10. ) {
                    int squeezeWidth = totalOverlapWidth;
                    // std::cout << "Width: " << squeezeWidth << "\n";
                    // curModule->width -= squeezeWidth;
                    // curModule->height = std::ceil(curModule->area / curModule->width);
                    squeezeWidthVec[i] = squeezeWidth;
                }
                else if ( aspectRatio < 0.1 ) {
                    int squeezeHeight = totalOverlapHeight;
                    std::cout << "Height: " << squeezeHeight << "\n";
                    // curModule->height -= squeezeHeight;
                    // curModule->width = std::ceil(curModule->area / curModule->height);
                    squeezeHeightVec[i] = squeezeHeight;
                }
                // else {
                //     aspectRatio = 0.2 * std::atan(aspectRatio - 1) + 1;
                //     if ( aspectRatio > 2. ) {
                //         aspectRatio = 2.;
                //     }
                //     else if ( aspectRatio < 0.5 ) {
                //         aspectRatio = 0.5;
                //     }
                //     curModule->width = std::ceil(std::sqrt(curModule->area / aspectRatio));
                //     curModule->height = std::ceil(std::sqrt(curModule->area * aspectRatio));
                // }
            }
        }
        for ( int i = 0; i < this->moduleNum; ++i ) {
            GlobalModule *curModule = modules[i];
            if ( curModule->fixed ) {
                continue;
            }
            if ( squeezeWidthVec[i] > 0. ) {
                curModule->width -= squeezeWidthVec[i];
                curModule->height = std::ceil((double) curModule->area / (double) curModule->width);
            }
            else if ( squeezeHeightVec[i] > 0. ) {
                curModule->height -= squeezeHeightVec[i];
                curModule->width = std::ceil((double) curModule->area / (double) curModule->height);
            }
            assert(curModule->height * curModule->width >= curModule->area);
            curModule->updateCord(DieWidth, DieHeight, 1.);
        }
    }

    bool GlobalSolver::isAreaLegal() {
        for ( auto &mod : modules ) {
            if ( mod->fixed ) {
                continue;
            }
            if ( mod->width * mod->height < mod->area ) {
                return false;
            }
        }
        return true;
    }

    void GlobalSolver::resetOptimizer() {
        xFirstMoment.resize(moduleNum, 0.);
        yFirstMoment.resize(moduleNum, 0.);
        xSecondMoment.resize(moduleNum, 0.);
        ySecondMoment.resize(moduleNum, 0.);
        timeStep = 0;
    }
} // namespace RectGrad