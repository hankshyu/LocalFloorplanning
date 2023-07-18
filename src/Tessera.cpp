#include <assert.h>
#include "Tessera.h"

Tessera::Tessera()
    : mType(tesseraType::EMPTY) {}

Tessera::Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height)
    : mType(type), mName(name), mLegalArea(area), 
    mInitLowerLeft(lowerleft), mInitWidth(width), mInitHeight(height) {}

std::string Tessera::getName () const{
    return this->mName;
}

area_t Tessera::getLegalArea () const{
    return this->mLegalArea;
}

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
            TileArr.push_back(tile);
            break;
        case tileType::OVERLAP:
            /* code */
            OverlapArr.push_back(tile);
            break;
        default:
            break;
    }
    calBoundingBox();

    return 1;
}

void Tessera::calBoundingBox(){
    if(TileArr.empty() && OverlapArr.empty()) return;

    //The lowerleft and upper right tiles
    Cord LL, UR;

    if(!TileArr.empty()){
        LL = TileArr[0]->getLowerLeft();
        UR = TileArr[0]->getUpperRight();
    }else{
        LL = OverlapArr[0]->getLowerLeft();
        UR = OverlapArr[0]->getUpperRight();
    }

    for(Tile *t : TileArr){
        if(t->getLowerLeft() < LL) LL = t->getLowerLeft();
        if(t->getUpperRight() > UR) UR = t->getUpperRight();
    }
    for(Tile *t : OverlapArr){
        if(t->getLowerLeft() < LL) LL = t->getLowerLeft();
        if(t->getUpperRight() > UR) UR = t->getUpperRight();  
    }

    this->mBBLowerLeft = LL;
    this->mBBUpperRight = UR;
}

/*
    // TODO
    Notes for Ryan Lin...
    We would call translateGlobalFloorplanning() from LFLegaliser.h to translate cirlces from 
    global floorplanning to Rectangles (Tessera.h), and it would include a default Tile (Tile.h)
    We would then call detectfloorplanningOverlaps() to detect overlaps. Overlaps would be record as
    another Tile(label with tileType::OVERLAP)
    Then call splitFloorplanningOverlaps() from LFLegaliser.h
    it would :
    for(Tessera t : all Tesserae vector){
        t.splitRectliearDueToOverlap()
    }


*/
void Tessera::splitRectliearDueToOverlap(){
    
    /*
    The translated info is stored in:
    - Cord mInitLowerLeft;
    - len_t mInitWidth;
    - len_t mInitHeight;

    There should be "ONE" default Tile in TileArr, which is the default tile(= InitXXX...)
    Overlap tiles are calculated as pushed into overlapArr
    You should read the entire overlap Arr and mark off the area indicated, this would lead you to 
    a rectlinear shape.
    Then you should further split the acquired rectilinear, and split them into smaller tiles. you "MUST"
    remove the default tile if new smaller splits are pushed in. GOOD LUCK!!


    */
}
