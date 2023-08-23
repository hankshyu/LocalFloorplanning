#ifndef __CAKE_H__
#define __CAKE_H__

#include <vector>
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"


struct crust{
public:
    Tile *tile;
    double direction;
    double crowdIdx;
    
    crust() = delete;
    crust(LFLegaliser *legaliser, Tile *t, double tessCentreX, double tessCentreY);
private:
    double calDirection(double tessCentreX, double tessCentreY);
    double calCrowdIdx(LFLegaliser *legaliser);
};

class cake{
private:
    LFLegaliser *mLegaliser;
    Tile *mOverlap;
    std::vector <Tessera *> mMothers;
    int mOverlapLevel;

public:

    //Index tis the same with mMothers
    std::vector <crust *> surroundings[4];

    cake() = delete;
    cake(LFLegaliser *legaliser, Tile *overlap, int overlapLV);
    ~cake();

    void showCake();

    void collectCrusts(LFLegaliser *legaliser);
};



#endif // __CAKE_H__