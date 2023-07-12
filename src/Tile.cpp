#include "Tile.h"

Tile::Tile()
    : lowerLeft(Cord(0,0)), width(0), height(0) {}

Tile::Tile(Cord LL, len_t w, len_t h) 
    : lowerLeft(LL), width(w), height(h) {}

Cord Tile::getLowerLeft() const {
    return this->lowerLeft;
};
Cord Tile::getUpperLeft() const {
    return this->lowerLeft + Cord(0, this->height);
};
Cord Tile::getLowerRight() const {
    return this->lowerLeft + Cord(this->width, 0);
};
Cord Tile::getUpperRight() const {
    return this->lowerLeft + Cord(this->width, this->height);
};
void Tile::setLowerLeft(Cord new_LL){
    this->lowerLeft = new_LL;
};


len_t Tile::getWidth() const {
    return this->width;
};

len_t Tile::getHeight() const {
    return this->height;
};

void Tile::setWidth(len_t new_width){
    this->width = new_width;
};
void Tile::setHeight(len_t new_height){
    this->height = new_height;
};

float Tile::getAspectRatio() const {
    return this->width / this->height;
};

area_t Tile::getArea() const {
    return this->width * this->height;
};

