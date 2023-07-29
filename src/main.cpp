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
    
    // LFLegaliser lfLegaliser(8, 7);
    // Tessera *firstT = new Tessera(tesseraType::SOFT, "FPU", 6, Cord(2, 2) , 2, 3);
    // lfLegaliser.softTesserae.push_back(firstT);
    // lfLegaliser.insertFirstTile(*(firstT->TileArr[0]));
    
    // Tile *newTile = new Tile(tileType::OVERLAP, Cord (1,1), 2, 1);
    // firstT->OverlapArr.push_back(newTile);
    // lfLegaliser.insertTile(*(firstT->OverlapArr.back()));

    // Tile *newTile2 = new Tile(tileType::BLOCK, Cord(2, 5), 1, 1);
    // firstT->TileArr.push_back(newTile2);
    // lfLegaliser.insertTile(*(firstT->TileArr.back()));

    LFLegaliser LFLegaliser(12, 10);
    
    Tessera *blueT = new Tessera(tesseraType::HARD, "PAD0", 10, Cord(2, 3), 4, 3);
    LFLegaliser.fixedTesserae.push_back(blueT);
    std::cout <<"BB-LL: ";
    printCord(blueT->getBBLowerLeft());
    std::cout <<"\nBB-UR: ";
    printCord(blueT->getBBUpperRight());
    std::cout << std::endl;

    blueT->TileArr.pop_back();
    Tile *t1 = new Tile(tileType::BLOCK, Cord(4, 7), 1, 1);
    Tile *t2 = new Tile(tileType::BLOCK, Cord(2, 4), 3, 3);
    Tile *t3 = new Tile(tileType::BLOCK, Cord(3, 3), 3, 1);

    Tile *t4 = new Tile(tileType::OVERLAP, Cord(6, 4), 1, 1);
    Tile *t5 = new Tile(tileType::OVERLAP, Cord(6, 3), 2, 1);

    blueT->insertTiles(t1);
    blueT->insertTiles(t2);
    blueT->insertTiles(t3);

    blueT->insertTiles(t4);
    blueT->insertTiles(t5);


    Tessera *greenT = new Tessera(tesseraType::SOFT, "CPU", 13, Cord(6, 2), 5, 3);
    LFLegaliser.softTesserae.push_back(greenT);
    greenT->TileArr.pop_back();
    Tile *t6 = new Tile(tileType::BLOCK, Cord(7, 4), 3, 1);
    Tile *t7 = new Tile(tileType::BLOCK, Cord(8, 2), 4, 2);

    greenT->insertTiles(t4);
    greenT->insertTiles(t5);

    greenT->insertTiles(t6);
    greenT->insertTiles(t7);

    std::cout <<"Insertions: \nBB-LL: ";
    printCord(blueT->getBBLowerLeft());
    std::cout <<"\nBB-UR: ";
    printCord(blueT->getBBUpperRight());
    std::cout << std::endl;

    bool insertedTile = false;
    for(int i = 0; i < blueT->TileArr.size(); ++i){
        if(!insertedTile){
            insertedTile = true;
            LFLegaliser.insertFirstTile(*(blueT->TileArr[i]));
        }else{
            LFLegaliser.insertTile(*(blueT->TileArr[i]));
        }
    }

    for(int i = 0; i < blueT->OverlapArr.size(); ++i){
        if(!insertedTile){
            insertedTile = true;
            LFLegaliser.insertFirstTile(*(blueT->OverlapArr[i]));
        }else{
            LFLegaliser.insertTile(*(blueT->OverlapArr[i]));
        }
    }

    for(int i = 0; i < greenT->TileArr.size(); ++i){
        if(!insertedTile){
            insertedTile = true;
            LFLegaliser.insertFirstTile(*(greenT->TileArr[i]));
        }else{
            LFLegaliser.insertTile(*(greenT->TileArr[i]));
        }
    }

    // for(int i = 0; i < greenT->OverlapArr.size(); ++i){
    //     if(!insertedTile){
    //         insertedTile = true;
    //         LFLegaliser.insertFirstTile(*(blueT->OverlapArr[i]));
    //     }else{
    //         LFLegaliser.insertTile(*(blueT->OverlapArr[i]));
    //     }
    // }

    

    LFLegaliser.visualiseArtpiece("outputs/artpc.txt");
    LFLegaliser.viewLinks("outputs/printlinks.txt");

}
