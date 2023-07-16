#include "LFLegaliser.h"

LFLegaliser::LFLegaliser(len_t chipWidth, len_t chipHeight)
    : mCanvasWidth(chipWidth), mCanvasHeight(chipHeight) {}

bool LFLegaliser::checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height){
    bool x_valid, y_valid;
    x_valid = (lowerLeft.x >= 0) && (lowerLeft.x + width <= this->mCanvasWidth);
    y_valid = (lowerLeft.y >= 0) && (lowerLeft.y + height <= this->mCanvasHeight);
    return (x_valid && y_valid);
}

//might need more info
int LFLegaliser::addFirstTessera(tesseraType type, Cord lowerLeft, len_t width, len_t height){
    
    assert(checkTesseraInCanvas(lowerLeft, width, height));
    assert(type != tesseraType::EMPTY);

    Tessera *newTessera = new Tessera(.....);



}