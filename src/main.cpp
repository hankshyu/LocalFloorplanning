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
    Tessera *firstT = new Tessera(tesseraType::SOFT, "FPU", 6, Cord(2, 2) , 2, 3);
    lfLegaliser.softTesserae.push_back(firstT);
    lfLegaliser.insertFirstTile(*(firstT->TileArr[0]));
    
    lfLegaliser.visualiseArtpiece("outputs/artpc.txt");
    lfLegaliser.viewLinks("outputs/printlinks.txt");

    // Tile *find = lfLegaliser.findPoint(Cord (7,3));
    // printTile(*find);

    // find = lfLegaliser.findPoint(Cord (0, 5));
    // printTile(*find);

    // Tile f;
    // bool findb = lfLegaliser.searchArea(Cord (3, 4), 1, 2, f);
    // std::cout << findb << std::endl;
    // if(findb) f.show();

    // std::cout << "Start EDA !!" <<std::endl;
    // std::vector <Tile *> eda;
    // lfLegaliser.enumerateDirectArea(Cord(2, 2), 3, 3, eda);
    // std::cout << "EDA: " << eda.size() << std::endl;
    // for(Tile *t : eda){
    //     t->show();
    // }

    // Tile *newTile = new Tile(tileType::BLOCK, Cord(6, 1), 1, 2);
    // Tile *newTile = new Tile(tileType::BLOCK, Cord(6, 1), 1, 3);
    // lfLegaliser.insertTile(*newTile);


}
