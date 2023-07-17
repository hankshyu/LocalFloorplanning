#ifndef __TILE_H__
#define __TILE_H__

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

    Tile *up, *down, *left, *right;
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

    void setWidth(len_t width);
    void setHeight(len_t height);
    
    float getAspectRatio() const;
    area_t getArea() const;
    
};


#endif // __TILE_H__