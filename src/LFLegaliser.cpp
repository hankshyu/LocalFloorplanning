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
    
    return 0;
}


void LFLegaliser::visualiseArtpiece(const std::string outputFileName) const{
    
    std::cout << "print to file..."<< outputFileName <<std::endl;

    std::ofstream ofs(outputFileName);
    ofs << "OUTLINE -1 0 0 " << this->mCanvasWidth << " " << this->mCanvasHeight << " " << "DIE_BLOCK" << std::endl;

    if(fixedTesserae.size() == 0 && softTesserae.size() == 0){
        //there is no blocks
        ofs.close();
        return;
    }
    std::cout << fixedTesserae.size() << " " << softTesserae.size() << std::endl;
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

    // Treaverse through all balnk tiles
    if(fixedTesserae.size() !=0 ){
        Tile *blank = nullptr;
        if(this->fixedTesserae[0]->TileArr.size() != 0){
            traverseBlank(ofs, *(this->fixedTesserae[0]->TileArr[0]));

        }else{

        }
    }else{

    }


    ofs.close();
}

void LFLegaliser::traverseBlank(std::ofstream & ofs, const Tile &seed){

}
