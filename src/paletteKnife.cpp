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

paletteKnife::~paletteKnife(){
    for(cake *ck : pastriesLevel2){
        delete(ck);
    }
    for(cake *ck : pastriesLevel3){
        delete(ck);
    }
    for(cake *ck : pastriesLevel4){
        delete(ck);
    }
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
    collectOverlaps();
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
                        int errorCode = -1;

                        if(!afterTess.isLegal(errorCode)){
                            std::cout << afterTess << std::endl;
                            std::cout <<"Potential Removal: violate rule #" << errorCode;
                            std::cout << "Residual = " << residual;
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
    //burntCakes records the LL Cord of unsolvable cakes at eatCakesLevel2
    std::vector <Cord> burntCake;
    for(int i = 0; i < mPaintClusters[2].size(); ++i){
        cake *ck = new cake(this->mLegaliser, this->mTessFavorDirection, this->mPaintClusters[2][i], 2);
        this->pastriesLevel2.push_back(ck);
    }

    bool solutionMade;
    while(burntCake.size() != mPaintClusters[2].size()){
        solutionMade = false;

        for(cake* cak : pastriesLevel2){
            cak->collectCrusts(mLegaliser);
        }
        std::sort(pastriesLevel2.begin(), pastriesLevel2.end(), compareCakes);
   
        int plateIdx = 0;
        cake *onPlate = pastriesLevel2[plateIdx];
        while(checkVectorInclude(burntCake, onPlate->getOverlapTile()->getLowerLeft())){
            plateIdx++;
            onPlate = pastriesLevel2[plateIdx];
        }
        // cake onPlate is the target for us to solve this round, either eat it, or put into burntCake
        std::cout << "This is onPlate for this Round..." << std::endl;
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

        // display the soft tesseras that would interact in the below distribution process
        std::cout << "showTessIndexes: " << std::endl;
        for(int i = 0; i < availTessIndexes.size(); ++i){
            std::cout << "Index = " << availTessIndexes[i] << ", Area = " << onPlate->mMothers[availTessIndexes[i]]->getLegalArea() << std::endl;
        }

        // start the real distribution process, we would apply water-Jar strategy
        for(int rounds = 0; rounds < availTessIndexes.size(); ++rounds){
            //each round would unlock another group of tesseras(tessera has many crusts inside), according to rounds e.g) avaliTessIndexes = [3, 7, 8], round0 = [3], round1 = [3, 7]
            std::vector <crust *> priorityCrust;
            for(int i = rounds; i >=0; --i){
                for(crust *cand : onPlate->surroundings[i]){

                    //first check if there is already a duplicate insice priorityCrust
                    bool priorityCrustExistDuplicate = false;
                    for(crust *cr : priorityCrust){
                        if(cr->tile->getLowerLeft() == cand->tile->getLowerLeft()) priorityCrustExistDuplicate = true;
                    }
                    if(priorityCrustExistDuplicate) continue;

                    
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
            // from now on the priority Crust includes all crusts available in this distribution round,
            // Mission is each round:
            // Try to use the crusts inside priority Crust to form a distribution to solve the overlap "onPlate"
            
            std::cout << std::endl << "Printing Priority crust:" << std::endl;
            for(int i = 0; i < priorityCrust.size(); ++i){
                crust *c = priorityCrust[i];
                std::cout << "(cr)";
                c->tile->show(std::cout, false);
                std::cout << ", Direction: " << c->direction << ", crowdIdx: " << c->crowdIdx << ", rating = " << c->ratingIdx;
                std::cout << ", assignedTess: " << c->assignedTessera->getName() << std::endl;
            }

            // Start by using Water-Jar strategy, iterage the priorityCrust vector from begin() to end()
            area_t leftToDistributeArea = onPlate->getOverlapTile()->getArea();
            // This is the phony Tesseras to check legality, recycle after use
            std::vector <Tessera *> jarcheckLegal;
            // this is phony tiles created to check legality, recycle after use
            std::vector <Tile *> jarcheckLegalTiles;

            for(int crustIdx = 0; crustIdx < priorityCrust.size(); ++crustIdx){
                if(priorityCrust[crustIdx]->tile->getArea() < leftToDistributeArea){
                    // We need the entire tile, no change of ending the round
                    
                    // this is the phony tessera we would push phony tiles into
                    Tessera *verifyTess;
                    int verifyTessIdx = findVectorIncludebyName(jarcheckLegal, priorityCrust[crustIdx]->assignedTessera);
                    if(verifyTessIdx != -1){
                        // already exist the phony tessera
                        verifyTess = jarcheckLegal[verifyTessIdx];
                    }else{
                        // create a new phony tessera
                        Tessera *newPhonyTess = new Tessera(*(priorityCrust[crustIdx]->assignedTessera));
                        //remove the overlap tile
                        int rmIdx;
                        for(int i = 0; i < newPhonyTess->OverlapArr.size(); ++i){
                            if(newPhonyTess->OverlapArr[i]->getLowerLeft() == onPlate->getOverlapTile()->getLowerLeft()){
                                rmIdx = i;
                                break;
                            }
                        }
                        newPhonyTess->OverlapArr.erase(newPhonyTess->OverlapArr.begin() + rmIdx);
                        jarcheckLegal.push_back(newPhonyTess);
                        verifyTess = newPhonyTess;
                    }
                    Tile *distributedTile = new Tile(tileType::BLOCK, priorityCrust[crustIdx]->tile->getLowerLeft(), 
                                                priorityCrust[crustIdx]->tile->getWidth(), priorityCrust[crustIdx]->tile->getHeight());
                    jarcheckLegalTiles.push_back(distributedTile);
                    verifyTess->TileArr.push_back(distributedTile);
                    leftToDistributeArea -= priorityCrust[crustIdx]->tile->getArea();

                    std::cout << "Put all into Tess" << verifyTess->getName() << " :";
                    distributedTile->show(std::cout, false);
                    std::cout << ", leftToDistributeArea = " << leftToDistributeArea << std::endl;
                }else{

                    // this is the phony tessera we would push phony tiles into
                    Tessera *verifyTess;
                    int verifyTessIdx = findVectorIncludebyName(jarcheckLegal, priorityCrust[crustIdx]->assignedTessera);
                    if(verifyTessIdx != -1){
                        // already exist the phony tessera
                        verifyTess = jarcheckLegal[verifyTessIdx];
                    }else{
                        // create a new phony tessera
                        Tessera *newPhonyTess = new Tessera(*(priorityCrust[crustIdx]->assignedTessera));
                        //remove the overlap tile
                        int rmIdx;
                        for(int i = 0; i < newPhonyTess->OverlapArr.size(); ++i){
                            if(newPhonyTess->OverlapArr[i]->getLowerLeft() == onPlate->getOverlapTile()->getLowerLeft()){
                                rmIdx = i;
                                break;
                            }
                        }
                        newPhonyTess->OverlapArr.erase(newPhonyTess->OverlapArr.begin() + rmIdx);
                        jarcheckLegal.push_back(newPhonyTess);
                        verifyTess = newPhonyTess;
                    }

                    // This one single Tile could do the job, entering thsi section indicates that the round ends
                    
                    // check the direction of the cake
                    Tile connectedTile;
                    len_t leftDownFitBorder, rightTopFitBorder;

                    int location = locateTileTesseraDirection(priorityCrust[crustIdx]->assignedTessera ,priorityCrust[crustIdx]->tile,
                                                                connectedTile, leftDownFitBorder, rightTopFitBorder);
                    assert((location >= 1) && (location <= 4));

                    len_t fittestBase = (rightTopFitBorder - leftDownFitBorder);
                    assert(fittestBase > 0);

                    // This is the last phony tile we would insert into the phony tessera
                    Tile *distributedTile;

                    std::cout << "locateTileTesseraDirection fnc(" << priorityCrust[crustIdx]->assignedTessera->getName() << ", ";
                    priorityCrust[crustIdx]->tile->show(std::cout, false);
                    std::cout << std::endl;
                    std::cout << "location = " << location;
                    std::cout<< ", (ld, rt) = (" << leftDownFitBorder << ", " << rightTopFitBorder << ")" << std::endl;
                    std::cout << "connectedTile:";

                    connectedTile.show(std::cout);
                    

                    switch (location){
                        case 1:{ // tessera is on top of the tile
                            bool useFittest = ((fittestBase * priorityCrust[crustIdx]->tile->getHeight()) >= leftToDistributeArea);
                            if(useFittest){
                                len_t newWidth = fittestBase;
                                len_t newHeight = leftToDistributeArea/fittestBase;
                                newHeight = ((newHeight * fittestBase) >= leftToDistributeArea)? newHeight : newHeight + 1;
                                Cord fitCord = Cord(leftDownFitBorder, connectedTile.getLowerLeft().y);
                                distributedTile = new Tile(tileType::BLOCK, fitCord, newWidth, newHeight);
                            }else{
                                // fittest plan would not work, use the entire base to distriute
                                len_t newWidth = priorityCrust[crustIdx]->tile->getWidth();
                                len_t newHeight = leftToDistributeArea / newWidth;
                                newWidth = ((newWidth * newHeight) >= leftToDistributeArea)? newWidth : newWidth + 1;
                                distributedTile = new Tile(tileType::BLOCK, connectedTile.getLowerLeft(), newWidth, newHeight);
                            }
                            break;
                        }
                        case 2:{ // tessera is under the tile
                            bool useFittest = ((fittestBase * priorityCrust[crustIdx]->tile->getHeight()) >= leftToDistributeArea);
                            if(useFittest){
                                len_t newWidth = fittestBase;
                                len_t newHeight = leftToDistributeArea/fittestBase;
                                newHeight = ((newHeight * fittestBase) >= leftToDistributeArea)? newHeight : newHeight + 1;
                                Cord fitCord = Cord(leftDownFitBorder, connectedTile.getUpperLeft().y - newHeight);
                                distributedTile = new Tile(tileType::BLOCK, fitCord, newWidth, newHeight);
                            }else{
                                len_t newWidth = priorityCrust[crustIdx]->tile->getWidth();
                                len_t newHeight = leftToDistributeArea / newWidth;
                                newWidth = ((newWidth * newHeight) >= leftToDistributeArea)? newWidth : newWidth + 1;
                                Cord normCord = Cord(connectedTile.getLowerLeft().x, connectedTile.getUpperRight().y - newHeight);
                                distributedTile = new Tile(tileType::BLOCK, normCord, newWidth, newHeight);
                            }
                            break;
                        }
                        case 3:{ // tessera is at the left of the tile
                            bool useFittest = ((fittestBase * priorityCrust[crustIdx]->tile->getWidth()) >= leftToDistributeArea);
                            if(useFittest){
                                len_t newHeight = fittestBase;
                                len_t newWidth = leftToDistributeArea / fittestBase;
                                newWidth = ((newHeight * newWidth) >= leftToDistributeArea) ? newWidth : newWidth + 1;
                                Cord fitCord = Cord(connectedTile.getLowerRight().x, leftDownFitBorder);
                                distributedTile = new Tile(tileType::BLOCK, fitCord, newWidth, newHeight);
                            }else{
                                len_t newHeight = priorityCrust[crustIdx]->tile->getHeight();
                                len_t newWidth = leftToDistributeArea / newHeight;
                                newWidth = ((newHeight * newWidth) >= leftToDistributeArea) ? newWidth : newWidth + 1;
                                Cord normCord = priorityCrust[crustIdx]->tile->getLowerLeft();
                                distributedTile = new Tile(tileType::BLOCK, normCord, newWidth, newHeight);
                            }

                            break;
                        }
                        case 4:{ // tessera is at the right of the tile
                            bool useFittest = ((fittestBase * priorityCrust[crustIdx]->tile->getWidth()) >= leftToDistributeArea);
                            if(useFittest){
                                len_t newHeight = fittestBase;
                                len_t newWidth = leftToDistributeArea / fittestBase;
                                newWidth = ((newHeight * newWidth) >= leftToDistributeArea) ? newWidth : newWidth + 1;
                                Cord fitCord = Cord(connectedTile.getLowerLeft().x - newWidth, leftDownFitBorder);
                                distributedTile = new Tile(tileType::BLOCK, fitCord, newWidth, newHeight);
                            }else{
                                len_t newHeight = priorityCrust[crustIdx]->tile->getHeight();
                                len_t newWidth = leftToDistributeArea / newHeight;
                                newWidth = ((newHeight * newWidth) >= leftToDistributeArea) ? newWidth : newWidth + 1;
                                Cord normCord = Cord(connectedTile.getLowerLeft().x - newWidth, priorityCrust[crustIdx]->tile->getLowerLeft().y);
                                distributedTile = new Tile(tileType::BLOCK, normCord, newWidth, newHeight);
                            }
                            break;
                        }
                    }
                    std::cout << "Show distributedTile: "<<std::endl;
                    distributedTile->show(std::cout);
                    jarcheckLegalTiles.push_back(distributedTile);
                    verifyTess->TileArr.push_back(distributedTile);
                    leftToDistributeArea = 0; // done! just clean to 0

                    //  now the phony tessera has been filled, verify!
                    break;

                }
            }

            // now we have selected the tiles to the tessera, start verification
            bool verifyplan = true;
            for(Tessera *phonyTess : jarcheckLegal){
                // TODO, start verification
                int errorCode;
                if(!phonyTess->isLegal(errorCode)){
                    verifyplan = false;
                    std::cout << "isLegal() return false, " << phonyTess->getName() << " returns" << errorCode << std::endl;
                }
            }
            if(verifyplan){
                if(leftToDistributeArea != 0) verifyplan = false;
            }

            if(verifyplan){
                // This plan works!
                std::cout << "Such plan would work!" << std::endl;
                onPlate->showCake();
                solutionMade = true;
                break;
            }else{
                std::cout << "No... not legal" << std::endl;
            }



            for(int i = 0; i < jarcheckLegal.size(); ++i){
                delete(jarcheckLegal[i]);
            }
            for(Tile *t : jarcheckLegalTiles){
                delete(t);
            }

        }
        if(solutionMade){
            std::cout << "found some solution!!!" << std::endl;
            break;
        }

        // Planning session ends here!, onPlate exists no known solution!
        std::cout << "No known solution to cake is found: ";
        onPlate->showCake();
        burntCake.push_back(onPlate->getOverlapTile()->getLowerLeft());


        // this is for debug, remove before use
        // std::cout << "Test Exit normally" << std::endl;
        // break;
    }
    
}

int paletteKnife::locateTileTesseraDirection(Tessera *tess, Tile *target, Tile &connectedTile, len_t &leftDownFitBorder, len_t &rightTopFitBorder){
    // if tessera is on top of the tile
    std::vector<Tile *> topNeighbors;
    mLegaliser->findTopNeighbors(target, topNeighbors);
    for(Tile *t : topNeighbors){
        if(t->getType() != tileType::BLANK){

            int inOverlap = findVectorInclude(tess->OverlapArr, t);
            int inTile = findVectorInclude(tess->TileArr, t);
            if((inOverlap != -1) || (inTile != -1)){
                
                if(inOverlap != -1){
                    connectedTile = *(tess->OverlapArr[inOverlap]);
                }else{ // intile != -1
                    connectedTile = *( tess->TileArr[inTile]);
                }

                if(target->getLowerLeft().x > connectedTile.getLowerLeft().x){
                    leftDownFitBorder = target->getLowerLeft().x;
                }else{
                    leftDownFitBorder = connectedTile.getLowerLeft().x;
                }
                rightTopFitBorder = std::min(target->getLowerRight().x, connectedTile.getLowerRight().x);
                return 1;
            }
        }
    }

    // if tessera is below the tile
    std::vector<Tile *> bottomNeighbors;
    mLegaliser->findDownNeighbors(target, bottomNeighbors);
    for(Tile *t : bottomNeighbors){
        if(t->getType() != tileType::BLANK){

            int inOverlap = findVectorInclude(tess->OverlapArr, t);
            int inTile = findVectorInclude(tess->TileArr, t);
            if((inOverlap != -1) || (inTile != -1)){
                
                if(inOverlap != -1){
                    connectedTile = *(tess->OverlapArr[inOverlap]);
                }else{ // intile != -1
                    connectedTile = *(tess->TileArr[inTile]);
                }

                if(target->getLowerLeft().x > connectedTile.getLowerLeft().x){
                    leftDownFitBorder = target->getLowerLeft().x;
                }else{
                    leftDownFitBorder = connectedTile.getLowerLeft().x;
                }
                
                rightTopFitBorder = std::min(target->getLowerRight().x, connectedTile.getLowerRight().x);
                return 2;
            }
        }
    }

    // if tessera is at the left of the tile
    std::vector<Tile *> leftNeighbors;
    mLegaliser->findLeftNeighbors(target, leftNeighbors);
    for(Tile *t : leftNeighbors){
        if(t->getType() != tileType::BLANK){

            int inOverlap = findVectorInclude(tess->OverlapArr, t);
            int inTile = findVectorInclude(tess->TileArr, t);
            if((inOverlap != -1) || (inTile != -1)){
                
                if(inOverlap != -1){
                    connectedTile = *(tess->OverlapArr[inOverlap]);
                }else{ // intile != -1
                    connectedTile = *(tess->TileArr[inTile]);
                }

                if(target->getLowerRight().y > connectedTile.getLowerRight().y){
                    leftDownFitBorder = target->getLowerRight().y;
                }else{
                    leftDownFitBorder = connectedTile.getLowerRight().y;
                }
                
                rightTopFitBorder = std::min(target->getUpperRight().y, connectedTile.getUpperRight().y);
                return 3;
            }
        }
    }

    // if tesera is at the right of the tile
    std::vector<Tile *> rightNeighbors;
    mLegaliser->findRightNeighbors(target, rightNeighbors);
    for(Tile *t : rightNeighbors){
        if(t->getType() != tileType::BLANK){

            int inOverlap = findVectorInclude(tess->OverlapArr, t);
            int inTile = findVectorInclude(tess->TileArr, t);
            if((inOverlap != -1) || (inTile != -1)){
                
                if(inOverlap != -1){
                    connectedTile = *(tess->OverlapArr[inOverlap]);
                }else{ // intile != -1
                    connectedTile = *(tess->TileArr[inTile]);
                }

                std::cout << "in locateTileTesseraDirection:";
                connectedTile.show(std::cout);

                if(target->getLowerRight().y > connectedTile.getLowerRight().y){
                    leftDownFitBorder = target->getLowerRight().y;
                }else{
                    leftDownFitBorder = connectedTile.getLowerRight().y;
                }
                
                rightTopFitBorder = std::min(target->getUpperRight().y, connectedTile.getUpperRight().y);
                return 4;
            }
        }
    }


    // no found, return error codes
    leftDownFitBorder = -1;
    rightTopFitBorder = -1;
    return -1;
}

bool compareCakes(cake *c1, cake *c2){
    return c1->getDifficultyIdx() < c2->getDifficultyIdx();
}

bool compareCrusts(crust *c1, crust *c2){
    return c1->ratingIdx < c2->ratingIdx;
}