#include "paletteKnife.h"
#include <assert>

paletteKnife::paletteKnife(LFLegaliser &legaliser){
    this->mLegaliser = &legaliser;

}

bool paletteKnife::collectOverlaps(){
    std::vector <Cord> record;

    for(Tessera *tess : this->mLegaliser.fixedTesserae){
        for(Tile *ovt : tess->OverlapArr){
            if(!checkVectorInclude(record, ovt->getLowerLeft())){
                assert(ovt->type == tileType::OVERLAP);

            }
            

        }
    }
}