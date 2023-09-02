#ifndef __RGMODULE_H__
#define __RGMODULE_H__

#include <string>
#include <vector>

struct RGModule;

struct RGConnection {
    RGModule *module;
    double value;
};

struct RGModule {
    std::string name;
    double centerX, centerY;
    int x, y;
    int area;
    int width, height;
    bool fixed;
    std::vector<RGConnection *> connections;
    RGModule(std::string in_name, double centerX, double centerY, int in_area, bool in_fixed);
    RGModule(std::string in_name, int x, int y, int width, int height, int in_area, bool in_fixed);
    ~RGModule();
    void addConnection(RGModule *in_module, double in_value);
    void updateCord(int DieWidth, int DieHeight, double sizeScalar);
};

#endif