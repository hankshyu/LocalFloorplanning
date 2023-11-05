#ifndef __TILE_H__
#define __TILE_H__

#include <vector>
#include "boost/polygon/polygon.hpp"
#include "LFUnits.h"

enum class tileType{
    BLOCK, BLANK, OVERLAP
};

class Tile{
private:
    tileType type;
    // class Cord mLowerLeft;
    
    // len_t mWidth;
    // len_t mHeight;
    Rectangle mRectangle;
    int mTessIndex;

public:

    Tile *rt, *tr, *bl, *lb;
    
    Tile();
    Tile(tileType t, Cord LL, len_t w, len_t h, int tessIndex);
    // indicates the tesseraIndex to which tile belongs, leave index empty for blank tiles
    Tile(tileType t, Rectangle& r, int tessIndex);
    Tile(const Tile &other);

    Tile& operator = (const Tile &other);
    friend std::ostream &operator << (std::ostream &os, const Tile &t);
    
    tileType getType() const;
    int getTessIndex() const;

    Rectangle getRectangle() const;

    Cord getLowerLeft() const;
    Cord getUpperLeft() const;
    Cord getLowerRight() const;
    Cord getUpperRight() const;
    void setLowerLeft(Cord lowerLeft);
    
    len_t getWidth() const;
    len_t getHeight() const;

    void setType(tileType type);
    void setTessOwnership(int tessIndex);
    void setCord(Cord cord);
    void setWidth(len_t width);
    void setHeight(len_t height);
    
    float getAspectRatio() const;
    area_t getArea() const;
    
    bool operator == (const Tile &comp) const;

    inline bool checkXCordInTile(const Cord &point) const;
    inline bool checkYCordInTile(const Cord &point) const;
    inline bool checkCordInTile(const Cord &point) const;

    // if input Tile's lower-left touches the right edge of current tile (used in Directed Area Enumeration)
    bool checkTRLLTouch(Tile *right) const;
    bool cutHeight (len_t cut) const;

    void show(std::ostream &os) const;
    void show(std::ostream &os, bool printNewLine) const;
    void showLink(std::ostream &os) const;

    
};

std::ostream &operator << (std::ostream &os, const Tile &t);

std::ostream &operator << (std::ostream &o, const Point &pt);
std::ostream &operator << (std::ostream &o, const Polygon90WithHoles &poly);
std::ostream &operator << (std::ostream &o, const Polygon90Set &polys);

// * These are new added functions for tile manipulation
std::vector<Tile> cutTile(Tile bigTile, Tile smallTile);
std::vector<Tile> mergeTile(Tile tile1, Tile tile2);
std::vector<Tile> mergeCutTiles(std::vector<Tile> toMerge, std::vector<Tile> toCut, int outputTessIndex);

#endif // __TILE_H__