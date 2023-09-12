#ifndef __TILE_H__
#define __TILE_H__

#include <vector>
#include "boost/polygon/polygon.hpp"
#include "boost/polygon/rectangle_data.hpp"
#include "boost/geometry.hpp"
#include "boost/geometry/geometries/box.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "LFUnits.h"

namespace gtl = boost::polygon;

typedef gtl::polygon_data<len_t>                 Polygon;
typedef gtl::rectangle_data<len_t>               Rectangle;
// typedef gtl::polygon_traits<Polygon>::point_type Point;
typedef gtl::point_data<len_t> Point;
typedef std::vector<Polygon>                     PolygonSet;


enum class tileType{
    BLOCK, BLANK, OVERLAP
};

class Tile{
private:
    tileType type;
    class Cord mLowerLeft;
    
    len_t mWidth;
    len_t mHeight;

public:
    std::vector <int> OverlapFixedTesseraeIdx;
    std::vector <int> OverlapSoftTesseraeIdx;

    Tile *rt, *tr, *bl, *lb;
    
    Tile();
    Tile(tileType t, Cord LL, len_t w, len_t h);
    Tile(const Tile &other);

    Tile& operator = (const Tile &other);
    friend std::ostream &operator << (std::ostream &os, const Tile &t);
    
    tileType getType() const;

    Cord getLowerLeft() const;
    Cord getUpperLeft() const;
    Cord getLowerRight() const;
    Cord getUpperRight() const;
    void setLowerLeft(Cord lowerLeft);
    
    len_t getWidth() const;
    len_t getHeight() const;

    void setType(tileType type);
    void setCord(Cord cord);
    void setWidth(len_t width);
    void setHeight(len_t height);
    
    float getAspectRatio() const;
    area_t getArea() const;
    
    bool operator == (const Tile &comp) const;

    inline bool checkXCordInTile(const Cord &point) const{
        return (point.x >= this->mLowerLeft.x) && (point.x < this->mLowerLeft.x + this->mWidth);
    }
    inline bool checkYCordInTile(const Cord &point) const{
        return (point.y >= this->mLowerLeft.y) && (point.y < this->mLowerLeft.y + this->mHeight);
    }
    inline bool checkCordInTile(const Cord &point) const{
        return (checkXCordInTile(point) && checkYCordInTile(point));
    }

    // if input Tile's lower-left touches the right edge of current tile (used in Directed Area Enumeration)
    bool checkTRLLTouch(Tile *right) const;
    bool cutHeight (len_t cut) const;

    void show(std::ostream &os) const;
    void show(std::ostream &os, bool printNewLine) const;
    void showLink(std::ostream &os) const;

    
};

std::ostream &operator << (std::ostream &os, const Tile &t);

std::ostream &operator << (std::ostream &o, const Point &pt);
std::ostream &operator << (std::ostream &o, const Polygon &poly);
std::ostream &operator << (std::ostream &o, const PolygonSet &polys);

// * These are new added functions for tile manipulation
std::vector<Tile> cutTile(Tile bigTile, Tile smallTile);
std::vector<Tile> mergeTile(Tile tile1, Tile tile2);
std::vector<Tile> mergeCutTiles(std::vector<Tile> toMerge, std::vector<Tile> toCut);

#endif // __TILE_H__