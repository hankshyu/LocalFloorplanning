#include "DFSLegalizer.h"
#include "maxflow.h"
#include "Tile.h"
#include "LFLegaliser.h"
#include <vector>
#include <utility>
#include <cstdint>
#include <numeric>

namespace DFSL {
    DFSLNode::DFSLNode(): area(0), index(0) {}

    DFSLegalizer::DFSLegalizer()
    {
    }

    DFSLegalizer::~DFSLegalizer()
    {
    }

    void DFSLegalizer::initDFSLegalizer(LFLegaliser* floorplan){
        mLF = floorplan;
        mTransientOverlapArea.clear();
        constructGraph();
        setDefaultConfig();
    }
    
    void DFSLegalizer::addBlockNode(Tessera* tess){
        DFSLNode newNode;
        newNode.nodeName = tess->getName();
        newNode.nodeType = MFLTessType::FIXED;
        newNode.index = mAllNodes.size();
        for(Tile* tile : tess->TileArr){
            newNode.tileList.push_back(tile); 
            newNode.area += tile->getArea();
            mTilePtr2NodeIndex.insert(std::pair<Tile*,int>(tile, mAllNodes.size()));
        }
        mAllNodes.push_back(newNode);
    }

    void DFSLegalizer::constructGraph(){
        mAllNodes.clear();
        mFixedTessNum = mLF->fixedTesserae.size();
        mSoftTessNum = mLF->softTesserae.size();

        // find fixed and soft tess
        for(int t = 0; t < mLF->fixedTesserae.size(); t++){
            Tessera* tess = mLF->fixedTesserae[t];
            addBlockNode(tess);
        }

        for(int t = 0; t < mLF->softTesserae.size(); t++){
            Tessera* tess = mLF->softTesserae[t];
            addBlockNode(tess);
        }

        // find overlaps 
        for(int t = 0; t < mLF->fixedTesserae.size(); t++){
            Tessera* tess = mLF->fixedTesserae[t];
            for(Tile* overlap : tess->OverlapArr){
                addOverlapInfo(overlap);
            }
        }

        for(int t = 0; t < mLF->softTesserae.size(); t++){
            Tessera* tess = mLF->softTesserae[t];
            for(Tile* overlap : tess->OverlapArr){
                addOverlapInfo(overlap);
            }
        }
        mOverlapNum = mAllNodes.size() - mFixedTessNum - mSoftTessNum;

        // find all blanks
        std::vector <Cord> blankRecord;
        DFSLTraverseBlank(mLF->softTesserae[0]->TileArr[0], blankRecord);
        mBlankNum = mAllNodes.size() - mFixedTessNum - mSoftTessNum - mOverlapNum;


        // find neighbors
        // overlap and block
        int overlapStartIndex = mFixedTessNum + mSoftTessNum;
        int overlapEndIndex = overlapStartIndex + mOverlapNum;
        for (int from = overlapStartIndex; from < overlapEndIndex; from++){
            DFSLNode& overlap = mAllNodes[from];
            for (int to: overlap.overlaps){
                if (to >= mFixedTessNum){
                    findEdge(from, to);
                }
            }
        }

        // block and block, block and whitespace
        int softStartIndex = mFixedTessNum;
        int softEndIndex = softStartIndex + mSoftTessNum;
        int blankStartIndex = mFixedTessNum + mSoftTessNum + mOverlapNum;
        int blankEndIndex = blankStartIndex + mBlankNum;
        for (int from = softStartIndex; from < softEndIndex; from++){
            std::set<int> allNeighbors;
            getTessNeighbors(from, allNeighbors);
            for (int to: allNeighbors){
                if (softStartIndex <= to && to < softEndIndex){
                    findEdge(from, to);
                }
                else if (blankStartIndex <= to && to < blankEndIndex){
                    findEdge(from, to);
                }
            }
        }
        
        // todo: finish transient overlap area thing

    }

    void DFSLegalizer::addOverlapInfo(Tile* tile){
        std::vector<int> allOverlaps;
        for (int fixedIndex: tile->OverlapFixedTesseraeIdx){
            allOverlaps.push_back(fixedIndex);
        }
        
        for (int softIndex: tile->OverlapSoftTesseraeIdx){
            allOverlaps.push_back(softIndex + mFixedTessNum);
        }

        for (int i = 0; i < allOverlaps.size(); i++){
            int tessIndex1 = allOverlaps[i];

            for (int j = i+1; j < allOverlaps.size(); j++){
                int tessIndex2 = allOverlaps[j];
                addSingleOverlapInfo(tile, tessIndex1, tessIndex2);
            }
        }
    }

