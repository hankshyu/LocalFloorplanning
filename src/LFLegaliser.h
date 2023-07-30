#ifndef __LFLEGALISER_H__
#define __LFLEGALISER_H__


#include <vector>
#include <assert.h>
#include <fstream>
#include <cmath>
#include <boost/polygon/polygon.hpp>
#include <boost/polygon/rectangle_data.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include "LFUnits.h"
#include "Tile.h"
#include "Tessera.h"
#include "ppsolver.h"

namespace bp = boost::polygon;
namespace bg = boost::geometry;

typedef bg::model::d2::point_xy<int> Point;
typedef bg::model::box<Point> Box;
typedef bp::rectangle_data<len_t> Rectangle;

class PPSolver;

class LFLegaliser {
private:
    len_t mCanvasWidth;
    len_t mCanvasHeight;
    bool overlap3;

    bool checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height) const;
    bool checkTileInCanvas(Tile &tile) const;
    void traverseBlank(std::ofstream &ofs, Tile &t);
    Tile *getRandomTile() const;

    // subRoutine used in enumerateDirectArea
    void enumerateDirectAreaRProcess(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles, Tile *targetTile) const;

    std::vector <Tile *> mMarkedTiles;


public:
    std::vector <Tessera *> fixedTesserae;
    std::vector <Tessera *> softTesserae;

    LFLegaliser(len_t chipWidth, len_t chipHeight);
    ~LFLegaliser();

    len_t getCanvasWidth() const;
    len_t getCanvasHeight() const;

    void translateGlobalFloorplanning(const PPSolver &solver);
    void detectfloorplanningOverlaps();
    bool has3overlap();
    void splitFloorplanningOverlaps();





    // int addFirstTessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height);

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

    // Enumerates all tiles in a given area, each tile is visited(pushed into vector) only after all the tiles above and to
    // its left is visited
    void enumerateDirectArea(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles) const;

    // pushes 
    void insertFirstTile(Tile &newtile);
    void insertTile(Tile &tile);

    // deleteTile (Don't need)
    void visualiseArtpiece(const std::string outputFileName);
    void visualiseArtpieceCYY(const std::string outputFileName);
    void visualiseAddMark(Tile *markTile);

};

#endif // __LFLEGALISER_H__