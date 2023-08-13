#include "maxflowLegaliser.h"
#include "maxflow.h"
#include "Tile.h"
#include "LFLegaliser.h"
#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <cstdint>

namespace MFL {
    MaxflowLegaliser::MaxflowLegaliser(/* args */)
    {
    }

    MaxflowLegaliser::~MaxflowLegaliser()
    {
    }

    void MFLTessInfo::initMembers(LFLegaliser* LF, MaxflowLegaliser* MFL){
        area = 0;
        std::vector<Tile*> allNeighborsDuplicate;
        for (Tile* tile: tileList){
            area += tile->getArea();
            LF->findAllNeighbors(tile, allNeighborsDuplicate);
        }

        for (Tile* foundNeighbor: allNeighborsDuplicate){
            bool found = false;
            for (Tile* tileInTess: tileList){
                if (tileInTess == foundNeighbor){
                    found = true;
                    break;
                }
            }
            for (Tile* existingNeighbor: allNeighborTiles){
                if (existingNeighbor == foundNeighbor){
                    found = true;
                    break;
                }
            }

            if (!found){
                allNeighborTiles.push_back(foundNeighbor);
            }
        }

        for (Tile* neighbor: allNeighborTiles){
            if (neighbor->getType() == tileType::BLOCK){
                int tessIndex = MFL->mTilePtr2TessInfoIdx[neighbor];
                tessNeighbors.insert(tessIndex);
            }
        }
    }

    void MaxflowLegaliser::initMFL(LFLegaliser* floorplan){
        // find all overlaps and blocks

        mLF = floorplan;
        mMaxflowInf = floorplan->getCanvasHeight() * floorplan->getCanvasWidth();
        mFixedTessNum = floorplan->fixedTesserae.size();

        for(int t = 0; t < floorplan->fixedTesserae.size(); t++){
            Tessera* tess = floorplan->fixedTesserae[t];
            MFLTessInfo newTessInfo;
            newTessInfo.index = t;
            newTessInfo.tessName = tess->getName();
            newTessInfo.type = MFLTessType::FIXED;

            for(Tile* tile : tess->TileArr){
                newTessInfo.tileList.push_back(tile);
                mTilePtr2TessInfoIdx.insert(std::pair<Tile*,int>(tile, t));
            }

            for(Tile* overlap : tess->OverlapArr){
                addOverlapInfo(overlap, newTessInfo);
            }
            newTessInfo.overlaps.erase(newTessInfo.index);

            mAllBlocks.push_back(newTessInfo);
        }

        for(int t = 0; t < floorplan->softTesserae.size(); t++){
            Tessera* tess = floorplan->softTesserae[t];
            MFLTessInfo newTessInfo;
            newTessInfo.index = t + mFixedTessNum;
            newTessInfo.tessName = tess->getName();
            newTessInfo.type = MFLTessType::SOFT;

            for(Tile* tile : tess->TileArr){
                newTessInfo.tileList.push_back(tile);
                mTilePtr2TessInfoIdx.insert(std::pair<Tile*,int>(tile, t + mFixedTessNum));
            }

            for(Tile* overlap : tess->OverlapArr){
                addOverlapInfo(overlap, newTessInfo);
            }
            newTessInfo.overlaps.erase(newTessInfo.index);

            mAllBlocks.push_back(newTessInfo);
        }

        // find all blanks
        std::vector <Cord> blankRecord;
        MFLTraverseBlank(floorplan->softTesserae[0]->TileArr[0], blankRecord);
  
        // initialize all TessInfo
        for (MFLTessInfo& tessInfo: mAllOverlaps){
            tessInfo.initMembers(mLF, this);
        }

        for (MFLTessInfo& tessInfo: mAllBlocks){
            tessInfo.initMembers(mLF, this);
        }

        for (MFLTessInfo& tessInfo: mAllBlanks){
            tessInfo.initMembers(mLF, this);
        }

        // construct Max flow graph
        mMaxflowManager.addNode(SUPERSOURCE, NodeType::SOURCE);
        mMaxflowManager.addNode(SUPERSINK, NodeType::SINK);

        // add nodes
        for (MFLTessInfo& tessInfo: mAllOverlaps){
            mMaxflowManager.addNode(tessInfo.tessName, NodeType::NODE);
        }

        for (MFLTessInfo& tessInfo: mAllBlocks){
            mMaxflowManager.addNode(tessInfo.tessName, NodeType::NODE);
        }

        for (MFLTessInfo& tessInfo: mAllBlanks){
            mMaxflowManager.addNode(tessInfo.tessName, NodeType::NODE);
        }
        
        
        // add edges from supersource to overlaps
        for (MFLTessInfo& tessInfo: mAllOverlaps){
            mMaxflowManager.addEdge(SUPERSOURCE, tessInfo.tessName, tessInfo.area);
        }

        // add edges from overlaps to neighboring SOFT tiles 
        // if overlap tile is the overlap of tessera A and tessera B, then only edges to 
        // tessera A or tessera B will be added
        for (MFLTessInfo& overlapInfo: mAllOverlaps){
            for (int overlapIndex: overlapInfo.overlaps){
                // is soft blcok
                if (overlapIndex >= mFixedTessNum){
                    mMaxflowManager.addEdge(overlapInfo.tessName, mAllBlocks[overlapIndex].tessName, mMaxflowInf);
                }
            }
        }

        // add edges from tiles to blanks 
        for (MFLTessInfo& tessInfo: mAllBlocks){
            if (tessInfo.index < mFixedTessNum){
                // don't add connections from fixed blocks
                continue;
            }
            for (Tile* neighbor: tessInfo.allNeighborTiles){
                if (neighbor->getType() == tileType::BLANK){
                    mMaxflowManager.addEdge(tessInfo.tessName, std::to_string((intptr_t)neighbor), mMaxflowInf);
                }
            }
        }

        // add edges from tiles to other soft tiles
        for (MFLTessInfo& tessInfo: mAllBlocks){
            if (tessInfo.index < mFixedTessNum){
                // don't add connections from fixed blocks
                continue;
            }
            for (int tessNeighbor: tessInfo.tessNeighbors){
                MFLTessInfo& neighborInfo = mAllBlocks[tessNeighbor]; 
                if (neighborInfo.type == MFLTessType::SOFT) {
                    mMaxflowManager.addEdge(tessInfo.tessName, neighborInfo.tessName, mMaxflowInf);
                }
            }
        }

        // add edges from blanks to supersinks
        for (MFLTessInfo& blankInfo: mAllBlanks){
            mMaxflowManager.addEdge(blankInfo.tessName, SUPERSINK, blankInfo.area);
        }
    }

