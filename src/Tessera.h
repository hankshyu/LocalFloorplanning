#ifndef __TESSERA_H__
#define __TESSERA_H__

#include <vector>
#include <string>
#include <iostream>

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
    area_t calRealArea();
public:
    std::vector <Tile *> TileArr;
    std::vector <Tile *> OverlapArr;

    Tessera();
    Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);
    Tessera(const Tessera &other);

    Tessera& operator = (const Tessera &other);
    bool operator == (const Tessera &tess) const;
    friend std::ostream &operator << (std::ostream &os, const Tessera &t);

    std::string getName () const;
    area_t getLegalArea () const;
    tesseraType getType() const;
    Cord getInitLowerLeft () const;
    len_t  getInitWidth () const;
    len_t getInitHeight () const;
    area_t calAreaMargin ();
    Cord getBBLowerLeft ();
    Cord getBBUpperRight();
    len_t getBBWidth ();
    len_t getBBHeight ();
    void calBBCentre(double &CentreX, double &CentreY);

    bool checkLegalNoHole();
    bool checkLegalNoEnclave();
    bool checkLegalEnoughArea();
    bool checkLegalAspectRatio();
    bool checkLegalStuffedRatio();

    int checkLegal();

    int insertTiles(Tile *tile);
    void splitRectliearDueToOverlap();

    void printCorners(std::ostream &fout);



    // ErrorCode encoding:
    // 1: check whether this Tessera is connected or not
    // 2: check whether this Tessera has holes or not
    // 3: check whether this Tessera violates area constraint or not
    // 4: check whether this Tessera violates aspect ratio or not
    // 5: check whether this Tessera violates rectangle ratio or not(0.8)
    bool isLegal(int &errorCode);
    bool isLegal();

};

std::ostream &operator << (std::ostream &os, const Tessera &t);

#endif // __TESSERA_H__