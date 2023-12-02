#ifndef __LFLEGALISER_H__
#define __LFLEGALISER_H__

#include <vector>
#include <assert.h>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <utility>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "ppsolver.h"
#include "rgsolver.h"

namespace pp = PushPull;
namespace rg = RectGrad;

class LFLegaliser {
private:
    len_t mCanvasWidth;
    len_t mCanvasHeight;

    bool overlap3;

    bool checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height) const;
    bool checkTileInCanvas(Tile &tile) const;

    void traverseBlank(std::ofstream &ofs, Tile &t, std::vector <Cord> &record);
    // void visualiseResetDFS(Tile &t, std::vector <Cord> &record);
    void visualiseDebugDFS(std::ofstream &ofs, Tile &t, std::vector <Cord> &record);


    // subRoutine used in enumerateDirectArea
    void enumerateDirectAreaRProcess(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles, Tile *targetTile) const;

    // This is for marking tiles to show on presentation
    std::vector <Tile *> mMarkedTiles;

    void detectCombinableBlanksDFS(std::vector <std::pair<Tile *, Tile *>> &candidateTile, Tile &t, std::vector <Cord> &record);
    void collectAllTilesDFS(Tile &head, std::vector <Cord> &record, std::vector<Tile *> &allTiles) const;

public:
    std::vector <Tessera *> fixedTesserae;
    std::vector <Tessera *> softTesserae;

    LFLegaliser() = delete;
    LFLegaliser(len_t chipWidth, len_t chipHeight);
    LFLegaliser(const LFLegaliser &other);
    ~LFLegaliser();

    // LFLegaliser& operator = (const LFLegaliser &other);

    len_t getCanvasWidth() const;
    len_t getCanvasHeight() const;

    void translateGlobalFloorplanning(const pp::GlobalSolver &solver);
    void translateGlobalFloorplanning(const rg::GlobalSolver &solver);
    void detectfloorplanningOverlaps();
    bool has3overlap();
    void splitTesseraeOverlaps();

    void arrangeTesseraetoCanvas();

    Tile *getRandomTile() const;


    /* Functions proposed in the paper */

    // Returns the Tile that includes the Cord "key"
    Tile *findPoint(const Cord &key) const;
    Tile *findPoint(const Cord &key, Tile *initTile) const;

    // Pushes all neighbors of Tile "centre" to vector "neighbors"
    void findTopNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findDownNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findLeftNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findRightNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;
    void findAllNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const;

    // Determine if there is any solid tiles insice the area (lowerleft, width, height), tile return through "target"
    bool searchArea(Cord lowerleft, len_t width, len_t height, Tile &target) const;
    // Clone of searchArea, no "target" is returned
    bool searchArea(Cord lowerleft, len_t width, len_t height) const;

    // Enumerates all tiles in a given area, each tile is visited only after all the tiles above and to its left does
    void enumerateDirectArea(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles) const;
    

    void insertFirstTile(Tile &newtile);
    void insertTile(Tile &tile);
    
    Tile* simpleSplitTile(Tile& tile, Rectangle rect, int direction);

    void visualiseArtpiece(const std::string outputFileName, bool checkBlankTile);
    void visualiseDebug(const std::string outputFileName);
    void visualiseAddMark(Tile *markTile);
    void visualiseRemoveAllmark();

    void detectCombinableBlanks(std::vector <std::pair<Tile *, Tile *>> &candidateTile);
    void combineVerticalMergeableBlanks(Tile *upTile, Tile *downTile);

    bool searchTesseraeIncludeTile(Tile *tile, std::vector <Tessera *> &inTessera) const;

    void printOutput(std::string outputFileName);
    void collectAllTiles(std::vector<Tile *> &allTiles) const;
};

bool checkVectorInclude(std::vector<Cord> &vec, Cord c);
bool checkVectorInclude(std::vector<Tessera *>&vec, Tessera *tess);
// return -1 if not found, otherwise index
int findVectorInclude(std::vector<Tile *>&vec, Tile *t);
// return -1 if not found, otherwise index
int findVectorIncludebyName(std::vector<Tessera *>&vec, Tessera *tess);

double calculateHPWL(LFLegaliser *legaliser, const std::vector<rg::ConnStruct> &connections, bool printReport);
void outputFinalAnswer(LFLegaliser *legaliser, const rg::Parser &rgparser, const std::string outputFileName);

struct CPTilePair{
    Tile *father;
    Tile *baby;
};


#endif // __LFLEGALISER_H__