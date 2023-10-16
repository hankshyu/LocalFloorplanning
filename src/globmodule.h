#ifndef __GLOBMODULE_H__
#define __GLOBMODULE_H__

#include <string>
#include <vector>
#include <iostream>
#include <cmath>

namespace PushPull {
    struct GlobalModule;

    struct Connection {
        GlobalModule* module;
        float value;
    };

    struct GlobalModule {
        std::string name;
        float x, y;
        float area, radius;
        bool fixed;
        float fx, fy, fw, fh;
        std::vector<Connection*> connections;
        GlobalModule(std::string in_name, float in_x, float in_y, float in_area, bool in_fixed);
        ~GlobalModule();
        void addConnection(GlobalModule* in_module, float in_value);
        void addFixedOutline(float fx, float fy, float fw, float fh);
    };
}

namespace RectGrad {
    struct GlobalModule;

    struct Connection {
        GlobalModule *module;
        double value;
    };

    struct GlobalModule {
        std::string name;
        double centerX, centerY;
        int x, y;
        int area;
        int width, height;
        bool fixed;
        std::vector<Connection *> connections;
        GlobalModule(std::string in_name, double centerX, double centerY, int in_area, bool in_fixed);
        GlobalModule(std::string in_name, int x, int y, int width, int height, int in_area, bool in_fixed);
        ~GlobalModule();
        void addConnection(GlobalModule *in_module, double in_value);
        void updateCord(int DieWidth, int DieHeight, double sizeScalar);
    };
}

#endif