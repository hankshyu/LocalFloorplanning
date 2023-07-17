#include "LFLegaliser.h"

LFLegaliser::LFLegaliser(len_t chipWidth, len_t chipHeight)
    : mCanvasWidth(chipWidth), mCanvasHeight(chipHeight) {}

bool LFLegaliser::checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height){
    bool x_valid, y_valid;
    x_valid = (lowerLeft.x >= 0) && (lowerLeft.x + width <= this->mCanvasWidth);
    y_valid = (lowerLeft.y >= 0) && (lowerLeft.y + height <= this->mCanvasHeight);
    return (x_valid && y_valid);
}

len_t LFLegaliser::getCanvasWidth () const{
    return this->mCanvasWidth;
}

len_t LFLegaliser::getCanvasHeight () const{
    return this->mCanvasHeight;
}

int LFLegaliser::addFirstTessera(tesseraType type, std::string name, area_t area, Cord lowerLeft, len_t width, len_t height){
    
    assert(checkTesseraInCanvas(lowerLeft, width, height));
    assert(type != tesseraType::EMPTY);

    Tessera *newTessera = new Tessera(type, name, area, lowerLeft, width, height);
    Tile *newTile = new Tile(tileType::BLOCK, lowerLeft, width, height);
    
    
    if(type == tesseraType::HARD){
        fixedTesserae.push_back(newTessera);
    }else if(type == tesseraType::SOFT){
        softTesserae.push_back(newTessera);
    }

    //todo: add links
    if(newTile->getLowerLeft().y != 0){
        Tile *tdown = new Tile(tileType::BLANK, Cord(0,0),
                            this->mCanvasWidth, newTile->getLowerLeft().y);
        newTile->down = tdown;
        tdown->up = newTile;
    }

    if(newTile->getUpperRight().y <= this->mCanvasHeight){
        Tile *tup = new Tile(tileType::BLANK, Cord(0,newTile->getUpperRight().y), 
                            this->mCanvasWidth, (this->mCanvasHeight - newTile->getUpperRight().y));
        newTile->up = tup;
        tup->down = newTile;
    }

    if(lowerLeft.x != 0){
        Tile *tleft = new Tile(tileType::BLANK, Cord(0, newTile->getLowerLeft().y),
                            newTile->getLowerLeft().x, height);
        newTile->left = tleft;
        tleft->right = newTile;    
    }
    if((lowerLeft.x + width)!= mCanvasWidth){
        Tile *tright = new Tile(tileType::BLANK, newTile->getLowerRight(), 
                            (this->mCanvasWidth - newTile->getUpperRight().x), height);
        newTile->right = tright;
        tright->left = newTile;
    }

}


void LFLegaliser::visualiseArtpiece(std::string outputFileName) const{

}
