#include "LFLegaliser.h"

LFLegaliser::LFLegaliser(len_t chipWidth, len_t chipHeight)
    : mCanvasWidth(chipWidth), mCanvasHeight(chipHeight) {}

bool LFLegaliser::checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height){
    bool x_valid, y_valid;
    x_valid = (lowerLeft.x >= 0) && (lowerLeft.x + width <= this->mCanvasWidth);
    y_valid = (lowerLeft.y >= 0) && (lowerLeft.y + height <= this->mCanvasHeight);
    return (x_valid && y_valid);
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

    std::vector <Tile *> TileArr;

    

    if(lowerLeft.y != 0){
        Tile *tdown = new Tile(tileType::BLANK, Cord(0,0), this->mCanvasWidth, lowerLeft.y);
    }
    if((lowerLeft.y + height) != mCanvasHeight){
        // Tile *tup = new Tile (Cord (0, ))
    }

    if(lowerLeft.x != 0){

    }
    if((lowerLeft.x + width)!= mCanvasWidth){

    }


}