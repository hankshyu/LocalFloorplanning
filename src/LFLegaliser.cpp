#include <iostream>
#include "LFLegaliser.h"


LFLegaliser::LFLegaliser(len_t chipWidth, len_t chipHeight)
    : mCanvasWidth(chipWidth), mCanvasHeight(chipHeight) {}

bool LFLegaliser::checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height) const {
    bool x_valid, y_valid;
    x_valid = (lowerLeft.x >= 0) && (lowerLeft.x + width <= this->mCanvasWidth);
    y_valid = (lowerLeft.y >= 0) && (lowerLeft.y + height <= this->mCanvasHeight);
    return (x_valid && y_valid);
}



Tile *LFLegaliser::getRandomTile() const{
    assert(!(fixedTesserae.empty() && softTesserae.empty()));
    
    if(!fixedTesserae.empty()){
        if(!fixedTesserae[0]->TileArr.empty()){
            return fixedTesserae[0]->TileArr[0];        
        }else{
            return fixedTesserae[0]->OverlapArr[0];        
        }
    }else{
        if(!softTesserae[0]->TileArr.empty()){
            return softTesserae[0]->TileArr[0];
        }else{
            return softTesserae[0]->OverlapArr[0];
        }
    }
}


len_t LFLegaliser::getCanvasWidth () const{
    return this->mCanvasWidth;
}

len_t LFLegaliser::getCanvasHeight () const{
    return this->mCanvasHeight;
}

//TODO
void LFLegaliser::translateGlobalFloorplanning(){

}
//TODO
void LFLegaliser::detectfloorplanningOverlaps(){

}
//TODO
void LFLegaliser::splitFloorplanningOverlaps(){

}


int LFLegaliser::addFirstTessera(tesseraType type, std::string name, area_t area, Cord lowerLeft, len_t width, len_t height){
    
    assert(checkTesseraInCanvas(lowerLeft, width, height));
    assert(type != tesseraType::EMPTY);

    Tessera *newTessera = new Tessera(type, name, area, lowerLeft, width, height);
    Tile *newTile = new Tile(tileType::BLOCK, lowerLeft, width, height);
    newTessera->insertTiles(tileType::BLOCK, newTile);
    
    if(type == tesseraType::HARD){
        this->fixedTesserae.push_back(newTessera);
    }else if(type == tesseraType::SOFT){
        this->softTesserae.push_back(newTessera);
    }

    //todo: add links
    if(newTile->getLowerLeft().y != 0){
        Tile *tdown = new Tile(tileType::BLANK, Cord(0,0),
                            this->mCanvasWidth, newTile->getLowerLeft().y);
        newTile->lb = tdown;
        tdown->rt = newTile;
    }

    if(newTile->getUpperRight().y <= this->mCanvasHeight){
        Tile *tup = new Tile(tileType::BLANK, Cord(0,newTile->getUpperRight().y), 
                            this->mCanvasWidth, (this->mCanvasHeight - newTile->getUpperRight().y));
        newTile->rt = tup;
        tup->lb = newTile;
    }

    if(lowerLeft.x != 0){
        Tile *tleft = new Tile(tileType::BLANK, Cord(0, newTile->getLowerLeft().y),
                            newTile->getLowerLeft().x, height);
        newTile->bl = tleft;
        tleft->tr = newTile;    
    }

    if((lowerLeft.x + width)!= mCanvasWidth){
        Tile *tright = new Tile(tileType::BLANK, newTile->getLowerRight(), 
                            (this->mCanvasWidth - newTile->getUpperRight().x), height);
        newTile->tr = tright;
        tright->bl = newTile;
    }

    return 0;
}


Tile *LFLegaliser::findPoint(const Cord &key) const{
    assert(key >= Cord(0,0));
    assert(key.x < getCanvasWidth());
    assert(key.y < getCanvasHeight());

    Tile *index = getRandomTile();
    
    while(!(index->checkCordInTile(key))){
        if(!index->checkYCordInTile(key)){
            // Adjust vertical range
            if(key.y >= index->getLowerLeft().y){
                assert(index->rt != nullptr);
                index = index->rt;
            }else{
                assert(index->lb != nullptr);
                index = index->lb;
            }
        }else{
            // Vertical range correct! adjust horizontal range
            if(key.x >= index->getLowerLeft().x){
                assert(index->tr != nullptr);
                index = index->tr;
            }else{
                assert(index->bl != nullptr);
                index = index->bl;
            }
        }
    }
    
    return index;
}

