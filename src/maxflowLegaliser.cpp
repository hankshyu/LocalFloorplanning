#include "maxflowLegaliser.h"
#include "maxflow.h"
#include "Tile.h"
#include "LFLegaliser.h"
#include <vector>
#include <utility>
#include <cstdint>

namespace MFL {
    MaxflowLegaliser::MaxflowLegaliser(/* args */)
    {
    }

    MaxflowLegaliser::~MaxflowLegaliser()
    {
    }


    void MaxflowLegaliser::initMFL(LFLegaliser* floorplan){
        // find all overlaps and blocks

        mLF = floorplan;
        mMaxflowInf = floorplan->getCanvasHeight() * floorplan->getCanvasWidth();

        for(int t = 0; t < floorplan->fixedTesserae.size(); t++){
            Tessera* tess = floorplan->fixedTesserae[t];

            for(Tile* tile : tess->TileArr){
                addTileInfo(tile);
                mTilePtr2FixedTessIdx.insert(std::pair<Tile*,int>(tile, t));
            }

            for(Tile* overlap : tess->OverlapArr){
                // for overlap tiles, only push when it's never met
                if (!checkOverlapDuplicate(overlap)){
                    addTileInfo(overlap);
                }
            }
        }

        for(int t = 0; t < floorplan->softTesserae.size(); t++){
            Tessera* tess = floorplan->softTesserae[t];

            for(Tile* tile : tess->TileArr){
                addTileInfo(tile);
                mTilePtr2SoftTessIdx.insert(std::pair<Tile*,int>(tile, t));
            }

            for(Tile* overlap : tess->OverlapArr){
                if (!checkOverlapDuplicate(overlap)){
                    addTileInfo(overlap);
                }
            }
        }

        // find all blanks
        std::vector <Cord> blankRecord;
        MFLTraverseBlank(floorplan->softTesserae[0]->TileArr[0], blankRecord);

        // construct Max flow graph
        mMaxflowManager.addNode(SUPERSOURCE, NodeType::SOURCE);
        mMaxflowManager.addNode(SUPERSINK, NodeType::SINK);

        // add nodes
        for (MFLTileInfo tileInfo: mAllOverlaps){
            mMaxflowManager.addNode(tileInfo.nodeName, NodeType::NODE);
        }

        for (MFLTileInfo tileInfo: mAllBlocks){
            mMaxflowManager.addNode(tileInfo.nodeName, NodeType::NODE);
        }

        for (MFLTileInfo tileInfo: mAllBlanks){
            mMaxflowManager.addNode(tileInfo.nodeName, NodeType::NODE);
        }
        
        // add edges from supersource to overlaps
        for (MFLTileInfo tileInfo: mAllOverlaps){
            mMaxflowManager.addEdge(SUPERSOURCE, tileInfo.nodeName, tileInfo.tile->getArea());
        }

        // add edges from overlaps to neighboring SOFT tiles 
        // if overlap tile is the overlap of tessera A and tessera B, then only edges to 
        // neighboring tiles belonging to tessera A or tessera B will be added
        for (MFLTileInfo& overlapInfo: mAllOverlaps){
            for (Tile* neighbor: overlapInfo.allNeighbors){
                if (neighbor->getType() == tileType::BLOCK){
                    bool overlapsTessera = false;
                    // if (mTilePtr2FixedTessIdx.count(neighbor) == 1){
                    //     // neighbor belongs to a fixed tessera
                    //     // check if overlap covers this tessera
                    //     int neighborFixedIndex = mTilePtr2FixedTessIdx[neighbor];
                    //     for (int fixedIndex: overlapInfo.tile->OverlapFixedTesseraeIdx){
                    //         if (fixedIndex == neighborFixedIndex){
                    //             overlapsTessera = true;
                    //             break;
                    //         }
                    //     }
                    // }
                    // else 
                    if (mTilePtr2SoftTessIdx.count(neighbor) == 1){
                        // neighbor belongs to a soft tessera
                        // check if overlap covers this tessera
                        int neighborSoftIndex = mTilePtr2SoftTessIdx[neighbor];
                        for (int softIndex: overlapInfo.tile->OverlapSoftTesseraeIdx){
                            if (softIndex == neighborSoftIndex){
                                overlapsTessera = true;
                                break;
                            }
                        }
                    }
                    
                    if (overlapsTessera){
                        mMaxflowManager.addEdge(overlapInfo.nodeName, std::to_string((intptr_t)neighbor), mMaxflowInf);
                        overlapInfo.validNeighbors.push_back(neighbor);
                    }
                }
            }
        }

        // add edges from tiles to blanks
        for (MFLTileInfo& tileInfo: mAllBlocks){
            if (mTilePtr2FixedTessIdx.count(tileInfo.tile) == 1){
                // don't add connections from fixed blocks to blanks
                continue;
            }
            for (Tile* neighbor: tileInfo.allNeighbors){
                if (neighbor->getType() == tileType::BLANK){
                    mMaxflowManager.addEdge(tileInfo.nodeName, std::to_string((intptr_t)neighbor), mMaxflowInf);
                    tileInfo.validNeighbors.push_back(neighbor);
                }
            }
        }

        // add edges from blanks to supersinks
        for (MFLTileInfo& blankInfo: mAllBlanks){
            mMaxflowManager.addEdge(blankInfo.nodeName, SUPERSINK, blankInfo.tile->getArea());
        }
    }

