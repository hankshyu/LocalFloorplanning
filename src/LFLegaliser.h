#ifndef __LFLEGALISER_H__
#define __LFLEGALISER_H__


#include <vector>
#include <assert.h>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"


class LFLegaliser{
private:
    len_t mCanvasWidth;
    len_t mCanvasHeight;
    
    std::vector <Tessera*> fixedTesserae;
    std::vector <Tessera*> softTesserae;

    bool checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height);


public:
    LFLegaliser() = delete;
    LFLegaliser(len_t chipWidth, len_t chipHeight);

    int addFirstTessera(tesseraType type, Cord lowerLeft, len_t width, len_t height);
    Tile *findPoint(const Cord &key) const;


    

};


#endif // __LFLEGALISER_H__