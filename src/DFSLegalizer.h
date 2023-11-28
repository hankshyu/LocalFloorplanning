#ifndef _DFSLEGALIZER_H_
#define _DFSLEGALIZER_H_

#include <string>
#include <vector>
#include <map>
#include "LFLegaliser.h"

namespace DFSL {

#define PI 3.14159265358979323846
#define UTIL_RULE 0.8
#define ASPECT_RATIO_RULE 2.0 

struct DFSLNode;
struct DFSLEdge;
struct Segment;
struct LegalInfo;
struct OverlapArea;
struct MigrationEdge;
struct Config;

enum class DFSLTessType : unsigned char { OVERLAP, FIXED, SOFT, BLANK };

enum class DIRECTION : unsigned char { TOP, RIGHT, DOWN, LEFT, NONE };

enum class RESULT : unsigned char { SUCCESS, OVERLAP_NOT_RESOLVED, CONSTRAINT_FAIL };

struct Config {
    Config();
    double maxCostCutoff;
    double OBAreaWeight;
    double OBUtilWeight;
    double OBAspWeight;
    double BWUtilWeight;
    double BWAspWeight;
    double BBFlatCost;
    double WWFlatCost;  
};


class DFSLegalizer{
private:
    std::vector<DFSLNode> mAllNodes;
    double mBestCost;
    int mMigratingArea;
    std::vector<MigrationEdge> mBestPath;
    std::vector<MigrationEdge> mCurrentPath;
    std::multimap<Tile*, int> mTilePtr2NodeIndex;
    std::vector<OverlapArea> mTransientOverlapArea;
    LFLegaliser* mLF;
    int mFixedTessNum;
    int mSoftTessNum;
    int mOverlapNum;
    int mBlankNum;
    
    bool migrateOverlap(int overlapIndex);
    void dfs(DFSLEdge& edge, double currentCost);
    // predicted cost, predicted tile
    MigrationEdge getEdgeCost(DFSLEdge& edge);
    void addOverlapInfo(Tile* tile);
    void addSingleOverlapInfo(Tile* tile, int overlapIdx1, int overlapIdx2);
    std::string toOverlapName(int tessIndex1, int tessIndex2);
    void DFSLTraverseBlank(Tile* tile, std::vector <Cord> &record);
    void findEdge(int fromIndex, int toIndex);
    void getTessNeighbors(int nodeId, std::set<int>& allNeighbors);
    LegalInfo getLegalInfo(std::vector<Tile*>& tiles); 
    LegalInfo getLegalInfo(std::set<Tile*>& tiles); 
    void addBlockNode(Tessera* tess, bool isFixed);
    bool splitOverlap(MigrationEdge& edge, int resolvableArea);
    void removeIndexFromOverlap(int indexToRemove, Tile* overlapTile);
public:
    DFSLegalizer();
    ~DFSLegalizer();
    void initDFSLegalizer(LFLegaliser* floorplan);
    RESULT legalize(int mode);
    void constructGraph();
    Config config;
};

bool removeFromVector(int a, std::vector<int>& vec);

bool removeFromVector(Tile* a, std::vector<Tile*>& vec);

static bool compareXSegment(Segment a, Segment b);
static bool compareYSegment(Segment a, Segment b);

struct DFSLNode {
    DFSLNode();
    std::vector<Tile*> tileList; 
    std::vector<DFSLEdge> edgeList;
    std::set<int> overlaps;
    std::string nodeName;
    DFSLTessType nodeType;
    int area;
    int index;
};

struct Segment {
    Cord segStart;
    Cord segEnd;
    DIRECTION direction;
};

struct DFSLEdge {
    int fromIndex;
    int toIndex; 
    std::vector<Segment> tangentSegments;
};

struct MigrationEdge {
    int fromIndex;
    int toIndex;
    Segment segment;
    Tile migratedArea; // replace this with boost rectangle
    double edgeCost;
    MigrationEdge();
    MigrationEdge(int from, int to, Tile& area, Segment& seg, double cost);
};

struct LegalInfo {
    // bounding box related
    Cord BL;
    int width;
    int height;
    int bbArea;
    double aspectRatio; // w/h

    // utilization
    int actualArea;
    double util;
};

struct OverlapArea {
    int index1;
    int index2;
    int area;
};

}

#endif