    void MaxflowLegaliser::addTileInfo(Tile* tile){
        MFLTileInfo tileInfo;
        tileInfo.tile = tile;
        mLF->findAllNeighbors(tile, tileInfo.allNeighbors);
        tileInfo.nodeName = std::to_string((intptr_t)tile);

        if (tile->getType() == tileType::OVERLAP){
            mAllOverlaps.push_back(tileInfo);
        }
        else if (tile->getType() == tileType::BLOCK){
            mAllBlocks.push_back(tileInfo);
        }
        else {
            mAllBlanks.push_back(tileInfo);
        }
    }

    void MaxflowLegaliser::MFLTraverseBlank(Tile* tile, std::vector <Cord> &record){
        record.push_back(tile->getLowerLeft());
        
        if(tile->getType() == tileType::BLANK){
            addTileInfo(tile);
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

    bool MaxflowLegaliser::checkOverlapDuplicate(Tile* overlap){
        for (MFLTileInfo tileInfo: mAllOverlaps){
            if (tileInfo.tile == overlap){
                return true;
            }
        }
        return false;
    }

    void MaxflowLegaliser::legaliseByMaxflow(){
        mMaxflowManager.solve();

        // check if all overflows are all resolved
        // ie. supersource->overlap flow amnt == overlap area
        for (MFLTileInfo& tileInfo: mAllOverlaps){
            if (mMaxflowManager.edgeFlow(SUPERSOURCE, tileInfo.nodeName) != tileInfo.tile->getArea()){
                std::string overlapTile1 = "\n\n"; // placeholder string
                std::string overlapTile2 = "\n\n";
                for (int fixedIndex: tileInfo.tile->OverlapFixedTesseraeIdx){
                    if (overlapTile1 == "\n\n"){
                        overlapTile1 = mLF->fixedTesserae[fixedIndex]->getName();
                    }
                    else if (overlapTile2 == "\n\n"){
                        overlapTile2 = mLF->fixedTesserae[fixedIndex]->getName();
                    }
                }
                for (int softIndex: tileInfo.tile->OverlapSoftTesseraeIdx){
                    if (overlapTile1 == "\n\n"){
                        overlapTile1 = mLF->softTesserae[softIndex]->getName();
                    }
                    else if (overlapTile2 == "\n\n"){
                        overlapTile2 = mLF->softTesserae[softIndex]->getName();
                    }
                }
                std::cout << "[MFL] ERROR: overlap not resolved between " << overlapTile1 << " and " << overlapTile2 << " \n";
                std::cout << "Overlap area: " << tileInfo.tile->getArea() << " Flow Amnt: " << mMaxflowManager.edgeFlow(SUPERSOURCE, tileInfo.nodeName) << '\n';
            }
        }
    }

    void MaxflowLegaliser::outputFlows(std::vector<MFLTileFlowInfo>& overlapTileFlows, 
                                        std::vector<MFLTileFlowInfo>& blockTileFlows,
                                        std::vector<MFLTileFlowInfo>& blankTileFlows)
    {
        // overlaps 
        for (MFLTileInfo tileInfo: mAllOverlaps){
            MFLTileFlowInfo overlapTileFlowInfo; 
            overlapTileFlowInfo.tile = tileInfo.tile;
            for (Tile* validNeighbor: tileInfo.validNeighbors){
                int maxflowAmnt = mMaxflowManager.edgeFlow(tileInfo.nodeName, std::to_string((intptr_t)validNeighbor));
                if (maxflowAmnt > 0){
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, tileInfo.tile, validNeighbor, maxflowAmnt);
                    overlapTileFlowInfo.fromFlows.push_back(flow);
                }
            }
            overlapTileFlows.push_back(overlapTileFlowInfo);
        }

        // blocks
        for (MFLTileInfo tileInfo: mAllBlocks){
            MFLTileFlowInfo blockTileFlowInfo; 
            blockTileFlowInfo.tile = tileInfo.tile;
            for (Tile* validNeighbor: tileInfo.validNeighbors){
                int maxflowAmnt = mMaxflowManager.edgeFlow(tileInfo.nodeName, std::to_string((intptr_t)validNeighbor));
                if (maxflowAmnt > 0){
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, tileInfo.tile, validNeighbor, maxflowAmnt);
                    blockTileFlowInfo.fromFlows.push_back(flow);
                }
            }
            blockTileFlows.push_back(blockTileFlowInfo);
        }

        // blanks
        for (MFLTileInfo tileInfo: mAllBlanks){
            MFLTileFlowInfo blankTileFlowInfo; 
            blankTileFlowInfo.tile = tileInfo.tile;
            for (Tile* validNeighbor: tileInfo.validNeighbors){
                int maxflowAmnt = mMaxflowManager.edgeFlow(tileInfo.nodeName, std::to_string((intptr_t)validNeighbor));
                if (maxflowAmnt > 0){
                    MFLSingleFlowInfo flow;
                    makeSingleFlow(flow, tileInfo.tile, validNeighbor, maxflowAmnt);
                    blankTileFlowInfo.fromFlows.push_back(flow);
                }
            }
            blankTileFlows.push_back(blankTileFlowInfo);
        }
    }   

