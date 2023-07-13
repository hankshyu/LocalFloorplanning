#ifndef __LFLEGALISER_H__
#define __LFLEGALISER_H__


#include <vector>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"


class LFLegaliser{
private:
    len_t mCanvasWidth;
    len_t mCanvasHeight;
    
    std::vector <Tessera*> fixedTesserae;
    std::vector <Tessera*> softTesserae;


public:
    LFLegaliser() = delete;
    LFLegaliser(len_t chipWidth, len_t chipHeight);

    int addTile();
    Tile *findPoint(const Cord &key) const;


    

};


#endif // __LFLEGALISER_H__