#include <assert.h>
#include <algorithm>
#include <string>
#include "paletteKnife.h"
#include "Tessera.h"
#include "tensor.h"

paletteKnife::paletteKnife(LFLegaliser *legaliser, std::vector <RGConnStruct> *connectionList){
    this->mLegaliser = legaliser;
    calAllTessFavorDirection(connectionList);

}

void paletteKnife::calAllTessFavorDirection(std::vector <RGConnStruct> *connectionList){
    for(Tessera *tess : mLegaliser->fixedTesserae){
        //this did not process when direction is absent!
        double favor;
        calTessFavorDirection(tess, connectionList, favor);
        mTessFavorDirection[tess->getName()] = favor;

    }
    for(Tessera *tess : mLegaliser->softTesserae){
        //this did not process when direction is absent!
        double favor;
        calTessFavorDirection(tess, connectionList, favor);
        mTessFavorDirection[tess->getName()] = favor;

    }
}

bool paletteKnife::calTessFavorDirection(Tessera *tessera, std::vector <RGConnStruct> *connectionList, double &direction){
    bool metConnections = false;
    double cumulateWeights = 0;
    double favorDirection;

    for(RGConnStruct cs : (*connectionList)){
        double fromX, fromY;
        double toX, toY;

        tessera->calBBCentre(fromX, fromY);
        
        Tessera *toT = nullptr;
        bool foundToT = false;

        if(cs.m0 == tessera->getName()){
            for(Tessera *t : mLegaliser->fixedTesserae){
                if(t->getName() == cs.m1){
                    toT = t;
                    foundToT = true;
                    break;
                }
            }
            if(!foundToT){
                for(Tessera *t : mLegaliser->softTesserae){
                    if(t->getName() == cs.m1){
                        toT = t;
                        foundToT = true;
                        break;
                    }
                }
            }
            assert(foundToT);

        }else if(cs.m1 == tessera->getName()){
            for(Tessera *t : mLegaliser->fixedTesserae){
                if(t->getName() == cs.m0){
                    toT = t;
                    foundToT = true;
                    break;
                }
            }
            if(!foundToT){
                for(Tessera *t : mLegaliser->softTesserae){
                    if(t->getName() == cs.m0){
                        toT = t;
                        foundToT = true;
                        break;
                    }
                }
            }
            assert(foundToT);
        }
        if(foundToT){
            if(!metConnections){
                //first connection
                metConnections = true;
                toT->calBBCentre(toX, toY);
                double angle = tns::calVectorAngle(fromX, fromY, toX, toY);
                favorDirection = angle;
                cumulateWeights = (double)cs.value;
            }else{
                toT->calBBCentre(toX, toY);
                double angle = tns::calVectorAngle(fromX, fromY, toX, toY);
                double tmpcumulateWeights = (double)cs.value + cumulateWeights;
                assert(tmpcumulateWeights >= 0);
                favorDirection = (angle * (cs.value/tmpcumulateWeights)) + (favorDirection * (cumulateWeights/tmpcumulateWeights));
                cumulateWeights = tmpcumulateWeights;
            }
        }

    }

    direction = (metConnections)? favorDirection : 127;

    return metConnections;
}

paletteKnife::~paletteKnife(){

    // free mPaintClusters memories
    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < mPaintClusters[i].size(); ++j){
            delete(mPaintClusters[i][j]);
        }
    }

    //free all pastry level memories
    for(cake *c : pastriesLevel2){
        delete(c);
    }
    for(cake *c : pastriesLevel3){
        delete(c);
    }
    for(cake *c : pastriesLevel4){
        delete(c);
    }


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
                
                if((overlapNum < 2) || (overlapNum > 4)){
                    std::cout << "ERROR Caugt!! - <2 or >4";
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
                assert(area >= 0);
                std::cout << idx << "(" << area << ") ";
            }
            std::cout << std::endl;

        }
        std::cout << std::endl << std::endl;
    }
}

