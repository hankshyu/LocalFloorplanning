#ifndef RGPARSER_H
#define RGPARSER_H

#include "rgmodule.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>


struct RGConnStruct {
    std::string m0;
    std::string m1;
    int value;
    RGConnStruct(std::string m0, std::string m1, int value) {
        this->m0 = m0;
        this->m1 = m1;
        this->value = value;
    }
};

class RGParser {
private:
    int DieWidth, DieHeight;
    int softModuleNum, fixedModuleNum, moduleNum, connectionNum;
    std::vector<RGModule> modules;
    std::vector<RGConnStruct> connectionList;
public:
    RGParser();
    RGParser(std::string file_name);
    ~RGParser();
    void read_input(std::string file_name);
    int getDieWidth();
    int getDieHeight();
    int getSoftModuleNum();
    int getFixedModuleNum();
    int getModuleNum();
    int getConnectionNum();
    RGModule getModule(int index);
    RGConnStruct getConnection(int index);
    std::vector<RGConnStruct> getConnectionList() const;

};

#endif