#include "ppsolver.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace PushPull {
    GlobalSolver::GlobalSolver() {
        softModuleNum = 0;
        fixedModuleNum = 0;
        radiusRatio = 1;
        pushForce = 1e4;
    }

    GlobalSolver::~GlobalSolver() {
        for ( int i = 0; i < modules.size(); i++ ) {
            delete modules[i];
        }
        //std::cout << "GlobalSolver Deleted Successfully." << std::endl;
    }

    void GlobalSolver::setOutline(int width, int height) {
        DieWidth = (float) width;
        DieHeight = (float) height;
        xMaxMovement = DieWidth / 4000.;
        yMaxMovement = DieHeight / 4000.;
    }

    void GlobalSolver::setSoftModuleNum(int num) {
        softModuleNum = num;
        moduleNum = softModuleNum + fixedModuleNum;
        xForce.resize(moduleNum);
        yForce.resize(moduleNum);
    }

    void GlobalSolver::setFixedModuleNum(int num) {
        fixedModuleNum = num;
        moduleNum = softModuleNum + fixedModuleNum;
        xForce.resize(moduleNum);
        yForce.resize(moduleNum);
    }

    void GlobalSolver::setConnectionNum(int num) {
        connectionNum = num;
    }

    void GlobalSolver::addModule(GlobalModule* in_module) {
        modules.push_back(in_module);
    }

    void GlobalSolver::addConnection(std::string ma, std::string mb, float value) {
        GlobalModule* m0;
        GlobalModule* m1;
        for ( int i = 0; i < modules.size(); i++ ) {
            if ( modules[i]->name == ma )
                m0 = modules[i];
            else if ( modules[i]->name == mb )
                m1 = modules[i];
        }
        m0->addConnection(m1, value);
        m1->addConnection(m0, value);
    }

    void GlobalSolver::readFromParser(Parser& parser) {
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
            GlobalModule* newModule = new GlobalModule(copy.name, copy.x, copy.y, copy.area, copy.fixed);
            if ( copy.fixed )
                newModule->addFixedOutline(copy.fx, copy.fy, copy.fw, copy.fh);
            this->modules.push_back(newModule);
        }
        for ( int i = 0; i < connectionNum; i++ ) {
            ConnStruct conn = parser.getConnection(i);

            GlobalModule* m0;
            GlobalModule* m1;
            for ( int i = 0; i < modules.size(); i++ ) {
                if ( modules[i]->name == conn.m0 )
                    m0 = modules[i];
                else if ( modules[i]->name == conn.m1 )
                    m1 = modules[i];
            }
            m0->addConnection(m1, (float) conn.value);
            m1->addConnection(m0, (float) conn.value);
        }
    }

    void GlobalSolver::currentPosition2txt(std::string file_name) {
        std::ofstream ostream(file_name);
        ostream << "BLOCK " << moduleNum << " CONNECTOIN " << connectionNum << std::endl;
        ostream << DieWidth << " " << DieHeight << std::endl;
        for ( int i = 0; i < moduleNum; i++ ) {
            ostream << modules[i]->name << " ";
            ostream << ( ( modules[i]->fixed ) ? "FIXED" : "SOFT" ) << " ";
            if ( modules[i]->fixed ) {
                ostream << modules[i]->fx << " " << modules[i]->fy << " ";
                ostream << modules[i]->fw << " " << modules[i]->fh << std::endl;
            }
            else {
                ostream << modules[i]->x << " " << modules[i]->y << " ";
                ostream << modules[i]->radius * radiusRatio << std::endl;
            }
        }
        std::vector<GlobalModule*> added;
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

    float GlobalSolver::calcDeadspace() {
        float dieArea = DieWidth * DieHeight;
        float moduleArea = 0;
        for ( int i = 0; i < moduleNum; i++ ) {
            moduleArea += modules[i]->area;
        }
        return 1. - moduleArea / dieArea;
    }

    float calcDistance(GlobalModule* ma, GlobalModule* mb) {
        float x1 = ma->x, x2 = mb->x;
        float y1 = ma->y, y2 = mb->y;
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }

    void calcSeg2PntDist(float s1x, float s1y, float s2x, float s2y, float px, float py, float* distance, float* angle) {
        float APAB = ( s2x - s1x ) * ( px - s1x ) + ( s2y - s1y ) * ( py - s1y );
        if ( APAB <= 0 ) {
            *distance = std::sqrt(std::pow(( px - s1x ), 2) + std::pow(( py - s1y ), 2));
            *angle = std::atan2(s1y - py, s1x - px);
            return;
        }

        float length = std::sqrt(std::pow(( s1x - s2x ), 2) + std::pow(( s1y - s2y ), 2));
        float AB2 = std::pow(length, 2);
        if ( APAB >= AB2 ) {
            *distance = std::sqrt(std::pow(( px - s2x ), 2) + std::pow(( py - s2y ), 2));
            *angle = std::atan2(s2y - py, s2x - px);
            return;
        }

        float r = APAB / AB2;
        float Cx = s1x + ( s2x - s1x ) * r;
        float Cy = s1y + ( s2y - s1y ) * r;
        *distance = std::sqrt(std::pow(( px - Cx ), 2) + std::pow(( py - Cy ), 2));
        *angle = std::atan2(Cy - py, Cx - px);
    }

    void GlobalSolver::calcModuleForce() {
        GlobalModule* curModule;
        for ( int i = 0; i < moduleNum; i++ ) {
            if ( modules[i]->fixed == true )
                continue;

            curModule = modules[i];
            float x_force = 0;
            float y_force = 0;

            for ( int j = 0; j < curModule->connections.size(); j++ ) {
                GlobalModule* pullModule = curModule->connections[j]->module;
                float pullValue = curModule->connections[j]->value;
                float distance, x_distance, y_distance;

                if ( pullModule->fixed == true ) {
                    float fx = pullModule->fx;
                    float fy = pullModule->fy;
                    float fw = pullModule->fw;
                    float fh = pullModule->fh;
                    float d[4], a[4];
                    calcSeg2PntDist(fx, fy, fx, fy + fh, curModule->x, curModule->y, d, a);
                    calcSeg2PntDist(fx, fy + fh, fx + fw, fy + fh, curModule->x, curModule->y, d + 1, a + 1);
                    calcSeg2PntDist(fx + fw, fy, fx + fw, fy + fh, curModule->x, curModule->y, d + 2, a + 2);
                    calcSeg2PntDist(fx, fy, fx + fw, fy, curModule->x, curModule->y, d + 3, a + 3);

                    float minD = d[0], angle = a[0];
                    for ( int m = 1; m < 4; m++ )
                        if ( d[m] < minD ) {
                            minD = d[m];
                            angle = a[m];
                        }

                    distance = minD;

                    float curModuleRadius = curModule->radius * radiusRatio;
                    if ( distance <= curModuleRadius )
                        continue;

                    distance -= curModuleRadius;
                    x_distance = distance * std::cos(angle);
                    y_distance = distance * std::sin(angle);
                }
                else {
                    x_distance = pullModule->x - curModule->x;
                    y_distance = pullModule->y - curModule->y;
                    if ( x_distance == 0 && y_distance == 0 )
                        continue;

                    float angle = std::atan2(y_distance, x_distance);
                    float curModuleRadius = curModule->radius * radiusRatio;
                    float pullModuleRadius = pullModule->radius * radiusRatio;
                    float distance = calcDistance(curModule, pullModule);
                    if ( distance <= curModuleRadius + pullModuleRadius )
                        continue;

                    distance -= curModuleRadius + pullModuleRadius;
                    //distance = std::abs(x_distance) + std::abs(y_distance) - curModuleRadius - pullModuleRadius;
                    //std::cout << angle << std::endl;
                    //float scale = std::abs(x_distance - y_distance);
                    x_distance = distance * std::cos(angle);
                    y_distance = distance * std::sin(angle);
                }

                float force = distance * pullValue;
                x_force += pullValue * x_distance;
                y_force += pullValue * y_distance;

            }

            for ( int j = 0; j < moduleNum; j++ ) {
                if ( j == i )
                    continue;
                GlobalModule* pushModule = modules[j];
                float distance, x_distance, y_distance;

                if ( pushModule->fixed ) {
                    float fx = pushModule->fx;
                    float fy = pushModule->fy;
                    float fw = pushModule->fw;
                    float fh = pushModule->fh;
                    float d[4], a[4];
                    calcSeg2PntDist(fx, fy, fx, fy + fh, curModule->x, curModule->y, d, a);
                    calcSeg2PntDist(fx, fy + fh, fx + fw, fy + fh, curModule->x, curModule->y, d + 1, a + 1);
                    calcSeg2PntDist(fx + fw, fy, fx + fw, fy + fh, curModule->x, curModule->y, d + 2, a + 2);
                    calcSeg2PntDist(fx, fy, fx + fw, fy, curModule->x, curModule->y, d + 3, a + 3);

                    float minD = d[0], angle = a[0];
                    for ( int m = 1; m < 4; m++ )
                        if ( d[m] < minD ) {
                            minD = d[m];
                            angle = a[m];
                        }

                    distance = minD;
                    float curModuleRadius = curModule->radius * radiusRatio;
                    if ( distance >= curModuleRadius )
                        continue;

                    distance -= curModuleRadius;
                    distance = -distance;
                    x_distance = distance * std::cos(angle);
                    y_distance = distance * std::sin(angle);
                }
                else {
                    x_distance = pushModule->x - curModule->x;
                    y_distance = pushModule->y - curModule->y;
                    if ( x_distance == 0 && y_distance == 0 )
                        continue;

                    float angle = std::atan2(y_distance, x_distance);
                    float curModuleRadius = curModule->radius * radiusRatio;
                    float pushModuleRadius = pushModule->radius * radiusRatio;
                    distance = calcDistance(curModule, pushModule);
                    if ( distance >= curModuleRadius + pushModuleRadius )
                        continue;

                    distance -= curModuleRadius + pushModuleRadius;
                    distance = -distance;
                    x_distance = distance * std::cos(angle);
                    y_distance = distance * std::sin(angle);
                }

                float force = pushForce * distance;
                x_force -= pushForce * x_distance;
                y_force -= pushForce * y_distance;

            }

            xForce[i] = x_force;
            yForce[i] = y_force;
        }

        //for ( int i = 0; i < moduleNum; i++ ) {
        //    std::cout << modules[i]->name << ": " << xForce[i] << " " << yForce[i] << std::endl;
        //}
    }

    void GlobalSolver::moveModule() {
        // find the maximum force in x, y direction
        float xMax = 0, yMax = 0;
        for ( int i = 1; i < moduleNum; i++ ) {
            if ( std::abs(xForce[i]) > xMax )
                xMax = std::abs(xForce[i]);
            if ( std::abs(yForce[i]) > yMax )
                yMax = std::abs(yForce[i]);
        }

        // scale forces if they are out of maximum tolerance value
        float xRatio = 1, yRatio = 1;

        if ( xMax > xMaxMovement )
            xRatio = xMaxMovement / xMax;
        if ( yMax > yMaxMovement )
            yRatio = yMaxMovement / yMax;

        // move soft modules
        GlobalModule* curModule;
        for ( int i = 0; i < moduleNum; i++ ) {
            if ( modules[i]->fixed == true )
                continue;

            curModule = modules[i];

            curModule->x += xForce[i] * xRatio;
            curModule->y += yForce[i] * yRatio;

            if ( curModule->x < curModule->radius )
                curModule->x = curModule->radius;
            if ( curModule->y < curModule->radius )
                curModule->y = curModule->radius;
            if ( curModule->x > DieWidth - curModule->radius )
                curModule->x = DieWidth - curModule->radius;
            if ( curModule->y > DieHeight - curModule->radius )
                curModule->y = DieHeight - curModule->radius;
        }
    }

    float GlobalSolver::calcEstimatedHPWL() {
        float HPWL = 0;
        for ( int i = 0; i < moduleNum; i++ ) {
            GlobalModule* curModule = modules[i];
            for ( int j = 0; j < curModule->connections.size(); j++ ) {
                GlobalModule* conModule = curModule->connections[j]->module;
                float value = curModule->connections[j]->value;
                float x_diff = std::abs(curModule->x - conModule->x);
                float y_diff = std::abs(curModule->y - conModule->y);
                HPWL += ( x_diff + y_diff ) * value;
            }
        }
        return HPWL / 2.;
    }

    void GlobalSolver::setRadiusRatio(float ratio) {
        radiusRatio = ratio;
    }

    void GlobalSolver::setPushForce(float force) {
        pushForce = force;
    }

    void GlobalSolver::setupPushForce(float amplification) {
        float maxForce = 0;
        for ( int i = 0; i < moduleNum; i++ ) {
            GlobalModule* curModule = modules[i];
            float forces = 0;
            for ( int j = 0; j < curModule->connections.size(); j++ ) {
                forces += curModule->connections[j]->value;
            }
            if ( forces > maxForce ) {
                maxForce = forces;
            }
        }
        pushForce = maxForce * amplification;
        std::cout << "Set push force = " << pushForce << std::endl;
    }
}