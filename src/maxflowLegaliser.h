#ifndef _MAXFLOWLEGALISER_H_
#define _MAXFLOWLEGALISER_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include "maxflow.h"
#include "LFLegaliser.h"

namespace MFL {
    const std::string SUPERSOURCE = "SUPERSOURCE";
    const std::string SUPERSINK = "SUPERSINK";

    class MaxflowLegaliser;
    struct MFLTileInfo;
    struct MFLTessInfo;
    struct MFLSingleFlowInfo;
    struct MFLFlowTessInfo;

    enum MFLTessType { OVERLAP, FIXED, SOFT, BLANK };

    struct MFLTessInfo{
        int index;
        std::string tessName;
        std::vector<Tile*> tileList;
        MFLTessType type;
        std::set<int> overlaps;
        std::vector<Tile*> allNeighborTiles;
        std::set<int> tessNeighbors;
        // std::vector<Tile*> validNeighbors; // only neighboring nodes with flow relation
        int area;
        void initMembers(LFLegaliser* LF, MaxflowLegaliser* MFL);
    };

    class MaxflowLegaliser{
    private:
        std::vector<MFLTessInfo> mAllOverlaps;
        std::vector<MFLTessInfo> mAllBlocks;
        std::vector<MFLTessInfo> mAllBlanks;

        std::map<Tile*,int> mTilePtr2TessInfoIdx;

        // bool checkOverlapDuplicate(Tile* overlap);
        void addOverlapInfo(Tile* tile, MFLTessInfo& tess);
        void addSingleOverlapInfo(Tile* tile, int overlapIdx1, int overlapIdx2);
        void MFLTraverseBlank(Tile* tile, std::vector <Cord> &record);
        void constructSingleFlow(MFLTessInfo& sourceTessInfo, MFLFlowTessInfo* sourceFlowInfo, 
                                 MFLTessInfo& destTessInfo, MFLFlowTessInfo* destFlowInfo );

        LFLegaliser* mLF;
        MaxFlow mMaxflowManager;
        int mMaxflowInf;
        int mFixedTessNum;

    public:
        MaxflowLegaliser(/* args */);
        ~MaxflowLegaliser();
        
        // initializes MaxflowLegalizer 
        // finding all TILES and builds flow network
        void initMFL(LFLegaliser* floorplan);
        // runs maxflow with given flow network
        void legaliseByMaxflow();
        void outputFlows(std::vector<MFLFlowTessInfo*>& overlapTileFlows, 
                            std::vector<MFLFlowTessInfo*>& blockTileFlows,
                            std::vector<MFLFlowTessInfo*>& blankTileFlows);
        friend void MFLTessInfo::initMembers(LFLegaliser* LF, MaxflowLegaliser* MFL);
    };

    struct MFLTileInfo{
        std::string nodeName;
        std::vector<Tile*> allNeighbors;
        std::vector<Tile*> validNeighbors; // only neighboring nodes with flow relation
        // std::vector<int> overlapFixedIdx;
        // std::vector<int> overlapSoftIdx;
        
        Tile* tile;
    };

    // this->tileList contains all the tiles that make up this 
    // "Tessera". this->fromFlows includes all the flows that originate from THIS
    // tessera. this->toFlows includes all the flows that flow TO this tessera
    // 
    // IF MFLFlowTessInfo.type is MFLTessType::OVERLAP :
    // this->tileList will include all the tiles that make up this overlap
    // this->softOverlaps/this->fixedOverlaps will contain the indexes of the two 
    // tesseras that make up this overlap (ie. index in 
    // LFLegaliser::softTesserae/fixedTesserae)
    //
    // IF MFLFlowTessInfo.type is MFLTessType::SOFT or MFLTessType::FIXED :
    // this->tileList will include all the tiles that make up this tessera
    // this->tessIndex will contain the index of this tessera (ie. index in 
    // LFLegaliser::softTesserae/fixedTesserae)
    //
    // IF MFLFlowTessInfo.type is MFLTessType::BLANK :
    // this->tileList will include one tile (the blank tile)
    struct MFLFlowTessInfo {
        std::vector<Tile*> tileList;
        MFLTessType type;
        int tessIndex;
        std::vector<int> softOverlaps;
        std::vector<int> fixedOverlaps;
        std::vector<MFLSingleFlowInfo> toFlows;
        std::vector<MFLSingleFlowInfo> fromFlows;
    };

    struct MFLSingleFlowInfo {
        MFLFlowTessInfo* sourceTile;
        MFLFlowTessInfo* destTile; 
        int flowAmount;
    };

    std::string toOverlapName(int tessIndex1, int tessIndex2);
}

#endif