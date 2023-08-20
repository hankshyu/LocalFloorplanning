#ifndef _DFSLEGALIZER_H_
#define _DFSLEGALIZER_H_

#include <string>
#include <vector>
#include <map>
#include "maxflow.h"
#include "LFLegaliser.h"

namespace DFSL {

struct DFSLNode;
struct DFSLEdge;
struct Segment;

enum class MFLTessType : unsigned char { OVERLAP, FIXED, SOFT, BLANK };

enum class DIRECTION : unsigned char { TOP, RIGHT, DOWN, LEFT };

class DFSLegalizer{
private:
    std::vector<DFSLNode*> mAllNodes;
    std::vector<int> mBestPath;
    std::vector<int> mCurrentPath;
    std::multimap<Tile*, int> mTilePtr2NodeIndex;
    int startingNode;
    LFLegaliser* mLF;
    int mFixedTessNum;
    int mSoftTessNum;
    int mOverlapNum;
    int mBlankNum;
    
    bool migrateOverlap(int overlapIndex);
    void dfs(/**/);
    void addOverlapInfo(Tile* tile);
    void addSingleOverlapInfo(Tile* tile, int overlapIdx1, int overlapIdx2);
    std::string toOverlapName(int tessIndex1, int tessIndex2);
    void DFSLTraverseBlank(Tile* tile, std::vector <Cord> &record);
    void findEdge(int fromIndex, int toIndex);
    void getTessNeighbors(int nodeId, std::set<int> allNeighbors);

public:
    DFSLegalizer();
    ~DFSLegalizer();
    void initDFSLegalizer(LFLegaliser* floorplan);
    void legalize(LFLegaliser* floorplan);
    void constructGraph();
};

static bool compareSegment(Segment a, Segment b);

struct DFSLNode {
    std::vector<Tile*> tileList; 
    std::vector<DFSLEdge> edgeList;
    std::set<int> overlaps;
    std::string nodeName;
    MFLTessType nodeType;
    int index;
};

struct Segment {
    Cord segStart;
    Cord segEnd;
    // Segment(Cord start, Cord end);
};

struct DFSLEdge {
    int fromIndex;
    int toIndex; 
    Segment commonEdge;  
    
    DIRECTION direction;
};


// class DFSLegalizer{
// private:
//     std::vector<MFLTileInfo> mAllOverlaps;
//     std::vector<MFLTileInfo> mAllBlocks;
//     std::vector<MFLTileInfo> mAllBlanks;

//     std::map<Tile*,int> mTilePtr2SoftTessIdx;
//     std::map<Tile*,int> mTilePtr2FixedTessIdx;

//     bool checkOverlapDuplicate(Tile* overlap);
//     void addTileInfo(Tile* tile);
//     void MFLTraverseBlank(Tile* tile, std::vector <Cord> &record);

//     LFLegaliser* mLF;
//     MaxFlow mMaxflowManager;
//     int mMaxflowInf;
//     void makeSingleFlow(MFLSingleFlowInfo& flow, Tile* source, Tile* dest, int flowAmt);

// public:
//     DFSLegalizer(/* args */);
//     ~DFSLegalizer();
    
//     // initializes MaxflowLegalizer 
//     // finding all TILES and builds flow network
//     void initMFL(LFLegaliser* floorplan);
//     // runs maxflow with given flow network
//     void legaliseByMaxflow();
//     void outputFlows(std::vector<MFLTileFlowInfo>& overlapTileFlows, 
//                         std::vector<MFLTileFlowInfo>& blockTileFlows,
//                         std::vector<MFLTileFlowInfo>& blankTileFlows);
// };

// struct MFLTileInfo{
//     std::string nodeName;
//     std::vector<Tile*> allNeighbors;
//     std::vector<Tile*> validNeighbors; // only neighboring nodes with flow relation
//     // std::vector<int> overlapFixedIdx;
//     // std::vector<int> overlapSoftIdx;
    
//     Tile* tile;
// };


// enum DIRECTION { TOP, RIGHT, DOWN, LEFT };

// // eg. There exists a flow from Tile A to Tile B. Tile B is on top of Tile A (Tile A's top neighbor).
// // sourceTile = &(Tile A);
// // destTile = &(Tile B);
// // flowAmount = (how much area needs to be migrated from Tile A to Tile B);
// // direction = TOP;

// struct MFLSingleFlowInfo {
//     Tile* sourceTile;
//     Tile* destTile; 
//     int flowAmount;
//     DIRECTION direction;
// };

// struct MFLTileFlowInfo {
//     Tile* tile;
//     std::vector<MFLSingleFlowInfo> fromFlows;
//     // std::vector<MFLSingleFlowInfo> toFlows;
// };
// struct MFLTesseraFlowInfo {
//     std::vector<Tile*> tileArr;
//     std::vector<Tile*> overlapArr;
//     // includes all non-zero flows originating FROM tiles belonging in this->tileArr to anywhere else 
//     std::vector<MFLSingleFlowInfo> fromFlows; 
// };
}

#endif