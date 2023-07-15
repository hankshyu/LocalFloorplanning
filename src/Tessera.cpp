#include "Tessera.h"

Tessera::Tessera()
    : mType(tesseraType::EMPTY) {}

Tessera::Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height)
    : mType(type), mName(name), mLegalArea(area), 
    mInitLowerLeft(lowerleft), mInitWidth(width), mInitHeight(height) {}

int Tessera::getTileCount(){
    return this->mTileArr.size();
}
void Tessera::getTileArr (std::vector <Tile *> *TileArr){
    TileArr = &mTileArr;
}
int Tessera::getOverlapCount(){
    return this->mOverlapArr.size();
}
void Tessera::getOverlapArr (std::vector <Tile *> *OverlapArr){
    OverlapArr = &mOverlapArr;
}

Cord Tessera::getBBLowerLeft (){
    return this->mBBLowerLeft;
}
len_t Tessera::getBBWidth (){
    return this->mBBWidth;
}
len_t Tessera::getBBHeight (){
    return this->mBBHeight;
}