void paletteKnife::disperseViaMargin(){
    for(int overlapNum = 4; overlapNum >= 2; overlapNum--){
        
        for(int tessIdx = 0; tessIdx < this->mLegaliser->softTesserae.size(); ++tessIdx){
            Tessera *tess = this->mLegaliser->softTesserae[tessIdx];
            area_t residual = tess->calAreaMargin();
            if(residual == 0) continue;
            
            for(int tileIdx = 0; tileIdx < tess->OverlapArr.size(); ++tileIdx){
                Tile *tile = tess->OverlapArr[tileIdx];
                if((tile->OverlapSoftTesseraeIdx.size() + tile->OverlapFixedTesseraeIdx.size()) == overlapNum){
                    if(residual >= tile->getArea()){
                        
                        //check if removing the tile is harmful to the tessera
                        Tessera afterTess(tesseraType::EMPTY, "TRY", tess->getLegalArea() ,tess->getBBLowerLeft(), 0, 0);
                        afterTess.TileArr.clear();
                        for(Tile *t : tess->TileArr){
                            afterTess.TileArr.push_back(t);
                        }
                        for(Tile *t : tess->OverlapArr){
                            if(t->getLowerLeft() != tile->getLowerLeft()){
                                afterTess.OverlapArr.push_back(t);
                            }
                        }
                        if(afterTess.checkLegal() != 0){
                            std::cout <<"Potential Removal: violate rule #" << afterTess.checkLegal();
                            tile->show(std::cout);
                            continue;
                        }

                        // Residual larger than overlap, we could discard the overlap directly.
                        // std::cout << "[Remove Overlap" << ""]";
                        if(overlapNum == 2){
                            std::cout << "[Remove Overlap]";
                        }else{
                            std::cout <<"[Overlap LV #" << overlapNum << " -> #" <<overlapNum-1 << "]";
                        }
                        tile->show(std::cout, true);
                        residual -= tile->getArea();
                        for(int rmIdx = 0; rmIdx < tile->OverlapSoftTesseraeIdx.size(); ++rmIdx){
                            if(tile->OverlapSoftTesseraeIdx[rmIdx] == tessIdx){
                                tile->OverlapSoftTesseraeIdx.erase(tile->OverlapSoftTesseraeIdx.begin() + rmIdx);
                                break;
                            }
                        }
                        tess->OverlapArr.erase(tess->OverlapArr.begin() + tileIdx);
                        tileIdx--;
                        if(overlapNum == 2){
                            tile->setType(tileType::BLOCK);
                            
                            bool hasChanged = false;
                            
                            if(!tile->OverlapSoftTesseraeIdx.empty()){
                                // The last tile belongs to a soft tess
                                int loneIdx = tile->OverlapSoftTesseraeIdx[0];
                                mLegaliser->softTesserae[loneIdx]->TileArr.push_back(tile);
                                for (int k = 0; k < mLegaliser->softTesserae[loneIdx]->OverlapArr.size(); ++k){
                                    if(mLegaliser->softTesserae[loneIdx]->OverlapArr[k]->getLowerLeft() == tile->getLowerLeft()){
                                        mLegaliser->softTesserae[loneIdx]->OverlapArr.erase(mLegaliser->softTesserae[loneIdx]->OverlapArr.begin() + k);
                                        hasChanged = true;
                                        break;
                                    }
                                }
                            }else{
                                // The last tile belongs to a hard tess
                                int loneIdx = tile->OverlapFixedTesseraeIdx[0];
                                mLegaliser->fixedTesserae[loneIdx]->TileArr.push_back(tile);
                                for (int k = 0; k < mLegaliser->fixedTesserae[loneIdx]->OverlapArr.size(); ++k){
                                    if(mLegaliser->fixedTesserae[loneIdx]->OverlapArr[k]->getLowerLeft() == tile->getLowerLeft()){
                                        mLegaliser->fixedTesserae[loneIdx]->OverlapArr.erase(mLegaliser->fixedTesserae[loneIdx]->OverlapArr.begin() + k);
                                        hasChanged = true;
                                        break;
                                    }
                                }
                            }
                            assert(hasChanged);

                        }
                        
                    }
                }
            }
            // std::cout << "DT" << overlapNum << " " << tess->getName() << " " << tess->calAreaMargin() << std::endl;
        }
    }

}