    void MaxflowLegaliser::addOverlapInfo(Tile* tile, MFLTessInfo& tess){
        std::vector<int> allOverlaps;
        for (int fixedIndex: tile->OverlapFixedTesseraeIdx){
            allOverlaps.push_back(fixedIndex);
        }
        
        for (int softIndex: tile->OverlapSoftTesseraeIdx){
            allOverlaps.push_back(softIndex + mFixedTessNum);
        }

        for (int i = 0; i < allOverlaps.size(); i++){
            int tessIndex1 = allOverlaps[i];
            tess.overlaps.insert(tessIndex1);

            for (int j = i+1; j< allOverlaps.size(); j++){
                int tessIndex2 = allOverlaps[j];
                addSingleOverlapInfo(tile, tessIndex1, tessIndex2);
            }
        }
    }

    void MaxflowLegaliser::addSingleOverlapInfo(Tile* tile, int overlapIdx1, int overlapIdx2){
        bool found = false;
        for (MFLTessInfo tess: mAllOverlaps){
            if (tess.overlaps.count(overlapIdx1) == 1 && tess.overlaps.count(overlapIdx2) == 1){
                tess.tileList.push_back(tile);
                found = true;
                break;
            }
        }
        if (!found){
            MFLTessInfo newTess;
            newTess.tessName = toOverlapName(overlapIdx1, overlapIdx2); 
            newTess.tileList.push_back(tile);
            newTess.type = MFLTessType::OVERLAP;
            newTess.index = mAllOverlaps.size();
            newTess.overlaps.insert(overlapIdx1);
            newTess.overlaps.insert(overlapIdx2);
            mAllOverlaps.push_back(newTess);
        }
    }