    void MaxflowLegaliser::makeSingleFlow(MFLSingleFlowInfo& flow, Tile* source, Tile* dest, int flowAmt){
        flow.sourceTile = source;
        flow.destTile = dest;
        flow.flowAmount = flowAmt;

        std::vector<Tile*> neighbors;
        mLF->findTopNeighbors(source,neighbors);
        for (Tile* topNeighbor: neighbors){
            if (topNeighbor == dest){
                flow.direction = DIRECTION::TOP;
                return;
            }
        }
        neighbors.clear();

        mLF->findRightNeighbors(source,neighbors);
        for (Tile* rightNeighbor: neighbors){
            if (rightNeighbor == dest){
                flow.direction = DIRECTION::RIGHT;
                return;
            }
        }
        neighbors.clear();

        mLF->findDownNeighbors(source,neighbors);
        for (Tile* downNeighbor: neighbors){
            if (downNeighbor == dest){
                flow.direction = DIRECTION::DOWN;
                return;
            }
        }
        neighbors.clear();

        mLF->findLeftNeighbors(source,neighbors);
        for (Tile* leftNeighbor: neighbors){
            if (leftNeighbor == dest){
                flow.direction = DIRECTION::LEFT;
                return;
            }
        }

        std::cout << "[MFL] ERROR: MakeSingleFlow neighbor not found\n";
        return;
    }
}