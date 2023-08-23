#ifndef __TESSERA_H__
#define __TESSERA_H__

#include <vector>
#include <string>

#include "LFUnits.h"
#include "Tile.h"
#include <vector>

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
    area_t calRealArea();
public:
    std::vector <Tile *> TileArr;
    std::vector <Tile *> OverlapArr;

    Tessera();
    Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);

    std::string getName () const;
    area_t getLegalArea () const;
    area_t calAreaMargin ();
    Cord getBBLowerLeft ();
    Cord getBBUpperRight();
    len_t getBBWidth ();
    len_t getBBHeight ();

    bool checkLegalNoHole();
    bool checkLegalNoEnclave();
    bool checkLegalEnoughArea();
    bool checkLegalAspectRatio();
    bool checkLegalStuffedRatio();

    int checkLegal();

    int insertTiles(Tile *tile);
    void splitRectliearDueToOverlap();

    void printCorners(std::ostream& fout);

    bool operator ==(const Tessera &tess) const;
    
};


#endif // __TESSERA_H__