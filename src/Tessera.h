#ifndef __TESSERA_H__
#define __TESSERA_H__

#include <vector>

#include "LFUnits.h"
#include "Tile.h"

enum class tesseraType{
    EMPTY, SOFT, HARD
};

class Tessera{
private:
    int mTileCount;
    std::vector <Tile *> mTileArr;
    
    Cord mBBLowerLeft;
    len_t mBBWidth;
    len_t mBBHeight;

    void calBoundingBox();


public:
    tesseraType type;

    int getTileCount ();
    void getTileArr (std::vector <Tile *> &TileArr);
    
    Cord getBBLowerLeft ();
    len_t getBBWidth ();
    len_t getBBHeight ();





};

class hardTessera : public Tessera{
public:

};

class softTessera : public Tessera{

};







#endif // __TESSERA_H__