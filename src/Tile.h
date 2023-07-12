#ifndef __TILE_H__
#define __TILE_H__

#include "LFUnits.h"

class Tile{
private:
    class Cord lowerLeft;
    
    len_t width;
    len_t height;

public:
    Tile();
    Tile(Cord LL, len_t w, len_t h);
    
    Cord getLowerLeft() const;
    Cord getUpperLeft() const;
    Cord getLowerRight() const;
    Cord getUpperRight() const;
    void setLowerLeft(Cord new_LL);
    
    len_t getWidth() const;
    len_t getHeight() const;

    void setWidth(len_t new_width);
    void setHeight(len_t new_height);
    
    float getAspectRatio() const;
    area_t getArea() const;
    
};


#endif // __TILE_H__