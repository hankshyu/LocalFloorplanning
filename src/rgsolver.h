#ifndef RGSOLVER_H
#define RGSOLVER_H

#include "globmodule.h"
#include "parser.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cassert>

class LFLegaliser;

namespace RectGrad {
    class GlobalSolver {
    private:
        friend class ::LFLegaliser;
        double DieWidth, DieHeight;
        int softModuleNum, fixedModuleNum, moduleNum, connectionNum;
        std::vector<GlobalModule *> modules;
        std::vector<double> xGradient, yGradient;
        std::vector<double> xFirstMoment, yFirstMoment;
        std::vector<double> xSecondMoment, ySecondMoment;
        double xMaxMovement, yMaxMovement;
        double sizeScalar;
        double punishment;
        double connectNormalize;
        double overlapTolaranceLen;
        bool saturated;
        bool pullWhileOverlap;
        bool overlapped;
        int timeStep;
    public:
        GlobalSolver();
        ~GlobalSolver();
        void setOutline(int width, int height);
        void setSoftModuleNum(int num);
        void setFixedModuleNum(int num);
        void setConnectionNum(int num);
        void addModule(GlobalModule *in_module);
        void addConnection(std::string ma, std::string mb, double value);
        void readFromParser(Parser parser);
        void currentPosition2txt(Parser parser, std::string file_name);
        double calcDeadspace();
        void calcGradient();
        void gradientDescent(double lr);
        double calcEstimatedHPWL();
        void setSizeScalar(double scalar);
        void setPunishment(double force);
        void setupPunishment(double amplification = 1.);
        void setMaxMovement(double ratio = 0.001);
        void setOverlapTolaranceLen(double len);
        void setPullWhileOverlap(bool onoff);
        bool hasOverlap();
        void reportOverlap();
        void squeezeToFit();
        bool isAreaLegal();
        void resetOptimizer();
    };
} // namespace RectGrad


#endif
