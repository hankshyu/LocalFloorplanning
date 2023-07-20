#include "Tile.h"

Tile::Tile()
    : type(tileType::BLANK), mLowerLeft(Cord(0,0)), mWidth(0), mHeight(0), printLabel(false),
        rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {}

Tile::Tile(tileType t, Cord LL, len_t w, len_t h) 
    : type(t), mLowerLeft(LL), mWidth(w), mHeight(h), printLabel(false),
        rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {}

tileType Tile::getType() const {
    return this->type;
}
Cord Tile::getLowerLeft() const {
    return this->mLowerLeft;
};
Cord Tile::getUpperLeft() const {
    return this->mLowerLeft + Cord(0, this->mHeight);
};
Cord Tile::getLowerRight() const {
    return this->mLowerLeft + Cord(this->mWidth, 0);
};
Cord Tile::getUpperRight() const {
    return this->mLowerLeft + Cord(this->mWidth, this->mHeight);
};
void Tile::setLowerLeft(Cord lowerLeft){
    this->mLowerLeft = lowerLeft;
};


len_t Tile::getWidth() const {
    return this->mWidth;
};

len_t Tile::getHeight() const {
    return this->mHeight;
};

void Tile::setWidth(len_t width){
    this->mWidth = width;
};
void Tile::setHeight(len_t height){
    this->mHeight = height;
};

float Tile::getAspectRatio() const {
    return this->mWidth / this->mHeight;
};

area_t Tile::getArea() const {
    return this->mWidth * this->mHeight;
};



