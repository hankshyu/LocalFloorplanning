#ifndef __PALETTEKNIFE_H__
#define __PALETTEKNIFE_H__

#include "LFLegaliser.h"
#include "Tile.h"
#include <vector>

class paletteKnife{
private:
    LFLegaliser *mLegaliser;

    std::vector <Tile *> mPaintClusters[5];


public:
    paletteKnife() = delete;
    paletteKnife(LFLegaliser *legaliser);
    
    int collectOverlaps();
    void printpaintClusters();

    void disperseViaMargin();



};

#endif // __PALETTEKNIFE_H__