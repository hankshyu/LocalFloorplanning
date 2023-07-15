#ifndef __TILE_H__
#define __TILE_H__

#include "LFUnits.h"

class Tile{
private:
    class Cord mLowerLeft;
    
    len_t mWidth;
    len_t mHeight;

    

public:
    Tile();
    Tile(Cord lowerLeft, len_t width, len_t height);
    
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