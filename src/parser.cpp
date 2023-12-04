#include "parser.h"

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
    std::istringstream ss;
    std::string line;
    if ( istream.fail() ) {
        std::cout << file_name << " doesn't exist.\n";
        return;
    }
    std::string s, type;
    int area, w, h, value;
    float x, y;

    std::getline(istream, line);
    ss.str(line);
    ss >> s >> this->moduleNum >> s >> this->connectionNum;

    std::getline(istream, line);
    ss.str(std::string());
    ss.clear();
    ss.str(line);
    ss >> this->DieWidth >> this->DieHeight;

    for ( int i = 0; i < this->moduleNum; i++ ) {
        std::getline(istream, line);
        ss.str(std::string());
        ss.clear();
        ss.str(line);
        ss >> s >> type >> x >> y >> w >> h;
        int x_int = (int) std::round(x);
        int y_int = (int) std::round(y);
        GlobalModule mod(s, x_int, y_int, w, h, w * h, (type == "FIXED"));
        if ( type == "FIXED" ) {
            fixedModuleNum++;
        } else {
            softModuleNum++;
        }
        modules.push_back(mod);
        std::cout << "Reading Module " << s << " " << x << " " << y << " " << w << " " << h << "..." << std::endl;
    }

    for ( int i = 0; i < connectionNum; i++ ) {
        std::string mod;
        std::vector<std::string> modules;
        
        std::getline(istream, line);
        ss.str(std::string());
        ss.clear();
        ss.str(line);

        while ( ss >> mod ) {
            modules.push_back(mod);
        }

        value = std::stoi(modules.back());
        modules.pop_back();

        ConnStruct conn(modules, value);
        connectionList.push_back(conn);
        // std::cout << "Reading Connection " << modules[0] << "<->" << modules[1] << " : " << value << std::endl;
        // std::cout << "Reading Connection ";
        // for ( std::string mod : modules ) {
        //     std::cout << mod << " ";
        // }
        // std::cout << " : " << value << std::endl;
    }

    istream.close();
}


int Parser::getDieWidth() {
    return DieWidth;
}

int Parser::getDieHeight() {
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

std::vector<ConnStruct> Parser::getConnectionList() const {
    return this->connectionList;
}

