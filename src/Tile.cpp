#include <iostream>
#include "Tile.h"

Tile::Tile()
    : type(tileType::BLANK), mLowerLeft(Cord(0,0)), mWidth(0), mHeight(0), printLabel(false), mprintReset(false),
        rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {}

Tile::Tile(tileType t, Cord LL, len_t w, len_t h) 
    : type(t), mLowerLeft(LL), mWidth(w), mHeight(h), printLabel(false), mprintReset(false),
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

void Tile::setCord (Cord cord){
    this->mLowerLeft = cord;
}

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

bool Tile::checkTRLLTouch(Tile *right) const{
    Cord rightLL = right->getLowerLeft();
    
    bool xAligned = ((getUpperRight().x) == (rightLL.x));
    bool yInRange = ((rightLL.y >= getLowerRight().y) && (rightLL.y < getUpperRight().y));

    return (xAligned && yInRange);
}

bool Tile::cutHeight(len_t cut) const{
    return ((cut > mLowerLeft.y) && (cut < (mLowerLeft.y + mHeight)));
}

void Tile::show(std::ostream &os) const {
    if(this->type == tileType::BLOCK) os <<"Type: BLOCK ";
    else if(this->type == tileType::BLANK) os <<"Type: BLANK ";
    else if(this->type == tileType::OVERLAP) os <<"Type: OVERLAP ";
    
    os << "(" <<mLowerLeft.x << ", " << mLowerLeft.y << ") ";
    os << "W=" << mWidth <<" H=" << mHeight << std::endl;
}

void Tile::showLink(std::ostream &os) const{
    os << "rt: ";
    if(this->rt == nullptr) os << "nullptr" << std::endl;
    else this->rt->show(os);

    os << "tr: ";
    if(this->tr == nullptr) os << "nullptr" << std::endl;
    else this->tr->show(os);

    os << "bl: ";
    if(this->bl == nullptr) os << "nullptr" << std::endl;
    else this->bl->show(os);

    os << "lb: ";
    if(this->lb == nullptr) os << "nullptr" << std::endl;
    else this->lb->show(os);
    os << std::endl;
}

