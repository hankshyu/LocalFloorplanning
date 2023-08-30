#ifndef __CAKE_H__
#define __CAKE_H__

#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"
#include <vector>
#include <map>

const double DIRECTION_COEFF = 2/M_PI;
const double CROWD_COEFF = 1.0/3.0;

struct crust{
public:
    Tile *tile;
    double direction;
    double crowdIdx;
    double ratingIdx;
    Tessera *assignedTessera;
    
    crust() = delete;
    crust(LFLegaliser *legaliser, Tile *t, double tessCentreX, double tessCentreY);
private:
    double calDirection(double tessCentreX, double tessCentreY);
    // double calRawCrowdIdx(LFLegaliser *legaliser);
};

class cake{
private:
    LFLegaliser *mLegaliser;
    Tile *mOverlap;

    int mOverlapLevel;

    // spare Area/ overlap
    double mDifficultyIdx;

public:
    std::vector <Tessera *> mMothers;
    std::vector <double> mMothersFavorDirection;
    //Index is the same with mMothers
    std::vector <crust *> surroundings[4];

    cake() = delete;
    cake(LFLegaliser *legaliser, std::map <std::string, double> mTessFavorDirection, Tile *overlap, int overlapLV);
    ~cake();

    double getDifficultyIdx() const;
    Tile *getOverlapTile() const;

    void showCake();

    void collectCrusts(LFLegaliser *legaliser);
};



#endif // __CAKE_H__