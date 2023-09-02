#include "rgmodule.h"
#include <cmath>
#include <iostream>

RGModule::RGModule(std::string in_name, double centerX, double centerY, int in_area, bool in_fixed) {
    name = in_name;
    this->centerX = centerX;
    this->centerY = centerY;
    area = in_area;
    fixed = in_fixed;
    width = std::ceil(std::sqrt((double) area));
    height = std::ceil(std::sqrt((double) area));
}

RGModule::RGModule(std::string in_name, int x, int y, int width, int height, int in_area, bool in_fixed) {
    name = in_name;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    area = in_area;
    fixed = in_fixed;
    this->centerX = x + width / 2.;
    this->centerY = y + height / 2.;
}

RGModule::~RGModule() {
    for ( int i = 0; i < connections.size(); i++ ) {
        delete connections[i];
    }
}

void RGModule::addConnection(RGModule *in_module, double in_value) {
    RGConnection *nc = new RGConnection;
    nc->module = in_module;
    nc->value = in_value;
    connections.push_back(nc);
}

void RGModule::updateCord(int DieWidth, int DieHeight, double sizeScalar) {
    if ( this->fixed ) {
        return;
    }

    this->x = (int) ( centerX - width * sizeScalar / 2. );
    this->y = (int) ( centerY - height * sizeScalar / 2. );
    if ( this->x < 0 ) {
        this->x = 0;
    }
    else if ( this->x > DieWidth - this->width ) {
        this->x = DieWidth - this->width;
    }
    if ( this->y < 0 ) {
        this->y = 0;
    }
    else if ( this->y > DieHeight - this->height ) {
        this->y = DieHeight - this->height;
    }

    this->centerX = this->x + this->width / 2.;
    this->centerY = this->y + this->height / 2.;

    // std::cout << this->name << " " << this->width << " , " << this->height << std::endl;
    // std::cout << this->centerX << " , " << this->centerY << std::endl;
}