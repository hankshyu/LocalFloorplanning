#ifndef __LFLEGALISER_H__
#define __LFLEGALISER_H__


#include <vector>
#include <assert.h>
#include <fstream>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"


class LFLegaliser{
private:
    len_t mCanvasWidth;
    len_t mCanvasHeight;
    
    bool checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height) const;
    void traverseBlank(std::ofstream &ofs, Tile &t) ;
    Tile *getRandomTile() const;

public:
    std::vector <Tessera*> fixedTesserae;
    std::vector <Tessera*> softTesserae;

    LFLegaliser() = delete;
    LFLegaliser(len_t chipWidth, len_t chipHeight);

    len_t getCanvasWidth() const;
    len_t getCanvasHeight() const;

    void translateGlobalFloorplanning();
    void detectfloorplanningOverlaps();
    void splitFloorplanningOverlaps();



    int addFirstTessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);

    /* 5 functions proposed in the paper */
    Tile *findPoint(const Cord &key) const;
    
    void findTopNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findDownNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findLeftNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findRightNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findAllNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;

    // searchArea
    // enumerateDirectArea
    // createTile

    // deleteTile (Don't need)
    void visualiseArtpiece(const std::string outputFileName);

};

#endif // __LFLEGALISER_H__