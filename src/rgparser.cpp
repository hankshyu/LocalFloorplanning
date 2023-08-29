#include "rgparser.h"

RGParser::RGParser() {
    this->softModuleNum = 0;
    this->fixedModuleNum = 0;
    this->moduleNum = 0;
}

RGParser::RGParser(std::string file_name) {
    this->softModuleNum = 0;
    this->fixedModuleNum = 0;
    this->moduleNum = 0;
    this->read_input(file_name);
}

RGParser::~RGParser() {}

void RGParser::read_input(std::string file_name) {
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
        RGModule mod(s, this->DieWidth / 2., this->DieHeight / 2., area, false);
        modules.push_back(mod);
        //std::cout << "Reading Soft Module " << s << "..." << std::endl;
    }

    istream >> s >> this->fixedModuleNum;

    for ( int i = 0; i < fixedModuleNum; i++ ) {
        istream >> s >> x >> y >> w >> h;
        RGModule mod(s, x, y, w, h, w * h, true);
        modules.push_back(mod);
        //std::cout << "Reading Fixed Module " << s << "..." << std::endl;
    }

    this->moduleNum = this->softModuleNum + this->fixedModuleNum;

    istream >> s >> this->connectionNum;

    for ( int i = 0; i < connectionNum; i++ ) {
        istream >> ma >> mb >> value;
        RGConnStruct conn(ma, mb, (double) value);
        connectionList.push_back(conn);
        //std::cout << "Reading Connection " << ma << "<->" << mb << std::endl;
    }

    istream.close();
}


int RGParser::getDieWidth() {
    return this->DieWidth;
}

int RGParser::getDieHeight() {
    return this->DieHeight;
}

int RGParser::getSoftModuleNum() {
    return this->softModuleNum;
}

int RGParser::getFixedModuleNum() {
    return this->fixedModuleNum;
}

int RGParser::getModuleNum() {
    return this->moduleNum;
}

int RGParser::getConnectionNum() {
    return this->connectionNum;
}

RGModule RGParser::getModule(int index) {
    return modules[index];
}

RGConnStruct RGParser::getConnection(int index) {
    return connectionList[index];
}

std::vector<RGConnStruct> RGParser::getConnectionList() const{
    return this->connectionList;
}