#ifndef __TESSERA_H__
#define __TESSERA_H__

#include <vector>
#include <string>

#include "LFUnits.h"
#include "Tile.h"

enum class tesseraType{
    EMPTY ,SOFT, HARD
};

class Tessera{
private:
    
    tesseraType mType;
    std::string mName;
    area_t mLegalArea;
    
    Cord mInitLowerLeft;
    len_t mInitWidth;
    len_t mInitHeight;
    
    Cord mBBLowerLeft;
    Cord mBBUpperRight;

    void calBoundingBox();

public:
    std::vector <Tile *> mTileArr;
    std::vector <Tile *> mOverlapArr;

    Tessera();
    Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);

    Cord getBBLowerLeft ();
    Cord getBBUpperRight();
    len_t getBBWidth ();
    len_t getBBHeight ();

    int insertTiles(tileType type, Tile *tile);

};

#endif // __TESSERA_H__