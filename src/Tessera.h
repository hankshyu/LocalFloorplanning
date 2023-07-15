#ifndef __TESSERA_H__
#define __TESSERA_H__

#include <vector>
#include <string>

#include "LFUnits.h"
#include "Tile.h"

enum class tesseraType{
    EMPTY, SOFT, HARD
};

class Tessera{
private:
    
    tesseraType mType;
    std::string mName;
    area_t mLegalArea;
    
    Cord mInitLowerLeft;
    len_t mInitWidth;
    len_t mInitHeight;

    len_t mWidth;
    len_t mHeight;
    
    std::vector <Tile *> mTileArr;
    std::vector <Tile *> mOverlapArr;
    
    Cord mBBLowerLeft;
    len_t mBBWidth;
    len_t mBBHeight;

    void calBoundingBox();

public:
    Tessera();
    Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);

    int getTileCount ();
    void getTileArr (std::vector <Tile *> *TileArr);

    int getOverlapCount ();
    void getOverlapArr (std::vector <Tile *> *OverlapArr);


    
    
    Cord getBBLowerLeft ();
    len_t getBBWidth ();
    len_t getBBHeight ();


};



#endif // __TESSERA_H__