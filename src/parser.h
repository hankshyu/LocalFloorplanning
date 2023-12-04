#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

struct GlobalModule {
    std::string name;
    int x, y;
    int area;
    int width, height;
    bool fixed;
    GlobalModule(std::string in_name, int x, int y, int width, int height, int in_area, bool in_fixed) {
        this->name = in_name;
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->area = in_area;
        this->fixed = in_fixed;
    }
    ~GlobalModule() {}
};

struct ConnStruct {
    std::vector<std::string> modules;
    int value;
    ConnStruct(const std::vector<std::string> &modules, int value) {
        this->modules = modules;
        this->value = value;
    }
};

class Parser {
private:
    int DieWidth, DieHeight;
    int softModuleNum, fixedModuleNum, moduleNum, connectionNum;
    std::vector<GlobalModule> modules;
    std::vector<ConnStruct> connectionList;
public:
    Parser();
    Parser(std::string file_name);
    ~Parser();
    void read_input(std::string file_name);
    int getDieWidth();
    int getDieHeight();
    int getSoftModuleNum();
    int getFixedModuleNum();
    int getModuleNum();
    int getConnectionNum();
    GlobalModule getModule(int index);
    ConnStruct getConnection(int index);
    std::vector<ConnStruct> getConnectionList() const;
};

#endif