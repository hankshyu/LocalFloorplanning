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
    lfLegaliser.addFirstTessera(tesseraType::SOFT, "FPU", 6, Cord(5, 0) , 3, 2);
    lfLegaliser.visualiseArtpiece("outputs/artpc.txt");
    
    return 0;
}
