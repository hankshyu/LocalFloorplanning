#include <iostream>
#include "LFUnits.h"
#include "Tile.h"

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
    Tile tt(tileType::BLANK, Cord(3,5), 11, 12);
    printTile(tt);
    std::cout << std::endl;
    printCord(tt.getLowerLeft());
    printCord(tt.getUpperLeft());
    printCord(tt.getLowerRight());
    printCord(tt.getUpperRight());


    std::cout << std::endl;
    tt.setLowerLeft(Cord(87,78));
    printCord(tt.getLowerLeft());
    printCord(tt.getUpperLeft());
    printCord(tt.getLowerRight());
    printCord(tt.getUpperRight());
    std::cout << std::endl;

    tt.setHeight(67);
    tt.setWidth(100);
    printCord(tt.getLowerLeft());
    printCord(tt.getUpperLeft());
    printCord(tt.getLowerRight());
    printCord(tt.getUpperRight());
    std::cout << std::endl;
    

    
    return 0;
}