void paletteKnife::eatCakesLevel2(){
    std::vector <Cord> burntCake;
    while(burntCake.size() != mPaintClusters[2].size()){
        bool swallowCake = false;
        for(int i = 0; i < mPaintClusters[2].size(); ++i){
            cake *ck = new cake(this->mLegaliser, this->mTessFavorDirection, this->mPaintClusters[2][i], 2);
            this->pastriesLevel2.push_back(ck);
        }
        for(cake* cak : pastriesLevel2){
            cak->collectCrusts(mLegaliser);
        }
        std::sort(pastriesLevel2.begin(), pastriesLevel2.end(), compareCakes);
        // for visualisation
        // for(cake *cak : pastriesLevel2){
        //     cak->showCake();
        //     std::cout << "--------------------------------" << std::endl << std::endl;
        // }
        
        int plateIdx = 0;
        cake *onPlate = pastriesLevel2[plateIdx];
        while(checkVectorInclude(burntCake, onPlate->getOverlapTile()->getLowerLeft())){
            plateIdx++;
            onPlate = pastriesLevel2[plateIdx];
        }
        // cake onPlate is the target for us to solve this round, either eat it, or put into burntCake
        onPlate->showCake();
        if(onPlate->getDifficultyIdx() < 1.0){
            // this is unsolvable, put into burntCake directly and start next cycle
            burntCake.push_back(onPlate->getOverlapTile()->getLowerLeft());
            std::cout << "Put into burntCake..." << std::endl;
            continue;
        }

        // Find the soft tesseras that would interact in the below distribution process
        std::vector <int> availTessIndexes;
        for(int i = 0; i < onPlate->mMothers.size(); ++i){
            if(onPlate->mMothers[i]->getType() == tesseraType::SOFT){
                availTessIndexes.push_back(i);
            }
        }
        // apply insertion sort to availtessIndexes
        if(availTessIndexes.size() > 1){
            int key, j;
            for(int i = 1; i < availTessIndexes.size(); ++i){
                key = availTessIndexes[i];
                j = i - 1;
                
                while((j >= 0) && (onPlate->mMothers[availTessIndexes[i]]->getLegalArea() < onPlate->mMothers[availTessIndexes[j]]->getLegalArea())){
                    availTessIndexes[j+1] = availTessIndexes[j];
                    j--;
                }
                availTessIndexes[j+1] = key;
            }
        }

        std::cout << "showTessIndexes: " << std::endl;
        for(int i = 0; i < availTessIndexes.size(); ++i){
            std::cout << "Index = " << availTessIndexes[i] << ", Area = " << onPlate->mMothers[availTessIndexes[i]]->getLegalArea() << std::endl;
        }

        for(int rounds = 0; rounds < availTessIndexes.size(); ++rounds){
            //each round would unlock
            std::vector <crust *> priorityCrust;
            for(int i = rounds; i >=0; --i){
                for(crust *cand : onPlate->surroundings[i]){

                    double realAngle = tns::calIncludeAngle(onPlate->mMothersFavorDirection[i], cand->direction);

                    // calculate CrowdIdx for each crust
                    double NeighborsOverlapArea = 0;

                    std::vector <Tile *> neighbors;
                    mLegaliser->findAllNeighbors(cand->tile, neighbors);
                    
                    std::vector <Tessera *> surroundingTess;
                    for(Tile *nb : neighbors){
                        if(nb->getType() != tileType::BLANK){
                            std::vector<Tessera *> inTess;
                            mLegaliser->searchTesseraeIncludeTile(nb, inTess);
                            for(Tessera* t : inTess){
                                if(!checkVectorInclude(surroundingTess, t)){
                                    surroundingTess.push_back(t);
                                }
                            }
                        }
                    }

                    for(Tessera *nbTess : surroundingTess){
                        for(Tile *t : nbTess->OverlapArr){
                            if((*t) == (*(cand->tile))){
                                continue;
                            }
                            NeighborsOverlapArea += (double)(t->getArea());
                        }
                    }
                    cand->crowdIdx = NeighborsOverlapArea / (double) (cand->tile->getArea());

                    cand->ratingIdx = (DIRECTION_COEFF * realAngle) + (CROWD_COEFF * cand->crowdIdx);

                    cand->assignedTessera = onPlate->mMothers[availTessIndexes[i]];
                    priorityCrust.push_back(cand);
                }
            }

            std::sort(priorityCrust.begin(), priorityCrust.end(), compareCrusts);
            // from now on the priority Crust includes all crusts available in this
            
            std::cout << std::endl << "Printing Priority crust:" << std::endl;
            for(int i = 0; i < priorityCrust.size(); ++i){
                crust *c = priorityCrust[i];
                std::cout << "(cr)";
                c->tile->show(std::cout, false);
                std::cout << ", Direction: " << c->direction << ", crowdIdx: " << c->crowdIdx << ", rating = " << c->ratingIdx;
                std::cout << ", assignedTess: " << c->assignedTessera->getName() << std::endl;
            }
        }

    }
    


}

bool compareCakes(cake *c1, cake *c2){
    return c1->getDifficultyIdx() < c2->getDifficultyIdx();
}

bool compareCrusts(crust *c1, crust *c2){
    return c1->ratingIdx < c2->ratingIdx;
}