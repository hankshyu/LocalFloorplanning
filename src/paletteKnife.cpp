#include "paletteKnife.h"
#include <assert.h>
#include <algorithm>

paletteKnife::paletteKnife(LFLegaliser *legaliser){
    this->mLegaliser = legaliser;

}

int paletteKnife::collectOverlaps(){
    std::vector <Cord> record;
    for(int i = 0; i < 5; ++i){
        mPaintClusters[i].clear();
    }

    for(Tessera *tess : this->mLegaliser->fixedTesserae){
        for(Tile *ovt : tess->OverlapArr){
            if(!checkVectorInclude(record, ovt->getLowerLeft())){
                record.push_back(ovt->getLowerLeft());
                int overlapNum = ovt->OverlapFixedTesseraeIdx.size() + ovt->OverlapSoftTesseraeIdx.size();
                assert(overlapNum <= 4);
                assert(overlapNum >= 2);

                mPaintClusters[overlapNum].push_back(ovt);

            }
        }
    }
    for(Tessera *tess : this->mLegaliser->softTesserae){
        for(Tile *ovt : tess->OverlapArr){
            if(!checkVectorInclude(record, ovt->getLowerLeft())){
                record.push_back(ovt->getLowerLeft());
                int overlapNum = ovt->OverlapFixedTesseraeIdx.size() + ovt->OverlapSoftTesseraeIdx.size();
                
                if(overlapNum < 2 || overlapNum >4){
                    for(Tile *pt : tess->OverlapArr){
                        pt->show(std::cout);
                    }
                    
                }
                assert(overlapNum <= 4);
                assert(overlapNum >= 2);

                mPaintClusters[overlapNum].push_back(ovt);

            }
        }
    }
    return record.size();
}

void paletteKnife::printpaintClusters(){
    for(int ov = 4; ov >=2 ; --ov){

        std::cout << ov << " Overlaps (" << mPaintClusters[ov].size() << ")" << std::endl;
        for(Tile *t : mPaintClusters[ov]){
            t->show(std::cout);
            std::cout << "FixedTess: ";
            for(int idx : t->OverlapFixedTesseraeIdx){
                std::cout << idx << " ";
            }
            std::cout << "SoftTess: ";
            for(int idx : t->OverlapSoftTesseraeIdx){
                
                area_t area = 0;
                for (Tile *t : mLegaliser->softTesserae[idx]->TileArr){
                    area += t->getArea();
                }
                for (Tile *t : mLegaliser->softTesserae[idx]->OverlapArr){
                    area += t->getArea();
                }
                area -=  mLegaliser->softTesserae[idx]->getLegalArea();
                std::cout << idx << "(" << area << ") ";
            }
            std::cout << std::endl;

        }
        std::cout << std::endl << std::endl;
    }
}

void paletteKnife::disperseViaMargin(){
    std::cout << "Enter disperseViaMargin " << this->mLegaliser->softTesserae.size() << std::endl;
    for(int tessIdx = 0; tessIdx < this->mLegaliser->softTesserae.size(); ++tessIdx){
        Tessera *tess = this->mLegaliser->softTesserae[tessIdx];
        area_t residual = 0;
        for(Tile *tile : tess->TileArr){
            residual += tile->getArea();
        }
        for(Tile *tile : tess->OverlapArr){
            residual += tile->getArea();
        }
        residual = residual - tess->getLegalArea();

        for(int overlapNum = 4; overlapNum >= 2; overlapNum--){
            if(residual == 0){
                break;
            }
            for(int tileIdx = 0; tileIdx < tess->OverlapArr.size(); ++tileIdx){
                Tile *tile = tess->OverlapArr[tileIdx];
                if((tile->OverlapSoftTesseraeIdx.size() + tile->OverlapFixedTesseraeIdx.size()) == overlapNum){
                    if(residual >= tile->getArea()){

                        // Residual larger than overlap, we could discard the overlap directly.
                        residual -= tile->getArea();
                        for(int rmIdx = 0; rmIdx < tile->OverlapSoftTesseraeIdx.size(); ++rmIdx){
                            if(tile->OverlapSoftTesseraeIdx[rmIdx] == tessIdx){
                                tile->OverlapSoftTesseraeIdx.erase(tile->OverlapSoftTesseraeIdx.begin() + rmIdx);
                                break;
                            }
                        }
                        tess->OverlapArr.erase(tess->OverlapArr.begin() + tileIdx);
                        if(overlapNum == 2){
                            tile->setType(tileType::BLOCK);
                            int loneIdx = tile->OverlapSoftTesseraeIdx[0];
                            for (int k = 0; k < mLegaliser->softTesserae[loneIdx]->OverlapArr.size(); ++k){
                                if(mLegaliser->softTesserae[loneIdx]->OverlapArr[k]->getLowerLeft() == tile->getLowerLeft()){
                                    mLegaliser->softTesserae[loneIdx]->OverlapArr.erase(mLegaliser->softTesserae[loneIdx]->OverlapArr.begin() + k);
                                    break;
                                }

                            }
                        }
                        
                    }
                }
            }
        }
    }

}
