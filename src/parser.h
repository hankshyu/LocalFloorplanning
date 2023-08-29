#ifndef PARSER_H
#define PARSER_H

#include "ppmodule.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>


struct ConnStruct {
    std::string m0;
    std::string m1;
    float value;
    ConnStruct(std::string m0, std::string m1, float value) {
        this->m0 = m0;
        this->m1 = m1;
        this->value = value;
    }
};

class Parser {
private:
    float DieWidth, DieHeight;
    int softModuleNum, fixedModuleNum, moduleNum, connectionNum;
    std::vector<PPModule> modules;
    std::vector<ConnStruct> connectionList;
public:
    Parser();
    Parser(std::string file_name);
    ~Parser();
    void read_input(std::string file_name);
    float getDieWidth();
    float getDieHeight();
    int getSoftModuleNum();
    int getFixedModuleNum();
    int getModuleNum();
    int getConnectionNum();
    PPModule getModule(int index);
    ConnStruct getConnection(int index);
    std::vector<ConnStruct> getConnectionList() const;
};

#endif