    void DFSLegalizer::addSingleOverlapInfo(Tile* tile, int overlapIdx1, int overlapIdx2){
        bool found = false;
        for (int i = mFixedTessNum + mSoftTessNum; i < mAllNodes.size(); i++){
            DFSLNode& tess = mAllNodes[i];
            if (tess.overlaps.count(overlapIdx1) == 1 && tess.overlaps.count(overlapIdx2) == 1){
                for (Tile* existingTile: tess.tileList){
                    if (existingTile == tile){
                        found = true;
                        break;
                    }
                }
                if (!found){
                    tess.tileList.push_back(tile);
                    tess.area += tile->getArea();
                    mTilePtr2NodeIndex.insert(std::pair<Tile*,int>(tile, i));
                    found = true;
                }
                break;
            }
        }
        if (!found){
            DFSLNode newNode;
            newNode.area += tile->getArea();
            newNode.tileList.push_back(tile);
            newNode.overlaps.insert(overlapIdx1);
            newNode.overlaps.insert(overlapIdx2);
            newNode.nodeName = toOverlapName(overlapIdx1, overlapIdx2); 
            newNode.nodeType = MFLTessType::OVERLAP;
            newNode.index = mAllNodes.size();
            mTilePtr2NodeIndex.insert(std::pair<Tile*,int>(tile, mAllNodes.size()));
            mAllNodes.push_back(newNode);
        }
    }

    std::string DFSLegalizer::toOverlapName(int tessIndex1, int tessIndex2){
        std::string name1 = mAllNodes[tessIndex1].nodeName;
        std::string name2 = mAllNodes[tessIndex2].nodeName;
        return "OVERLAP " + name1 + " " + name2;
    }

    void DFSLegalizer::DFSLTraverseBlank(Tile* tile, std::vector <Cord> &record){
        record.push_back(tile->getLowerLeft());
        
        if(tile->getType() == tileType::BLANK){
            DFSLNode newNode;
            newNode.tileList.push_back(tile);
            newNode.nodeName = std::to_string((intptr_t)tile);
            newNode.nodeType = MFLTessType::BLANK;
            newNode.index = mAllNodes.size();
            newNode.area += tile->getArea();
            mTilePtr2NodeIndex.insert(std::pair<Tile*,int>(tile, mAllNodes.size()));
            mAllNodes.push_back(newNode);
        }
        //TODO: finish rewirte function
        if(tile->rt != nullptr){        
            if(!checkVectorInclude(record, tile->rt->getLowerLeft())){
                DFSLTraverseBlank(tile->rt, record);
            }
        }

        if(tile->lb != nullptr){
            if(!checkVectorInclude(record, tile->lb->getLowerLeft())){
                DFSLTraverseBlank(tile->lb, record);
            }
        }

        if(tile->bl != nullptr){
            if(!checkVectorInclude(record, tile->bl->getLowerLeft())){
                DFSLTraverseBlank(tile->bl, record);
            }
        }

        if(tile->tr != nullptr){
            if(!checkVectorInclude(record, tile->tr->getLowerLeft())){
                DFSLTraverseBlank(tile->tr, record);
            }
        }
        
        return;
    }

