#ifndef PARSER_H
#define PARSER_H

#include "globmodule.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

namespace PushPull {
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
        std::vector<GlobalModule> modules;
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
        GlobalModule getModule(int index);
        ConnStruct getConnection(int index);
        std::vector<ConnStruct> getConnectionList() const;
    };
}


namespace RectGrad {
    struct ConnStruct {
        std::string m0;
        std::string m1;
        int value;
        ConnStruct(std::string m0, std::string m1, int value) {
            this->m0 = m0;
            this->m1 = m1;
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
} // namespace RectGrad

#endif