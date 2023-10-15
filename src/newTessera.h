#ifndef __TESSERA_H__
#define __TESSERA_H__

#include <vector>
#include <string>
#include <iostream>
#include <boost/polygon/polygon.hpp>
#include "FPManager.h"
#include "LFUnits.h"
// #include "Tile.h"

namespace bg = boost::polygon;
using namespace boost::polygon::operators;

typedef bg::rectangle_data<len_t> Rectangle;
typedef bg::polygon_90_set_data<len_t> Polygon90Set;
typedef bg::polygon_90_with_holes_data<len_t> Polygon90WithHoles;
typedef bg::point_data<len_t> Point;

enum class tesseraType{
    EMPTY ,SOFT, HARD
};

class Tessera{
private:
    Polygon90Set mShape; 
    
    tesseraType mType;
    std::string mName;
    area_t mLegalArea;
    
    Cord mInitLowerLeft;
    len_t mInitWidth;
    len_t mInitHeight;
    
    Cord mBBLowerLeft;
    Cord mBBUpperRight;
    

    FPManager& mFPM;
    void calBoundingBox();
    area_t calRealArea();
    void _addArea(Cord lowerleft, len_t width, len_t height);
    bool _checkHole();
public:
    std::vector <int> OverlapArr;

    Tessera() = delete; 
    Tessera(FPManager& FP, tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);
    Tessera(const Tessera &other);

    Tessera& operator = (const Tessera &other);
    bool operator == (const Tessera &tess) const;
    friend std::ostream &operator << (std::ostream &os, const Tessera &t);

    // TODO: dirty bit?
    void getFullShape(Polygon90Set& poly);
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

std::ostream &operator << (std::ostream &o, const Point &pt);
std::ostream &operator << (std::ostream &o, const Polygon &poly);
std::ostream &operator << (std::ostream &o, const PolygonSet &polys);

class Overlap{
private:
    Polygon90WithHoles mShape; 
public:
    std::vector<int> overlaps;
    Overlap() = delete;
    Overlap(Polygon90WithHoles& shape, std::vector<int> overlapIndices);
    Polygon90WithHoles& getShape();
};

#endif // __TESSERA_H__