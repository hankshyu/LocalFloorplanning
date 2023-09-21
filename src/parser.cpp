#include "parser.h"

namespace PushPull {
    Parser::Parser() {
        this->softModuleNum = 0;
        this->fixedModuleNum = 0;
        this->moduleNum = 0;
    }

    Parser::Parser(std::string file_name) {
        this->softModuleNum = 0;
        this->fixedModuleNum = 0;
        this->moduleNum = 0;
        this->read_input(file_name);
    }

    Parser::~Parser() {
        //std::cout << "Parser Deleted Successfully." << std::endl;
    }

    void Parser::read_input(std::string file_name) {
        std::ifstream istream(file_name);
        if ( istream.fail() ) {
            std::cout << file_name << " doesn't exist.\n";
            return;
        }
        std::string s, ma, mb;
        int area, x, y, w, h, value;

        istream >> s >> this->DieWidth >> this->DieHeight;
        istream >> s >> this->softModuleNum;

        for ( int i = 0; i < this->softModuleNum; i++ ) {
            istream >> s >> area;
            GlobalModule mod(s, this->DieWidth / 2., this->DieHeight / 2., (float) area, false);
            modules.push_back(mod);
            //std::cout << "Reading Soft Module " << s << "..." << std::endl;
        }

        istream >> s >> this->fixedModuleNum;

        for ( int i = 0; i < fixedModuleNum; i++ ) {
            istream >> s >> x >> y >> w >> h;
            GlobalModule mod(s, x + w / 2., y + h / 2., (float) w * h, true);
            mod.addFixedOutline(x, y, w, h);
            modules.push_back(mod);
            //std::cout << "Reading Fixed Module " << s << "..." << std::endl;
        }

        this->moduleNum = this->softModuleNum + this->fixedModuleNum;

        istream >> s >> this->connectionNum;

        for ( int i = 0; i < connectionNum; i++ ) {
            istream >> ma >> mb >> value;
            ConnStruct conn(ma, mb, (float) value);
            connectionList.push_back(conn);
            //std::cout << "Reading Connection " << ma << "<->" << mb << std::endl;
        }

        istream.close();
    }


    float Parser::getDieWidth() {
        return DieWidth;
    }

    float Parser::getDieHeight() {
        return DieHeight;
    }

    int Parser::getSoftModuleNum() {
        return softModuleNum;
    }

    int Parser::getFixedModuleNum() {
        return fixedModuleNum;
    }

    int Parser::getModuleNum() {
        return moduleNum;
    }

    int Parser::getConnectionNum() {
        return connectionNum;
    }

    GlobalModule Parser::getModule(int index) {
        return modules[index];
    }

    ConnStruct Parser::getConnection(int index) {
        return connectionList[index];
    }

    std::vector<ConnStruct> Parser::getConnectionList() const{
        return this->connectionList;
    }
}


namespace RectGrad {
    Parser::Parser() {
        this->softModuleNum = 0;
        this->fixedModuleNum = 0;
        this->moduleNum = 0;
    }

    Parser::Parser(std::string file_name) {
        this->softModuleNum = 0;
        this->fixedModuleNum = 0;
        this->moduleNum = 0;
        this->read_input(file_name);
    }

    Parser::~Parser() {}

    void Parser::read_input(std::string file_name) {
        std::ifstream istream(file_name);
        if ( istream.fail() ) {
            std::cout << file_name << " doesn't exist.\n";
            return;
        }
        std::string s, ma, mb;
        int area, x, y, w, h, value;

        istream >> s >> this->DieWidth >> this->DieHeight;
        istream >> s >> this->softModuleNum;

        for ( int i = 0; i < this->softModuleNum; i++ ) {
            istream >> s >> area;
            GlobalModule mod(s, this->DieWidth / 2., this->DieHeight / 2., area, false);
            modules.push_back(mod);
            //std::cout << "Reading Soft Module " << s << "..." << std::endl;
        }

        istream >> s >> this->fixedModuleNum;

        for ( int i = 0; i < fixedModuleNum; i++ ) {
            istream >> s >> x >> y >> w >> h;
            GlobalModule mod(s, x, y, w, h, w * h, true);
            modules.push_back(mod);
            //std::cout << "Reading Fixed Module " << s << "..." << std::endl;
        }

        this->moduleNum = this->softModuleNum + this->fixedModuleNum;

        istream >> s >> this->connectionNum;

        for ( int i = 0; i < connectionNum; i++ ) {
            istream >> ma >> mb >> value;
            ConnStruct conn(ma, mb, value);
            connectionList.push_back(conn);
            //std::cout << "Reading Connection " << ma << "<->" << mb << std::endl;
        }

        istream.close();
    }


    int Parser::getDieWidth() {
        return this->DieWidth;
    }

    int Parser::getDieHeight() {
        return this->DieHeight;
    }

    int Parser::getSoftModuleNum() {
        return this->softModuleNum;
    }

    int Parser::getFixedModuleNum() {
        return this->fixedModuleNum;
    }

    int Parser::getModuleNum() {
        return this->moduleNum;
    }

    int Parser::getConnectionNum() {
        return this->connectionNum;
    }

    GlobalModule Parser::getModule(int index) {
        return modules[index];
    }

    ConnStruct Parser::getConnection(int index) {
        return connectionList[index];
    }

    std::vector<ConnStruct> Parser::getConnectionList() const{
        return this->connectionList;
    }
} // namespace RectGrad