    void DFSLegalizer::findEdge(int fromIndex, int toIndex){
        DFSLNode& fromNode = mAllNodes[fromIndex];
        DFSLNode& toNode = mAllNodes[toIndex];

        int bestLength = 0;
        int bestDirection = 0;
        std::vector<Segment> bestTangent;
        for (int dir = 0; dir < 4; dir++){
            // find Top, right, bottom, left neighbros
            std::vector<Segment> currentSegment;
            for (Tile* tile: fromNode.tileList){
                std::vector<Tile*> neighbors;
                switch (dir) {
                case 0:
                    mLF->findTopNeighbors(tile, neighbors);
                    break;
                case 1:
                    mLF->findRightNeighbors(tile, neighbors);
                    break;
                case 2:
                    mLF->findDownNeighbors(tile, neighbors);
                    break;
                case 3:
                    mLF->findLeftNeighbors(tile, neighbors);
                    break;
                }
                
                for (Tile* neighbor: neighbors){
                    auto itlow = mTilePtr2NodeIndex.lower_bound(neighbor);
                    auto itup = mTilePtr2NodeIndex.upper_bound(neighbor);
                    for (auto it = itlow; it != itup; ++it){
                        int nodeIndex = it->second;
                        if (nodeIndex == toIndex){
                            // find tangent
                            Segment tangent;
                            Cord fromStart, fromEnd, toStart, toEnd;
                            if (dir == 0){
                                // neighbor on top of fromNode 
                                fromStart = tile->getUpperLeft();
                                fromEnd = tile->getUpperRight();
                                toStart = neighbor->getLowerLeft(); 
                                toEnd = neighbor->getLowerRight();
                            }
                            else if (dir == 1){
                                // neighbor is right of fromNode 
                                fromStart = tile->getLowerRight();
                                fromEnd = tile->getUpperRight();
                                toStart = neighbor->getLowerLeft(); 
                                toEnd = neighbor->getUpperLeft();
                            }
                            else if (dir == 2){
                                // neighbor is bottom of fromNode 
                                fromStart = tile->getLowerLeft();
                                fromEnd = tile->getLowerRight();
                                toStart = neighbor->getUpperLeft(); 
                                toEnd = neighbor->getUpperRight();
                            }
                            else {
                                // neighbor is left of fromNode 
                                fromStart = tile->getLowerLeft();
                                fromEnd = tile->getUpperLeft();
                                toStart = neighbor->getLowerRight(); 
                                toEnd = neighbor->getUpperRight();
                            }
                            tangent.segStart = fromStart <= toStart ? toStart : fromStart;
                            tangent.segEnd = fromEnd <= toEnd ? fromEnd : toEnd;
                            currentSegment.push_back(tangent);
                        }
                    }
                }
            }
            
            // check segment, splice segments together to one large segment
            int segLength = 0;
            std::vector<Segment> splicedSegments;
            if (currentSegment.size() > 0){
                std::sort(currentSegment.begin(), currentSegment.end(), compareSegment);
                Cord segBegin = currentSegment.front().segStart;
                for (int j = 1; j < currentSegment.size(); j++){
                    if (currentSegment[j].segStart != currentSegment[j-1].segEnd){
                        Cord segEnd = currentSegment[j-1].segEnd;
                        Segment splicedSegment;
                        splicedSegment.segStart = segBegin;
                        splicedSegment.segEnd = segEnd;
                        splicedSegments.push_back(splicedSegment);
                        segLength += (segEnd.x - segBegin.x) + (segEnd.y - segBegin.y);  

                        segBegin = currentSegment[j].segStart;
                    }
                }
                Cord segEnd = currentSegment.back().segEnd;
                Segment splicedSegment;
                splicedSegment.segStart = segBegin;
                splicedSegment.segEnd = segEnd;
                splicedSegments.push_back(splicedSegment);
                segLength += (segEnd.x - segBegin.x) + (segEnd.y - segBegin.y);  
            }

            if (segLength > bestLength){
                bestDirection = dir;
                bestTangent = splicedSegments;
                bestLength = segLength;
            }
        }

        // construct edge in graph
        DFSLEdge newEdge;
        newEdge.commonEdge = bestTangent[0];
        if (bestDirection == 0){
            newEdge.direction = DIRECTION::TOP;
        } 
        else if (bestDirection == 1){
            newEdge.direction = DIRECTION::RIGHT;
        }
        else if (bestDirection == 2){
            newEdge.direction = DIRECTION::DOWN;
        }
        else {
            newEdge.direction = DIRECTION::LEFT;
        }
        newEdge.fromIndex = fromIndex;
        newEdge.toIndex = toIndex; 
        mAllNodes[fromIndex].edgeList.push_back(newEdge);
    }

    void DFSLegalizer::getTessNeighbors(int nodeId, std::set<int> allNeighbors){
        DFSLNode& node = mAllNodes[nodeId];
        std::vector<Tile*> allNeighborTiles;
        for (Tile* tile : node.tileList){
            mLF->findAllNeighbors(tile, allNeighborTiles);
        }

        for (Tile* tile : allNeighborTiles){
            auto itlow = mTilePtr2NodeIndex.lower_bound(tile);
            auto itup = mTilePtr2NodeIndex.upper_bound(tile);
            for (auto it = itlow; it != itup; ++it){
                int nodeIndex = it->second;
                if (nodeIndex != nodeId){
                    allNeighbors.insert(nodeIndex);
                }
            }
        }
    }