    void MaxflowLegaliser::MFLTraverseBlank(Tile* tile, std::vector <Cord> &record){
        record.push_back(tile->getLowerLeft());
        
        if(tile->getType() == tileType::BLANK){
            MFLTessInfo newBlankInfo;
            newBlankInfo.type = MFLTessType::BLANK;
            newBlankInfo.tessName = std::to_string((intptr_t)tile);
            newBlankInfo.tileList.push_back(tile);
            newBlankInfo.index = mAllBlanks.size();
            mAllBlanks.push_back(newBlankInfo);
        }
        //TODO: finish rewirte function
        if(tile->rt != nullptr){        
            if(!checkVectorInclude(record, tile->rt->getLowerLeft())){
                MFLTraverseBlank(tile->rt, record);
            }
        }

        if(tile->lb != nullptr){
            if(!checkVectorInclude(record, tile->lb->getLowerLeft())){
                MFLTraverseBlank(tile->lb, record);
            }
        }

        if(tile->bl != nullptr){
            if(!checkVectorInclude(record, tile->bl->getLowerLeft())){
                MFLTraverseBlank(tile->bl, record);
            }
        }

        if(tile->tr != nullptr){
            if(!checkVectorInclude(record, tile->tr->getLowerLeft())){
                MFLTraverseBlank(tile->tr, record);
            }
        }
        
        return;
    }

    void MaxflowLegaliser::legaliseByMaxflow(){
        mMaxflowManager.solve();

        // check if all overflows are all resolved
        // ie. supersource->overlap flow amnt == overlap area
        for (MFLTessInfo& tessInfo: mAllOverlaps){
            if (mMaxflowManager.edgeFlow(SUPERSOURCE, tessInfo.tessName) != tessInfo.area){
                std::stringstream ss;
                ss.str(tessInfo.tessName);
                std::string overlapTile1, overlapTile2, placeholder;
                ss >> placeholder >> overlapTile1 >> overlapTile2;
                std::cout << "[MFL] WARNING: overlap not resolved between " << overlapTile1 << " and " << overlapTile2 << " \n";
                std::cout << "Overlap area: " << tessInfo.area << " Flow Amnt: " << mMaxflowManager.edgeFlow(SUPERSOURCE, tessInfo.tessName) << '\n';
            }
        }
    }


