#ifndef __PALETTEKNIFE_H__
#define __PALETTEKNIFE_H__

#include "LFLegaliser.h"
#include "Tile.h"
#include "cake.h"
#include "parser.h"
#include <vector>
#include <map>

class paletteKnife{
private:
    LFLegaliser *mLegaliser;
    
    std::vector <Tile *> mPaintClusters[5];
    // maps the Tessera's favor direction, range from -PI ~ PI, if == 127, no connection is present.
    std::map <std::string, double> mTessFavorDirection;

    
    // this fills mTessFavorDirection with input connectionList
    void calAllTessFavorDirection(std::vector <RGConnStruct> *connectionList);
    bool calTessFavorDirection(Tessera *tessera, std::vector <RGConnStruct> *connectionList, double &direction);

public:
    paletteKnife() = delete;
    paletteKnife(LFLegaliser *legaliser, std::vector <RGConnStruct> *connectionList);
    ~paletteKnife();
    
    std::vector <cake *> pastriesLevel2;
    std::vector <cake *> pastriesLevel3;
    std::vector <cake *> pastriesLevel4;


    // clear old info, find all overlaps(2, 3, 4) and place them into mPaintClusters[].
    int collectOverlaps();
    void printpaintClusters();

    // primitive tile reduction via margin of circle to square. shall be called only once.
    void disperseViaMargin();

    void eatCakesLevel2();

    //retrun -1 if no contact, tessera is (1, 2, 3, 4) = (up, down, left, right) of the tile
    int locateTileTesseraDirection(Tessera *tess, Tile *target, Tile &connectedTile, len_t &leftDownFitBorder, len_t &rightTopFitBorder);

};

#endif // __PALETTEKNIFE_H__

bool compareCakes(cake *c1, cake *c2);
bool compareCrusts(crust *c1, crust *c2);