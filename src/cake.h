#ifndef __CAKE_H__
#define __CAKE_H__

#include <vector>
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"

enum class cakeDirection{
    UP, DOWN, LEFT, RIGHT
};

struct crust{
    Tile *tile;
    cakeDirection direction;
    int crowdIdx;
    
    crust() = delete;
    crust(Tile *t, cakeDirection d): tile(t), direction(d) {};
};

class cake{
private:
    LFLegaliser *mLegaliser;
    Tile *mOverlap;
    std::vector <Tessera *> mMothers;

public:
    std::vector <crust> surroundings[4];

    cake() = delete;
    cake(LFLegaliser *legaliser, Tile *overlap, std::vector<Tessera *> moms);

    void collectCrusts();

};

#endif // __CAKE_H__