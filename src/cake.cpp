#include <assert.h>
#include "cake.h"

cake::cake(LFLegaliser *legaliser, Tile *overlap, int overlapLV){
    this->mLegaliser = legaliser;
    this->mOverlap = overlap;
    this->mOverlapLevel = overlapLV;

    assert(overlapLV >= 2);
    assert(overlapLV <= 4);
    
    std::vector <Tile *> allSurroudnings;
    mLegaliser->findAllNeighbors(this->mOverlap, allSurroudnings);
    for(Tile * t : allSurroudnings){
        if(t->getType() == tileType::BLOCK || t->getType() == tileType::OVERLAP){
            // such tessera shall be added into mMothers
            this->mMothers.push_back(mLegaliser->searchTileInTessera(t));
        }
    }
    assert(this->mMothers.size() == this->mOverlapLevel);

}

cake::~cake(){
    for(int i = 0; i < 4; ++i){
        for(crust *c : this->surroundings[i]){
            delete(c);
        }
    }
}

void cake::collectCrusts(){
    
    // there could be optimisation here: by preknowing the linkings, we may prioritize some blank tiles
    
    for(int i = 0; i < mMothers.size(); ++i){
        Tessera *tess = mMothers[i];
        std::vector <Cord> record;
        for(Tile *tileInTess : tess->TileArr){
            std::vector <Tile *> topNeighbors;
            mLegaliser->findTopNeighbors(tileInTess, topNeighbors);
            for(Tile *t : topNeighbors){
                if(t->getType() == tileType::BLANK){
                    
                }
            }
        }
    }

}

// void cake::findBlanksAroundTessera(Tessera *tessera, std::vector <Tile *> neighbors){
//     for(Tile *t : tessera->TileArr){
//         std::vector <Tile *> topNeighbors;
//         mLegaliser->findTopNeighbors(t, topNeighbors);
//         for(Tile *potNeighbor : topNeighbors){
//             if(potNeighbor->getType() == tileType::BLANK){
//                 if()
//             }
//         }


//     }
// }