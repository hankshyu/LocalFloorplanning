#include <iostream>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "LFLegaliser.h"

void printCord(const Cord &c){
    std::cout << "(" << c.x << ", " << c.y << ")";
}
void printTile(const Tile &t){
    printCord(t.getLowerLeft());
    std::cout << ", W=" << t.getWidth() << ", H=" << t.getHeight() << std::endl;
}
int main(int argc, char const *argv[])
{
    std::cout << "This is Local floorplanner!" << std::endl;
    
    LFLegaliser lfLegaliser(8, 7);
    lfLegaliser.addFirstTessera(tesseraType::SOFT, "FPU", 6, Cord(2, 2) , 2, 3);
    
    if(lfLegaliser.softTesserae[0]->TileArr[0]->up == nullptr){
        std::cout << "Direction: up is nullptr" << std::endl;
    }
    if(lfLegaliser.softTesserae[0]->TileArr[0]->down == nullptr){
        std::cout << "Direction: down is nullptr" << std::endl;
    }
    if(lfLegaliser.softTesserae[0]->TileArr[0]->left == nullptr){
        std::cout << "Direction: left is nullptr" << std::endl;
    }
    if(lfLegaliser.softTesserae[0]->TileArr[0]->right == nullptr){
        std::cout << "Direction: right is nullptr" << std::endl;
    }

    lfLegaliser.visualiseArtpiece("outputs/artpc.txt");

    return 0;
}
