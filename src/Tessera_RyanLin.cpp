#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Tessera.h"

Tessera::Tessera()
    : mType(tesseraType::EMPTY) {}

Tessera::Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height)
    : mType(type), mName(name), mLegalArea(area), 
    mInitLowerLeft(lowerleft), mInitWidth(width), mInitHeight(height) {}

std::string Tessera::getName () const{
    return this->mName;
}

area_t Tessera::getLegalArea () const{
    return this->mLegalArea;
}

Cord Tessera::getBBLowerLeft (){
    return this->mBBLowerLeft;
}
Cord Tessera::getBBUpperRight(){
    return this->mBBUpperRight;
}
len_t Tessera::getBBWidth (){
    return this->mBBUpperRight.x - this->mBBLowerLeft.x;
}
len_t Tessera::getBBHeight (){
    return this->mBBUpperRight.y - this->mBBLowerLeft.y;
}

int Tessera::insertTiles(tileType type, Tile *tile){
    assert(type != tileType::BLANK);
    switch (type){
        case tileType::BLOCK:
            /* code */
            TileArr.push_back(tile);
            break;
        case tileType::OVERLAP:
            /* code */
            OverlapArr.push_back(tile);
            break;
        default:
            break;
    }
    calBoundingBox();

    return 1;
}

void Tessera::calBoundingBox(){
    if(TileArr.empty() && OverlapArr.empty()) return;

    //The lowerleft and upper right tiles
    Cord LL, UR;

    if(!TileArr.empty()){
        LL = TileArr[0]->getLowerLeft();
        UR = TileArr[0]->getUpperRight();
    }else{
        LL = OverlapArr[0]->getLowerLeft();
        UR = OverlapArr[0]->getUpperRight();
    }

    for(Tile *t : TileArr){
        if(t->getLowerLeft() < LL) LL = t->getLowerLeft();
        if(t->getUpperRight() > UR) UR = t->getUpperRight();
    }
    for(Tile *t : OverlapArr){
        if(t->getLowerLeft() < LL) LL = t->getLowerLeft();
        if(t->getUpperRight() > UR) UR = t->getUpperRight();  
    }

    this->mBBLowerLeft = LL;
    this->mBBUpperRight = UR;
}