    void MaxflowLegaliser::outputFlows(std::vector<MFLFlowTessInfo*>& overlapTessFlows, 
                                        std::vector<MFLFlowTessInfo*>& blockTessFlows,
                                        std::vector<MFLFlowTessInfo*>& blankTessFlows)
    {
        // contruct all vectors
        // overlaps 
        for (MFLTessInfo& tessInfo: mAllOverlaps){
            MFLFlowTessInfo* overlapTileFlowInfo = new MFLFlowTessInfo;
            overlapTileFlowInfo->tileList = tessInfo.tileList;
            overlapTileFlowInfo->type = MFLTessType::OVERLAP;
            for (int tessIdx: tessInfo.overlaps){
                if (tessIdx < mFixedTessNum){
                    overlapTileFlowInfo->fixedOverlaps.push_back(tessIdx);
                }
                else {
                    overlapTileFlowInfo->softOverlaps.push_back(tessIdx - mFixedTessNum);
                }
            }
            overlapTessFlows.push_back(overlapTileFlowInfo);
        }

        // blocks
        for (MFLTessInfo& tessInfo: mAllBlocks){
            MFLFlowTessInfo* blockTileFlowInfo = new MFLFlowTessInfo;
            blockTileFlowInfo->tileList = tessInfo.tileList;
            if (tessInfo.index < mFixedTessNum){
                blockTileFlowInfo->type = MFLTessType::FIXED;
                blockTileFlowInfo->tessIndex = tessInfo.index;
            }
            else {
                blockTileFlowInfo->type = MFLTessType::SOFT;
                blockTileFlowInfo->tessIndex = tessInfo.index - mFixedTessNum;
            }
            blockTessFlows.push_back(blockTileFlowInfo);
        }
        
        // blanks
        for (MFLTessInfo& tessInfo: mAllBlanks){
            MFLFlowTessInfo* blankTileFlowInfo = new MFLFlowTessInfo;
            blankTileFlowInfo->tileList = tessInfo.tileList;
            blankTileFlowInfo->type = MFLTessType::BLANK;
            blankTessFlows.push_back(blankTileFlowInfo);
        }

        // find all flows

        // add edges from overlaps to other tiles
        for (int i = 0; i < overlapTessFlows.size(); i++){
            MFLTessInfo& tessInfo = mAllOverlaps[i];
            MFLFlowTessInfo* sourceFlowInfo = overlapTessFlows[i];
            for (int j = 0; j < overlapTessFlows.size(); j++){
                MFLTessInfo& overlapInfo = mAllOverlaps[j];
                MFLFlowTessInfo* destFlowInfo = overlapTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, overlapInfo.tessName);
                if (maxflowAmnt > 0){
                    // should not happen
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }

            for (int j = 0; j < blockTessFlows.size(); j++){
                MFLTessInfo& blockInfo = mAllBlocks[j];
                MFLFlowTessInfo* destFlowInfo = blockTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, blockInfo.tessName);
                if (maxflowAmnt > 0){
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }
            
            for (int j = 0; j < blankTessFlows.size(); j++){
                MFLTessInfo& blankInfo = mAllBlanks[j];
                MFLFlowTessInfo* destFlowInfo = blankTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, blankInfo.tessName);
                if (maxflowAmnt > 0){
                    // should not happen
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }
        }


        // add edges from blocks to other tiles
        for (int i = 0; i < blockTessFlows.size(); i++){
            MFLTessInfo& tessInfo = mAllBlocks[i];
            MFLFlowTessInfo* sourceFlowInfo = blockTessFlows[i];

            for (int j = 0; j < overlapTessFlows.size(); j++){
                MFLTessInfo& overlapInfo = mAllOverlaps[j];
                MFLFlowTessInfo* destFlowInfo = overlapTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, overlapInfo.tessName);
                if (maxflowAmnt > 0){
                    // should not happen
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }

            for (int j = 0; j < blockTessFlows.size(); j++){
                MFLTessInfo& blockInfo = mAllBlocks[j];
                MFLFlowTessInfo* destFlowInfo = blockTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, blockInfo.tessName);
                if (maxflowAmnt > 0){
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }
            
            for (int j = 0; j < blankTessFlows.size(); j++){
                MFLTessInfo& blankInfo = mAllBlanks[j];
                MFLFlowTessInfo* destFlowInfo = blankTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, blankInfo.tessName);
                if (maxflowAmnt > 0){
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }
        }

        // add edges from blanks to other tiles
        for (int i = 0; i < blankTessFlows.size(); i++){
            MFLTessInfo& tessInfo = mAllBlanks[i];
            MFLFlowTessInfo* sourceFlowInfo = blankTessFlows[i];
            for (int j = 0; j < overlapTessFlows.size(); j++){
                MFLTessInfo& overlapInfo = mAllOverlaps[j];
                MFLFlowTessInfo* destFlowInfo = overlapTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, overlapInfo.tessName);
                if (maxflowAmnt > 0){
                    // should not happen
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }

            for (int j = 0; j < blockTessFlows.size(); j++){
                MFLTessInfo& blockInfo = mAllBlocks[j];
                MFLFlowTessInfo* destFlowInfo = blockTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, blockInfo.tessName);
                if (maxflowAmnt > 0){
                    // should not happen
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }
            
            for (int j = 0; j < blankTessFlows.size(); j++){
                MFLTessInfo& blankInfo = mAllBlanks[j];
                MFLFlowTessInfo* destFlowInfo = blankTessFlows[j];
                int maxflowAmnt = mMaxflowManager.edgeFlow(tessInfo.tessName, blankInfo.tessName);
                if (maxflowAmnt > 0){
                    // should not happen
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, sourceFlowInfo, destFlowInfo, maxflowAmnt);
                    sourceFlowInfo->fromFlows.push_back(flow);
                }
            }
        }

    }   

    void MaxflowLegaliser::makeSingleFlow(MFLSingleFlowInfo& flow, MFLFlowTessInfo* source, MFLFlowTessInfo* dest, int flowAmt){
        flow.sourceTile = source;
        flow.destTile = dest;
        flow.flowAmount = flowAmt;
    }

    std::string toOverlapName(int tessIndex1, int tessIndex2){
        return "OVERLAP " + std::to_string(tessIndex1) + " " + std::to_string(tessIndex2);
    }
}