Tile *LFLegaliser::findPoint(const Cord &key, Tile *initTile) const{
    assert(key >= Cord(0,0));
    assert(key.x < getCanvasWidth());
    assert(key.y < getCanvasHeight());

    Tile *index = initTile;
    
    while(!(index->checkCordInTile(key))){
        if(!index->checkYCordInTile(key)){
            // Adjust vertical range
            if(key.y >= index->getLowerLeft().y){
                assert(index->rt != nullptr);
                index = index->rt;
            }else{
                assert(index->lb != nullptr);
                index = index->lb;
            }
        }else{
            // Vertical range correct! adjust horizontal range
            if(key.x >= index->getLowerLeft().x){
                assert(index->tr != nullptr);
                index = index->tr;
            }else{
                assert(index->bl != nullptr);
                index = index->bl;
            }
        }
    }
    
    return index;
}

void LFLegaliser::findTopNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    Tile *n = centre->rt;
    while(n->getLowerLeft().x > centre->getLowerLeft().x){
        neighbors.push_back(n);
        n = n->bl;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findDownNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    Tile *n = centre->lb;
    while(n->getUpperRight().x < centre->getUpperRight().x){
        neighbors.push_back(n);
        n = n->tr;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findLeftNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    Tile *n = centre->bl;
    while(n->getUpperRight().y < centre->getUpperRight().y){
        neighbors.push_back(n);
        n = n->rt;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findRightNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    Tile *n = centre->tr;
    // the last neighbor is the first tile encountered whose lower y cord <= lower y cord of starting tile
    while(n->getLowerLeft().y > centre->getLowerLeft().y){
        neighbors.push_back(n);
        n = n->lb;
    }
    neighbors.push_back(n);
    
}

void LFLegaliser::findAllNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    findTopNeighbors(centre, neighbors);
    findDownNeighbors(centre, neighbors);
    findLeftNeighbors(centre, neighbors);
    findRightNeighbors(centre, neighbors);
}

bool LFLegaliser::searchArea(Cord lowerleft, len_t width, len_t height, Tile &target) const{

    assert(checkTesseraInCanvas(lowerleft, width, height));

    // Use point-finding algo to locate the tile containin the upperleft corner of AOI
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *currentFind = findPoint(Cord(lowerleft.x, searchRBorderHeight));
    std::cout << "Init found:" <<std::endl;

    while(currentFind->getUpperLeft().y > lowerleft.y){
        // See if the tile is solid
        if(currentFind->getType() != tileType::BLANK){
            // This is an edge of a solid tile
            target = *currentFind;
            return true;
        }else if(currentFind->getUpperRight().x < lowerleft.x + width){
            // See if the right edge within AOI, right must be a tile
            target = *(currentFind->tr);
            return true;
        }else{
            // Move down to the next tile touching the left edge of AOI
            if(currentFind->getLowerLeft().y <= 1){
                break;
            }
            currentFind = findPoint(Cord(lowerleft.x, currentFind->getLowerLeft().y -1));
        }
    }

    return false;

}

bool LFLegaliser::searchArea(Cord lowerleft, len_t width, len_t height) const{
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *currentFind = findPoint(Cord(lowerleft.x, searchRBorderHeight));
    
    while(currentFind->getUpperLeft().y > lowerleft.y){
        // See if the tile is solid
        if(currentFind->getType() != tileType::BLANK){
            // This is an edge of a solid tile
            return true;
        }else if(currentFind->getUpperRight().x < lowerleft.x + width){
            // See if the right edge within AOI, right must be a tile
            return true;
        }else{
            if(currentFind->getLowerLeft().y <= 1){
                break;
            }
            currentFind = findPoint(Cord(lowerleft.x, currentFind->getLowerLeft().y -1));
        }
    }

    return false;
}

void LFLegaliser::enumerateDirectArea(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles) const{
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *leftTouchTile = findPoint(Cord(lowerleft.x, searchRBorderHeight));

    
    while(leftTouchTile->getUpperLeft().y > lowerleft.y){
        enumerateDirectAreaRProcess(lowerleft, width, height, allTiles, leftTouchTile);
        if(leftTouchTile->getLowerLeft().y <= 1) break;
        leftTouchTile = findPoint(Cord(lowerleft.x, leftTouchTile->getLowerLeft().y -1));
    }
}

void LFLegaliser::enumerateDirectAreaRProcess(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles, Tile *targetTile) const{
    
    // R1) Enumerate the tile
    if(targetTile->getType() == tileType::BLOCK || targetTile->getType() == tileType::OVERLAP){
        allTiles.push_back(targetTile);

    }

    // R2) If the right edge of the tile is outside of the seearch area, return
    if(targetTile->getLowerRight().x >= (lowerleft.x + width)){
        return;
    }

    // R3) Use neighbor-finding algo to locate all the tiles that touch the right side of the current tile and also intersect the search area
    std::vector<Tile *> rightNeighbors;
    findRightNeighbors(targetTile, rightNeighbors);
    for(Tile *t : rightNeighbors){

        // R4) If bottom left corner of the neighbor touches the current tile
        bool R4 = targetTile->checkTRLLTouch(t);
        // R5) If the bottom edge ofthe search area cuts both the urrent tile and the neighbor
        bool R5 = (targetTile->cutHeight(lowerleft.y)) && (t->cutHeight(lowerleft.y));

        if(R4 || R5){
            enumerateDirectAreaRProcess(lowerleft, width, height, allTiles, t);
        }
    }

}

bool LFLegaliser::insertTile(Tile &tile){
    assert(!searchArea(tile.getLowerLeft(), tile.getWidth(), tile.getHeight()));

    /* STEP 1) Find the space Tile containing the top edge of the aera to be occupied, process */
    bool tileTouchesSky = (tile.getUpperRight().y == getCanvasHeight());
    bool cleanTopCut = true;
    Tile *origTop;
    
    if(!tileTouchesSky){
        origTop = findPoint(tile.getUpperLeft());
        cleanTopCut = (origTop->getLowerLeft().y == tile.getupperright().y);
    } 
    
    if((!tileTouchesSky)&&(!cleanTopCut)){
    
        Tile *newDown = new Tile(tileType::BLANK, origTop->getLowerLeft(),origTop->getWidth(), (tile.getUpperLeft().y - origTop->getLowerLeft().y));
        newDown->rt = origTop;
        newDown->lb = origTop->lb;
        newDown->bl = origTop->bl;

        // manipulate neighbors around the split tiles

        // change lower-neighbors' rt pointer to newly created tile
        std::vector <Tile *> origDownNeighbors;
        findDownNeighbors(origTop, origDownNeighbors);
        for(Tile *t : origDownNeighbors){
            if(t->rt == origTop){
                t->rt = newDown;
            }
        }
        
        // change right neighbors to pointer their bl to the correct tile (one of the split)
        std::vector <Tile *> origRightNeighbors;
        findRightNeighbors(origTop, origRightNeighbors);
        
        bool rightModified = false;
        for(int i = 0; i < origRightNeighbors.size(); ++i){
            if(origRightNeighbors[i]->getLowerLeft().y < newDown->getUpperRight().y){
                if(!rightModified){
                    rightModified = true;
                    newDown->tr = origRightNeighbors[i];
                }
                origRightNeighbors[i]->bl = newDown;
                
            }
        }
        assert(rightModified);

        // change Left neighbors to point their tr to the correct tiles
        std::vector <Tile *> origLeftNeighbors;
        findLeftNeighbors(origTop, origLeftNeighbors);

        bool leftModified = false;
        for(int i = 0; i < origLeftNeighbors.size(); ++i){
            if(origLeftNeighbors[i]->getUpperLeft().y > tile.getUpperLeft().y){
                if(!leftModified){
                    leftModified = true;
                    origTop->bl = origLeftNeighbors[i];
                }
            }else{
                origLeftNeighbors[i]->tr = newDown;
            }
        }
        assert(leftModified);

        origTop->setCord((origTop->getLowerLeft().x, tile.getUpperLeft().y));
        origTop->setHeight = (origTop->getUpperLeft().y - tile.getUpperLeft().y)
    }

    /* STEP 2) Find the space Tile containing the bottom edge of the aera to be occupied, process */

    bool tileTouchesGround = (tile.getLowerLeft().y == 0);
    bool cleanBottomCut = true;
    Tile *origBottom;
    if(!tileTouchesGround){
        origBottom = findPoint(tile.getLowerLeft() - Cord(0,1));
        cleanBottomcut = (origBottom->getUpperRight().y == tile.getLowerLeft().y)
    }

    if((!tileTouchesGround) && (!cleanBottomCut)){
        
        tile *newUp = new Tile(tileType::BLANK, Cord(origBottom->getLowerLeft().x, tile.getLowerLeft().y)
                                , origBottom->getWidth(), (origBottom->getUpperLeft().y - tile.getLowerLeft().y));         
        
        newUp->rt = origBottom->rt;
        newUp->lb = origBottom;
        newUp->tr = origBottom->tr;

        // manipulate neighbors around the split tiles

        // change the upper-neighbors' lb pointer to newly created tile
        std::vector <Tile *> origUpNeighbors;
        findTopNeighbors(origBottom, origUpNeighbors);
        for(Tile *t : origUpNeighbors){
            if(t->lb == origBottom){
                t->lb = newUp;
            }
        }

        // change 
        //TODO: adjust right & left neighbors's pointer



    }


}

void LFLegaliser::visualiseArtpiece(const std::string outputFileName) {
    
    std::cout << "print to file..."<< outputFileName <<std::endl;

    std::ofstream ofs(outputFileName);
    ofs << "BLOCK " << fixedTesserae.size() + softTesserae.size() << std::endl;
    ofs << this->mCanvasWidth << " " << this->mCanvasHeight << std::endl;

    if(fixedTesserae.size() == 0 && softTesserae.size() == 0){
        //there is no blocks
        ofs.close();
        return;
    }

    for(Tessera *tess : softTesserae){
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " " << "SOFT_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for(Tile *t : tess->TileArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for(Tile *t : tess->OverlapArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" <<std::endl;
        }
    }

    for(Tessera *tess : fixedTesserae){
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " " << "HARD_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for(Tile *t : tess->TileArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for(Tile *t : tess->OverlapArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" <<std::endl;
        }
    }

    // DFS Traverse through all balnk tiles
    if(fixedTesserae.size() !=0 ){
        if(this->fixedTesserae[0]->TileArr.size() != 0){
            traverseBlank(ofs, *(this->fixedTesserae[0]->TileArr[0]));
        }else{
            traverseBlank(ofs, *(this->fixedTesserae[0]->OverlapArr[0]));
        }
    }else{
        if(softTesserae.size() != 0){
            traverseBlank(ofs, *(this->softTesserae[0]->TileArr[0]));
        }else{
            traverseBlank(ofs, *(this->softTesserae[0]->OverlapArr[0]));
        }
    }
    ofs << "CONNECTION 0" << std::endl;

    ofs.close();
}

void LFLegaliser::traverseBlank(std::ofstream &ofs,  Tile &t) {
    
    t.printLabel = (!t.printLabel);
    
    if(t.getType() == tileType::BLANK){

        ofs << t.getLowerLeft().x << " " << t.getLowerLeft().y << " ";
        ofs << t.getWidth() << " " << t.getHeight() << " ";
        ofs << "BLANK_TILE" << std::endl;
    }

    if(t.rt != nullptr){
        if(t.rt->printLabel != t.printLabel){
            traverseBlank(ofs, *(t.rt));
        }
    }

    if(t.lb != nullptr){
        if(t.lb->printLabel != t.printLabel){
            traverseBlank(ofs, *(t.lb));
        }
    }

    if(t.bl != nullptr){
        if(t.bl->printLabel != t.printLabel){
            traverseBlank(ofs, *(t.bl));
        }
    }

    if(t.tr != nullptr){
        if(t.tr->printLabel != t.printLabel){
            traverseBlank(ofs, *(t.tr));
        }
    }
    
    return;
}