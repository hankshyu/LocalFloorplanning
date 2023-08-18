#include <assert.h>
#include "cake.h"

cake::cake(LFLegaliser *legaliser, Tile *overlap, std::vector<Tessera *> moms){
    this->mLegaliser = legaliser;
    this->mOverlap = overlap;

    assert(moms.size() >= 2);
    assert(moms.size() <= 4);
    for(Tessera* tess : moms){
        this->mMothers.push_back(tess);
    }

}

void cake::collectCrusts(){
    for(int mothersIdx = 0; mothersIdx <= mMothers.size(); ++mothersIdx){
        Tessera *motherTess = mMothers[mothersIdx];
        for(Tile *t : motherTess->TileArr){

            std::vector<Tile *> topNeighbors;
            this->mLegaliser->findTopNeighbors(t, topNeighbors);
            for(Tile *nt : topNeighbors){
                if(nt->getType() == tileType::BLANK){
                    surroundings[mothersIdx].push_back();
                }
            }







        }
    }
}