#ifndef __RGSOLVER_H__
#define __RGSOLVER_H__

#include "rgmodule.h"
#include "rgparser.h"
#include <vector>
#include <string>

class RGSolver {
private:
    friend class LFLegaliser;
    double DieWidth, DieHeight;
    int softModuleNum, fixedModuleNum, moduleNum, connectionNum;
    std::vector<RGModule *> modules;
    std::vector<double> xGradient, yGradient;
    std::vector<double> xGradientPrev, yGradientPrev;
    double xMaxMovement, yMaxMovement;
    double sizeScalar;
    double punishment;
    double connectNormalize;
    double overlapTolaranceLen;
    double momentum;
    bool saturated;
    bool pullWhileOverlap;
    bool overlapped;
public:
    RGSolver();
    ~RGSolver();
    void setOutline(int width, int height);
    void setSoftModuleNum(int num);
    void setFixedModuleNum(int num);
    void setConnectionNum(int num);
    void addModule(RGModule *in_module);
    void addConnection(std::string ma, std::string mb, double value);
    void readFromParser(RGParser parser);
    void currentPosition2txt(std::string file_name);
    double calcDeadspace();
    void calcGradient();
    void gradientDescent(double lr);
    double calcEstimatedHPWL();
    void setSizeScalar(double scalar);
    void setPunishment(double force);
    void setupPunishment(double amplification = 1.);
    void setMaxMovement(double ratio = 0.001);
    void setOverlapTolaranceLen(double len);
    bool isSaturated();
    void setMomentum(double momentum);
    void setPullWhileOverlap(bool onoff);
    bool hasOverlap();
    void reportOverlap();
    void squeezeToFit();
    bool isAreaLegal();
};



#endif
