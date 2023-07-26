#ifndef __TILE_H__
#define __TILE_H__

#include <vector>
#include "LFUnits.h"
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
    bool printLabel;

    Tile();
    Tile(tileType t, Cord LL, len_t w, len_t h);
    
    tileType getType() const;

    Cord getLowerLeft() const;
    Cord getUpperLeft() const;
    Cord getLowerRight() const;
    Cord getUpperRight() const;
    void setLowerLeft(Cord lowerLeft);
    
    len_t getWidth() const;
    len_t getHeight() const;

    void setCord(Cord cord);
    void setWidth(len_t width);
    void setHeight(len_t height);
    
    float getAspectRatio() const;
    area_t getArea() const;

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

    void show() const;
    
};

#endif // __TILE_H__