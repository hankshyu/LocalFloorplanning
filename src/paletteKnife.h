#ifndef __PALETTEKNIFE_H__
#define __PALETTEKNIFE_H__

#include "LFLegaliser.h"
#include "Tile.h"
#include "cake.h"
#include <vector>

class paletteKnife{
private:
    LFLegaliser *mLegaliser;

    std::vector <Tile *> mPaintClusters[5];


public:
    paletteKnife() = delete;
    paletteKnife(LFLegaliser *legaliser);
    ~paletteKnife();
    
    std::vector <cake *> pastriesLevel2;
    std::vector <cake *> pastriesLevel3;
    std::vector <cake *> pastriesLevel4;


    // clear old info, find all overlaps(2, 3, 4) and place them into mPaintClusters[].
    int collectOverlaps();
    void printpaintClusters();

    // primitive tile reduction via margin of circle to square. shall be called only once.
    void disperseViaMargin();

    void bakeCakesLevel2();
    void eatCakesLevel2();

    

};

#endif // __PALETTEKNIFE_H__