#ifndef __PALETTEKNIFE_H__
#define __PALETTEKNIFE_H__

#include "LFLegaliser.h"
#include "Tile.h"
#include <vector>

class paletteKnife{
private:
    LFLegaliser *mLegaliser;
    std::vector <Tile *> overlap2;
    std::vector <Tile *> overlap3;
    std::vector <Tile *> overlap4;


public:
    paletteKnife() = delete;
    paletteKnife(LFLegaliser &legaliser);
    
    bool collectOverlaps();



};

#endif // __PALETTEKNIFE_H__