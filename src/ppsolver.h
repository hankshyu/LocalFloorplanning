#ifndef PPSOLVER_H
#define PPSOLVER_H

#include "globmodule.h"
#include "parser.h"
#include <vector>
#include <string>

class LFLegaliser;

namespace PushPull {
    class GlobalSolver {
    private:
        friend class ::LFLegaliser;
        float DieWidth, DieHeight;
        int softModuleNum, fixedModuleNum, moduleNum, connectionNum;
        std::vector<GlobalModule*> modules;
        std::vector<float> xForce, yForce;
        float xMaxMovement, yMaxMovement;
        float radiusRatio;
        float pushForce;
    public:
        GlobalSolver();
        ~GlobalSolver();
        void setOutline(int width, int height);
        void setSoftModuleNum(int num);
        void setFixedModuleNum(int num);
        void setConnectionNum(int num);
        void addModule(GlobalModule* in_module);
        void addConnection(std::string ma, std::string mb, float value);
        void readFromParser(Parser& parser);
        void currentPosition2txt(std::string file_name);
        float calcDeadspace();
        void calcModuleForce();
        void moveModule();
        float calcEstimatedHPWL();
        void setRadiusRatio(float ratio);
        void setPushForce(float force);
        void setupPushForce(float amplification = 1.);
    };
}





#endif