    RESULT DFSLegalizer::legalize(){
        // todo: create backup (deep copy)
        RESULT result;

        while (1) {
            int overlapStart = mFixedTessNum + mSoftTessNum;
            int overlapEnd = overlapStart + mOverlapNum;

            if (overlapStart == overlapEnd){
                break;
            }

            bool overlapResolved = false;
            std::vector<bool> solveable(mAllNodes.size(), true);
            
            while (!overlapResolved){
                int maxOverlapArea = 0;
                int maxOverlapIndex = -1;
                for (int i = overlapStart; i < overlapEnd; i++){
                    DFSLNode& currentOverlap = mAllNodes[i];
                    if (currentOverlap.area > maxOverlapArea && solveable[i]){
                        maxOverlapArea = currentOverlap.area;
                        maxOverlapIndex = i;
                    }
                }
                if (maxOverlapIndex == -1){
                    std::cout << "[DFSL] WARNING: Overlap not resolved\n";
                    result = RESULT::OVERLAP_NOT_RESOLVED;
                    return result;
                }
                else {
                    solveable[maxOverlapIndex] = overlapResolved = migrateOverlap(maxOverlapIndex);
                }
            }

            constructGraph();
        }

        // test each tess for legality
        result = RESULT::SUCCESS;
        int nodeStart = 0;
        int nodeEnd = mFixedTessNum + mSoftTessNum;
        for (int i = nodeStart; i < nodeEnd; i++){
            DFSLNode& node = mAllNodes[i];
            LegalInfo legal = getNodeLegalInfo(i);
            if (legal.util < UTIL_RULE){
                // todo: finish warning messages
                std::cout << "[DFSL] Warning: util for \n";
                result = RESULT::CONSTRAINT_FAIL;
            }
            if (legal.aspectRatio > ASPECT_RATIO_RULE || legal.aspectRatio < ASPECT_RATIO_RULE){
                // todo: finish warning messages
                std::cout << "[DFSL] Warning: aspect ratio for \n";
                result = RESULT::CONSTRAINT_FAIL;
            }
        }
        return result;
    }

    bool DFSLegalizer::migrateOverlap(int overlapIndex){
        mBestPath.clear();
        mCurrentPath.clear();
        mBestCost = INT_MAX;

        mCurrentPath.push_back(overlapIndex);
        for (DFSLEdge edge: mAllNodes[overlapIndex].edgeList){
            dfs(edge, 0);
        }

        // transient overlap area

        return true;
    }

    void DFSLegalizer::dfs(DFSLEdge edge, int currentCost){
        int toIndex = edge.toIndex;
        int edgeCost = getEdgeCost(edge);
        currentCost += edgeCost;
        mCurrentPath.push_back(toIndex);
        int blankStart = mFixedTessNum + mSoftTessNum + mOverlapNum;
        int blankEnd = blankStart + mBlankNum;
        if (blankStart <= toIndex && toIndex < blankEnd){
            if (currentCost < mBestCost){
                mBestPath = mCurrentPath;
                mBestCost = currentCost;
            }
        }

        if (currentCost < config.maxCostCutoff){
            for (DFSLEdge edge: mAllNodes[toIndex].edgeList){
                if (!inVector(edge.toIndex, mCurrentPath)){
                    dfs(edge, currentCost);
                }
            }
        }

        mCurrentPath.pop_back();
    }

    int DFSLegalizer::getEdgeCost(DFSLEdge edge){

    }

    inline bool inVector(int a, std::vector<int>& vec){
        for (int b: vec){
            if (b == a) {
                return true;
            }
        }
        return false;
    } 

    void DFSLegalizer::setDefaultConfig(){
        this->config.maxCostCutoff = 100000.0;
        this->config.OBAreaWeight = 1000;
        this->config.OBUtilWeight = 100;
        this->config.BWUtilWeight = 100;
        this->config.BBFlatCost = 10000;
        this->config.WWFlatCost = 10000;
    }

    LegalInfo DFSLegalizer::getNodeLegalInfo(int index){
        LegalInfo legal;
        int min_x, max_x, min_y, max_y;
        min_x = min_y = INT_MAX;
        max_x = max_y = -INT_MAX;
        legal.actualArea = 0;
        for (Tile* tile: mAllNodes[index].tileList){
            if (tile->getLowerLeft().x < min_x){
                min_x = tile->getLowerLeft().x;
            }            
            if (tile->getLowerLeft().y < min_y){
                min_y = tile->getLowerLeft().y;
            }            
            if (tile->getUpperRight().x > max_x){
                max_x = tile->getUpperRight().x;
            }            
            if (tile->getUpperRight().y < max_y){
                max_y = tile->getUpperRight().y;
            }
            legal.actualArea += tile->getArea();   
        }
        legal.width = max_x - min_x;
        legal.height = max_y - min_y;

        legal.bbArea = legal.width * legal.height; 
        legal.BL = Cord(min_x, min_y);  
        legal.aspectRatio = (double) legal.width / (double) legal.height;
        legal.util = (double) legal.actualArea / (double) legal.bbArea;

        return legal;
    } 

    static bool compareSegment(Segment a, Segment b){
        return (a.segStart.x < b.segStart.x || a.segStart.y < b.segStart.y);
    }

}