/*
    // TODO
    Notes for Ryan Lin...
    We would call translateGlobalFloorplanning() from LFLegaliser.h to translate cirlces from 
    global floorplanning to Rectangles (Tessera.h), and it would include a default Tile (Tile.h)
    We would then call detectfloorplanningOverlaps() to detect overlaps. Overlaps would be record as
    another Tile(label with tileType::OVERLAP)
    Then call splitFloorplanningOverlaps() from LFLegaliser.h
    it would :
    for(Tessera t : all Tesserae vector){
        t.splitRectliearDueToOverlap()
    }
    
    to acquire the correct tile distribution with overlap tiles correctly split.
*/
void Tessera::splitRectliearDueToOverlap(){
    
    /*
    The translated info is stored in:
    - Cord mInitLowerLeft;
    - len_t mInitWidth;
    - len_t mInitHeight;

    There should be "ONE" default Tile in TileArr, which is the default tile(= InitXXX...)
    Overlap tiles are calculated as pushed into overlapArr
    You should read the entire overlap Arr and mark off the area indicated, this would lead you to 
    a rectlinear shape.
    Then you should further split the acquired rectilinear, and split them into smaller tiles. you "MUST"
    remove the default tile if new smaller splits are pushed in. GOOD LUCK!!

    p.s. Just insert the smaller tiles like so:
    Tile *newsmallersplit = new Tile(tileType::BLOCK, Cord(x, y), width, height);
    
    You don't have to maintain *up *donw *left right pointers, they will be taken care of when insertion happen


    */

    // if overlap exists, default tile will always be split
    // delete default tile, insert tile size of block
    if (OverlapArr.size() > 0 ){
        Tile* defaultTile = TileArr.back();
        Cord lowerLeft = defaultTile->getLowerLeft();
        len_t width = defaultTile->getWidth();
        len_t height = defaultTile->getHeight();
        // Do I need to delete pointer????
        delete defaultTile; 
        Tile* newDefault = new Tile(tileType::BLOCK, lowerLeft, width, height);
        TileArr[0] = newDefault;
    }
    

    // iterate thru all overlaps
    for (int o = 0; o < OverlapArr.size(); o++){
        std::vector <Tile *> influencedTiles;
        Tile* currentOverlap = OverlapArr[o];
        int overlapRightBoundary = currentOverlap->getUpperRight().x;
        int overlapUpperBoundary = currentOverlap->getUpperRight().y;
        int overlapLeftBoundary = currentOverlap->getLowerLeft().x;
        int overlapLowerBoundary = currentOverlap->getLowerLeft().y;

        // step 1: find all block Tiles that will be influenced
        for (int t = 0; t < TileArr.size(); ++t){
            // check overlap
            bool xOverlap = TileArr[t]->getLowerLeft().x < currentOverlap->getUpperRight().x &&
                            currentOverlap->getLowerLeft().x < TileArr[t]->getUpperRight().x;
            bool yOverlap = TileArr[t]->getLowerLeft().y < currentOverlap->getUpperRight().y &&
                            currentOverlap->getLowerLeft().y < TileArr[t]->getUpperRight().y;
            if (xOverlap && yOverlap){
                influencedTiles.push_back(TileArr[t]);
            }
        }

        // step 2: find Tiles that include the upper/lower boundary of overlap,  
        // split those tiles
        for (int i = 0; i < influencedTiles.size(); ++i){
            Tile* currentTile = influencedTiles[i];
            int currentRightBoundary = currentTile->getUpperRight().x;
            int currentUpperBoundary = currentTile->getUpperRight().y;
            int currentLeftBoundary = currentTile->getLowerLeft().x;
            int currentLowerBoundary = currentTile->getLowerLeft().y;
            bool includeUpperBoundary = currentLowerBoundary < overlapUpperBoundary && overlapUpperBoundary < currentUpperBoundary;
            if (includeUpperBoundary){
                // change hieght of currentTile, add a new tile to tile array
                Cord newLL = Cord(currentLeftBoundary, overlapUpperBoundary);
                int newWidth = currentTile->getWidth();
                int newHeight = currentUpperBoundary - overlapUpperBoundary;
                Tile* newTile = new Tile(tileType::BLOCK, newLL, newWidth, newHeight);
                TileArr.push_back(newTile);

                int alterHeight = overlapUpperBoundary - currentLowerBoundary;
                currentTile->setHeight(alterHeight);
                currentUpperBoundary = currentTile->getUpperRight().y;
            }

            bool includeLowerBoundary = currentLowerBoundary < overlapLowerBoundary && overlapLowerBoundary < currentUpperBoundary;
            if (includeLowerBoundary){
                // change LL and height of currentTile, add a new tile to tile array
                Cord newLL = Cord(currentLeftBoundary, currentLowerBoundary);
                int newWidth = currentTile->getWidth();
                int newHeight = overlapLowerBoundary - currentLowerBoundary;
                Tile* newTile = new Tile(tileType::BLOCK, newLL, newWidth, newHeight);
                TileArr.push_back(newTile);

                int alterHeight = currentUpperBoundary - overlapLowerBoundary;
                Cord alterLL = Cord(currentLeftBoundary, overlapLowerBoundary);
                currentTile->setHeight(alterHeight);
                currentTile->setLowerLeft(alterLL);
            }
        }

        // step 3: split all influenced tiles into 2 segments (left, right) if possible
        for (int i = 0; i < influencedTiles.size(); ++i){
            Tile* currentTile = influencedTiles[i];
            int currentRightBoundary = currentTile->getUpperRight().x;
            int currentUpperBoundary = currentTile->getUpperRight().y;
            int currentLeftBoundary = currentTile->getLowerLeft().x;
            int currentLowerBoundary = currentTile->getLowerLeft().y;
            bool includeLeftBoundary = currentLeftBoundary < overlapLeftBoundary && overlapLeftBoundary < currentRightBoundary;
            bool includeRightBoundary = currentLeftBoundary < overlapRightBoundary && overlapRightBoundary < currentRightBoundary;

            // case 1: tile only includes left boundary
            // shrink tile width
            if (includeLeftBoundary && !includeRightBoundary){
                int alterWidth = overlapLeftBoundary - currentLeftBoundary;
                currentTile->setWidth(alterWidth);
            }
            // case 2: tile only includes right boundary
            // adjust LL and shrink tile width
            else if (!includeLeftBoundary && includeRightBoundary){
                Cord alterLL = Cord(overlapRightBoundary, currentLowerBoundary);
                int alterWidth = currentRightBoundary - overlapRightBoundary;
                currentTile->setLowerLeft(alterLL);
                currentTile->setWidth(alterWidth);
            }
            // case 3: tile includes entire overlap interval 
            // shrink tile width, create new tile
            else if (includeLeftBoundary && includeRightBoundary){
                int alterWidth = overlapLeftBoundary - currentLeftBoundary;
                currentTile->setWidth(alterWidth);

                Cord newLL = Cord(overlapRightBoundary, currentLowerBoundary);
                int newWidth = currentRightBoundary - overlapRightBoundary;
                int newHeight = currentTile->getHeight();
                Tile* newTile = new Tile(tileType::BLOCK, newLL, newWidth, newHeight);
                TileArr.push_back(newTile);
                // influencedTiles.push_back(newTile);
            }
            // case 4: overlap is same size or wider than tile
            // reduce width to 0, will clean up later
            else {
                currentTile->setWidth(0);
            }
        }

        // step 4: merge up down neighboring tiles with same width 
        for (int t1 = 0; t1 < TileArr.size(); ++t1){
            Tile* tile1 = TileArr[t1];
            if (tile1->getWidth() == 0){
                continue;
            }
            for (int t2 = 0; t2 < TileArr.size(); ++t2){
                Tile* tile2 = TileArr[t2];
                if (tile1 == tile2 || tile2->getWidth() == 0){
                    continue;
                }
                // 1 is on top of 2
                bool canMerge1 = (tile1->getLowerLeft() == tile2->getUpperLeft()) && (tile1->getLowerRight() == tile2->getUpperRight()); 
                // 2 is on top of 1
                bool canMerge2 = (tile1->getUpperLeft() == tile2->getLowerLeft()) && (tile2->getLowerRight() == tile1->getUpperRight()); 
                
                // shrink 2, expand 1
                if (canMerge1){
                    Cord alterLL = tile2->getLowerLeft();
                    int alterHeight = tile1->getHeight() + tile2->getHeight();
                    tile1->setLowerLeft(alterLL);
                    tile1->setHeight(alterHeight);

                    tile2->setWidth(0);
                }
                else if (canMerge2){
                    int alterHeight = tile1->getHeight() + tile2->getHeight();
                    tile1->setHeight(alterHeight);

                    tile2->setWidth(0);
                }
            }
        }

        // step 5: clean up, remove tiles with 0 width
        std::vector<Tile*> deleteTiles;
        for (auto it = TileArr.begin(); it != TileArr.end(); it++){
            if ((*it)->getWidth() == 0){
                deleteTiles.push_back(*it);
                TileArr.erase(it);
                it--;
            }
        }

        for (int i = 0; i < deleteTiles.size(); i++){
            delete deleteTiles[i];
        }
    }
    
}

void Tessera::printLayout(){
    std::string filename = mName + "layout.txt";

    std::ofstream ofs(filename);
    ofs << "BLOCK " << TileArr.size() + OverlapArr.size() << std::endl;
    ofs << getBBWidth() << " " << getBBHeight() << std::endl;
    ofs << "OUTLINE -1 0 0 " << getBBWidth() << " " << getBBHeight() << " " << "DIE_BLOCK" << std::endl;
    for (int t = 0; t < TileArr.size(); t++){
        Tile* tile = TileArr[t];
        ofs << "Tile"+std::to_string(t) << " 1 " << tile->getLowerLeft().x << " " << tile->getLowerLeft().y << " " << tile->getWidth() << " " << tile->getHeight() << " SOFT_BLOCK" << std::endl;
    }
    
    for (int t = 0; t < OverlapArr.size(); t++){
        Tile* tile = OverlapArr[t];
        ofs << "OVERLAP"+std::to_string(t) << " 1 " << tile->getLowerLeft().x << " " << tile->getLowerLeft().y << " " << tile->getWidth() << " " << tile->getHeight() << " OVERLAP_BLOCK" << std::endl;
    }
    ofs.close();
}