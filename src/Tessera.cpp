#include "Tessera.h"

Tessera::Tessera()
    : mType(tesseraType::EMPTY) {}

Tessera::Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height)
    : mType(type), mName(name), mLegalArea(area), 
    mInitLowerLeft(lowerleft), mInitWidth(width), mInitHeight(height) {}


Cord Tessera::getBBLowerLeft (){
    return this->mBBLowerLeft;
}
Cord Tessera::getBBUpperRight(){
    return this->mBBUpperRight;
}
len_t Tessera::getBBWidth (){
    return this->mBBUpperRight.x - this->mBBLowerLeft.x;
}
len_t Tessera::getBBHeight (){
    return this->mBBUpperRight.y - this->mBBLowerLeft.y;
}

int Tessera::insertTiles(tileType type, Tile *tile){
    assert(type != tileType::BLANK);
    switch (type){
        case tileType::BLOCK:
            /* code */
            mTileArr.push_back(tile);
            break;
        case tileType::OVERLAP:
            /* code */
            mOverlapArr.push_back(tile);
            break;
        default:
            break;
    }
    calBoundingBox();

    return 1;
}

void Tessera::calBoundingBox(){
    if(mTileArr.empty() && mOverlapArr.empty()) return;

    //The lowerleft and upper right tiles
    Cord LL, UR;

    if(!mTileArr.empty()){
        LL = mTileArr[0]->getLowerLeft();
        UR = mTileArr[0]->getUpperRight();
    }else{
        LL = mOverlapArr[0]->getLowerLeft();
        UR = mOverlapArr[0]->getUpperRight();
    }

    for(Tile *t : mTileArr){
        if(t->getLowerLeft() < LL) LL = t->getLowerLeft();
        if(t->getUpperRight() > UR) UR = t->getUpperRight();
    }
    for(Tile *t : mOverlapArr){
        if(t->getLowerLeft() < LL) LL = t->getLowerLeft();
        if(t->getUpperRight() > UR) UR = t->getUpperRight();  
    }

    this->
}
