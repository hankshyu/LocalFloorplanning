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
    if(t.getType() == tileType::BLOCK){
        std::cout << "Type: Block "; 
    }else if(t.getType() == tileType::OVERLAP){
        std::cout << "Type: OVlap "; 

    }else if(t.getType() == tileType::BLANK){
        std::cout << "Type: Blank "; 
    } else{
        std::cout << "ERRRROR! Type blank!! ";
    }
    std::cout << ", W=" << t.getWidth() << ", H=" << t.getHeight() << std::endl;
}
int main(int argc, char const *argv[])
{
    std::cout << "This is Local floorplanner!" << std::endl;
    
    LFLegaliser lfLegaliser(8, 7);
    lfLegaliser.addFirstTessera(tesseraType::SOFT, "FPU", 6, Cord(2, 2) , 2, 3);
    
    lfLegaliser.visualiseArtpiece("outputs/artpc.txt");

    Tile *find = lfLegaliser.findPoint(Cord (4,4));
    printTile(*find);

    Tile *c = lfLegaliser.softTesserae[0]->TileArr[0];
    printTile(*c);

    std::vector<Tile *> top;
    lfLegaliser.findAllNeighbors(c, top);
    std::cout << "top has: " << top.size() << " elements" << std::endl;
    for(Tile *t : top){
        printTile(*t);
    }
    return 0;
}
