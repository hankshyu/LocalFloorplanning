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

void LFLegaliser::traverseBlank(std::ofstream &ofs,  Tile &t) {
    
    t.printLabel = (!t.printLabel);
    
    if(t.getType() == tileType::BLANK){

        ofs << "B " << t.getLowerLeft().x << " " << t.getLowerLeft().y << " ";
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

void LFLegaliser::findTopNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    Tile *n = centre->tr;
    neighbors.push_back(n);
    n = n->lb;
    while(n->getLowerLeft().y > centre->getLowerLeft().y){
        neighbors.push_back(n);
        n = n->lb;
    }
}

void LFLegaliser::findDownNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{

}

void LFLegaliser::findLeftNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{

}

void LFLegaliser::findRightNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{

}

void LFLegaliser::findAllNeighbors(const Tile *centre, std::vector<Tile *> &neighbors) const{
    findTopNeighbors(centre, neighbors);
}



void LFLegaliser::visualiseArtpiece(const std::string outputFileName) {
    
    std::cout << "print to file..."<< outputFileName <<std::endl;

    std::ofstream ofs(outputFileName);
    ofs << "OUTLINE -1 0 0 " << this->mCanvasWidth << " " << this->mCanvasHeight << " " << "DIE_BLOCK" << std::endl;

    if(fixedTesserae.size() == 0 && softTesserae.size() == 0){
        //there is no blocks
        ofs.close();
        return;
    }

    for(Tessera *tess : softTesserae){
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " ";
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for(Tile *t : tess->TileArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << std::endl;
        }
        for(Tile *t : tess->OverlapArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << std::endl;
        }
    }

    for(Tessera *tess : fixedTesserae){
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " ";
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for(Tile *t : tess->TileArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << std::endl;
        }
        for(Tile *t : tess->OverlapArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << std::endl;
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

    ofs.close();
}
