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
    }
    
    void DFSLegalizer::addBlockNode(Tessera* tess, bool isFixed){
        DFSLNode newNode;
        newNode.nodeName = tess->getName();
        newNode.nodeType = isFixed ? DFSLTessType::FIXED : DFSLTessType::SOFT;
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
        mTilePtr2NodeIndex.clear();
        mFixedTessNum = mLF->fixedTesserae.size();
        mSoftTessNum = mLF->softTesserae.size();

        // find fixed and soft tess
        for(int t = 0; t < mLF->fixedTesserae.size(); t++){
            Tessera* tess = mLF->fixedTesserae[t];
            addBlockNode(tess, true);
        }

        for(int t = 0; t < mLF->softTesserae.size(); t++){
            Tessera* tess = mLF->softTesserae[t];
            addBlockNode(tess, false);
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
        
        for (OverlapArea& tempArea: mTransientOverlapArea){
            for (int from = overlapStartIndex; from < overlapEndIndex; from++){
                DFSLNode& overlap = mAllNodes[from];
                if (overlap.overlaps.count(tempArea.index1) == 1 && overlap.overlaps.count(tempArea.index2) == 1){
                    overlap.area = tempArea.area;
                }
            }
        }
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
            newNode.nodeType = DFSLTessType::OVERLAP;
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
            newNode.nodeType = DFSLTessType::BLANK;
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
        if (bestTangent.size() == 0){
            Segment dummySeg;
            dummySeg.segStart = Cord(-1,-1);
            dummySeg.segEnd = Cord(-1,-1);
            newEdge.commonEdge = dummySeg;
            newEdge.direction = DIRECTION::NONE;
        }
        else {
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
        }
        newEdge.fromIndex = fromIndex;
        newEdge.toIndex = toIndex; 
        mAllNodes[fromIndex].edgeList.push_back(newEdge);
    }

    void DFSLegalizer::getTessNeighbors(int nodeId, std::set<int>& allNeighbors){
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
                    std::cout << "[DFSL] ERROR: Overlaps unable to resolve\n";
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
            LegalInfo legal = getLegalInfo(node.tileList);
            if (legal.util < UTIL_RULE){
                std::cout << "[DFSL] Warning: util for " << node.nodeName << " (" << legal.util << " < " << UTIL_RULE << ")\n";
                result = RESULT::CONSTRAINT_FAIL;
            }

            if (legal.aspectRatio > ASPECT_RATIO_RULE && node.nodeType == DFSLTessType::SOFT){
                std::cout << "[DFSL] Warning: aspect ratio for " << node.nodeName << " fail (" << legal.aspectRatio << " > " << ASPECT_RATIO_RULE << ")\n";
                result = RESULT::CONSTRAINT_FAIL;
            }
            else if (legal.aspectRatio < 1.0 / ASPECT_RATIO_RULE && node.nodeType == DFSLTessType::SOFT){
                std::cout << "[DFSL] Warning: aspect ratio for " << node.nodeName << " fail (" << legal.aspectRatio << " < " << 1.0 / ASPECT_RATIO_RULE << ")\n";
                result = RESULT::CONSTRAINT_FAIL;
            }
        }
        return result;
    }

    bool DFSLegalizer::migrateOverlap(int overlapIndex){
        // 0903: SEVERE BUG
        // Because migrating overlap may take several passes, an overlap may migrate to A in one iteration
        // and then to B in later iterations. But at the end, the overlap will completely assigned to one Tess
        // This causes mismatch in tess area
        // possible fix: implementation of slicing of block tiles
        mBestPath.clear();
        mCurrentPath.clear();
        mBestCost = (double) INT_MAX;
        mMigratingArea = mAllNodes[overlapIndex].area;

        std::cout << "[DFSL] Info: Migrating Overlap: " << mAllNodes[overlapIndex].nodeName << '\n';
        for (DFSLEdge edge: mAllNodes[overlapIndex].edgeList){
            dfs(edge, 0.0);
        }

        if (mBestPath.size() == 0){
            std::cout << "Path not found. Layout unchanged\n\n";
            return false;
        }

        for (DFSLEdge& edge: mBestPath){
            if (mAllNodes[edge.toIndex].nodeType == DFSLTessType::BLANK){
                std::string direction;
                switch (edge.direction)
                {
                case DIRECTION::TOP:
                    direction = "above";
                    break;
                case DIRECTION::RIGHT:
                    direction = "right of";
                    break;
                case DIRECTION::DOWN:
                    direction = "below";
                    break;
                case DIRECTION::LEFT:
                    direction = "left of";
                    break;
                default:
                    direction = "error";
                    break;
                }
                std::cout << "-> Whitespace " << direction << ' ' << mAllNodes[edge.fromIndex].nodeName 
                            << " (LL: " << mAllNodes[edge.toIndex].tileList[0]->getLowerLeft() << ") ";
            }
            else {
                std::cout << "-> " << mAllNodes[edge.toIndex].nodeName << ' ';
            }
        }
        std::cout << '\n';

        // start changing physical layout
        int resolvableArea = 0;
        for (int i = mBestPath.size() - 1; i >= 0; i--){
            DFSLEdge edge = mBestPath[i];
            DFSLNode& fromNode = mAllNodes[edge.fromIndex];
            DFSLNode& toNode = mAllNodes[edge.toIndex];

            if (fromNode.nodeType == DFSLTessType::OVERLAP && toNode.nodeType == DFSLTessType::SOFT){
                if (resolvableArea < mMigratingArea){
                    // create transient overlap area
                    std::cout << "Overlap not completely resolvable (overlap area: " << mMigratingArea << " whitespace area: " << resolvableArea << ")\n";
                    std::cout << "Saving rest of overlap to resolve later\n";
                    int index1 = *(fromNode.overlaps.begin());
                    int index2 = *(fromNode.overlaps.rbegin());

                    bool found = false;
                    for (OverlapArea& overlapInfo: mTransientOverlapArea){
                        if ((overlapInfo.index1 == index1 || overlapInfo.index2 == index1) && (overlapInfo.index1 == index2 || overlapInfo.index2 == index2)){
                            found = true;
                            overlapInfo.area = mMigratingArea - resolvableArea;
                            break;
                        }
                    }
                    if (!found){
                        OverlapArea tempArea;
                        tempArea.index1 = index1;
                        tempArea.index2 = index2;
                        tempArea.area = mMigratingArea - resolvableArea;
                        mTransientOverlapArea.push_back(tempArea);
                    }
                }
                else {
                    std::cout << "Removing " << toNode.nodeName << " attribute from " << fromNode.tileList.size() << " tiles\n";
                    int indexToRemove = edge.toIndex; // should be soft index
                    if (indexToRemove < mFixedTessNum){
                        // should not happen
                        std::cout << "[DFSL] ERROR: Migrating overlap to fixed block: " << mLF->fixedTesserae[indexToRemove]->getName() << '\n';
                    }
                    else {
                        for (Tile* overlapTile: fromNode.tileList){
                            int fixedOverlapNum = overlapTile->OverlapFixedTesseraeIdx.size();
                            int softOverlapNum = overlapTile->OverlapSoftTesseraeIdx.size();
                            if (fixedOverlapNum + softOverlapNum == 2){
                                // remove from overlaparr of other tess, add to tilearr
                                int otherIndex;
                                for (int index: overlapTile->OverlapFixedTesseraeIdx){
                                    if (index != indexToRemove){
                                        otherIndex = index;
                                    }
                                }
                                for (int index: overlapTile->OverlapSoftTesseraeIdx){
                                    if (index + mFixedTessNum != indexToRemove){
                                        otherIndex = index + mFixedTessNum;
                                    }
                                }

                                overlapTile->OverlapFixedTesseraeIdx.clear();
                                overlapTile->OverlapSoftTesseraeIdx.clear();
                                overlapTile->setType(tileType::BLOCK);

                                Tessera* otherTess = otherIndex < mFixedTessNum ? mLF->fixedTesserae[otherIndex] : mLF->softTesserae[otherIndex - mFixedTessNum];
                                removeFromVector(overlapTile, otherTess->OverlapArr);
                                otherTess->TileArr.push_back(overlapTile);

                                removeFromVector(overlapTile, mLF->softTesserae[indexToRemove - mFixedTessNum]->OverlapArr);
                            }
                            else {
                                removeFromVector(overlapTile, mLF->softTesserae[indexToRemove - mFixedTessNum]->OverlapArr);
                                removeFromVector(indexToRemove - mFixedTessNum, overlapTile->OverlapSoftTesseraeIdx);                                
                            }
                        }

                        int index1 = *(fromNode.overlaps.begin());
                        int index2 = *(fromNode.overlaps.rbegin());
                        for (std::vector<OverlapArea>::iterator it = mTransientOverlapArea.begin() ; it != mTransientOverlapArea.end(); ++it){
                            if ((it->index1 == index1 || it->index2 == index1)
                                && (it->index1 == index2 || it->index2 == index2)){
                                mTransientOverlapArea.erase(it);
                                break;
                            }
                        }
                    }
                }
            }
                // todo: deal with block -> block
            else if (fromNode.nodeType == DFSLTessType::SOFT && toNode.nodeType == DFSLTessType::BLANK){
                Cord BL;
                int width;
                int height;
                if (edge.direction == DIRECTION::TOP){
                    width = abs(edge.commonEdge.segEnd.x - edge.commonEdge.segStart.x); 
                    int requiredHeight = ceil((double) mMigratingArea / (double) width);
                    height = toNode.tileList[0]->getHeight() > requiredHeight ? requiredHeight : toNode.tileList[0]->getHeight();
                    BL.x = edge.commonEdge.segStart.x < edge.commonEdge.segEnd.x ? edge.commonEdge.segStart.x : edge.commonEdge.segEnd.x ;
                    BL.y = edge.commonEdge.segStart.y;
                }
                else if (edge.direction == DIRECTION::RIGHT){
                    height = abs(edge.commonEdge.segEnd.y - edge.commonEdge.segStart.y); 
                    int requiredWidth = ceil((double) mMigratingArea / (double) height);
                    width = toNode.tileList[0]->getWidth() > requiredWidth ? requiredWidth : toNode.tileList[0]->getWidth();
                    BL.x = edge.commonEdge.segStart.x;
                    BL.y = edge.commonEdge.segStart.y < edge.commonEdge.segEnd.y ? edge.commonEdge.segStart.y : edge.commonEdge.segEnd.y ;
                }
                else if (edge.direction == DIRECTION::DOWN){
                    width = abs(edge.commonEdge.segEnd.x - edge.commonEdge.segStart.x); 
                    int requiredHeight = ceil((double) mMigratingArea / (double) width);
                    height = toNode.tileList[0]->getHeight() > requiredHeight ? requiredHeight : toNode.tileList[0]->getHeight();
                    BL.x = edge.commonEdge.segStart.x < edge.commonEdge.segEnd.x ? edge.commonEdge.segStart.x : edge.commonEdge.segEnd.x ;
                    BL.y = edge.commonEdge.segStart.y - height;
                }
                else if (edge.direction == DIRECTION::LEFT){
                    height = abs(edge.commonEdge.segEnd.y - edge.commonEdge.segStart.y); 
                    int requiredWidth = ceil((double) mMigratingArea / (double) height);
                    width = toNode.tileList[0]->getWidth() > requiredWidth ? requiredWidth : toNode.tileList[0]->getWidth();
                    BL.x = edge.commonEdge.segStart.x - width;
                    BL.y = edge.commonEdge.segStart.y < edge.commonEdge.segEnd.y ? edge.commonEdge.segStart.y : edge.commonEdge.segEnd.y ;
                }
                else {
                    std::cout << "[DFSL] ERROR: BW edge ( " << fromNode.nodeName << " -> " << toNode.tileList[0]->getLowerLeft() << " ) must have DIRECTION\n";
                    return false;
                }

                resolvableArea = width * height;
                Tile* newTile = new Tile(tileType::BLOCK, BL, width, height);
                std::cout << "Placing new Tile: ";
                newTile->show(std::cout, true);

                // add to tess, place in physical layout
                Tessera* blockTess = mLF->softTesserae[fromNode.index - mFixedTessNum]; 
                blockTess->TileArr.push_back(newTile);

                mLF->insertTile(*newTile);

            }
                // todo: deal with white -> white
            else {
                std::cout << "[DFSL] Warning: Doing nothing for migrating path: " << fromNode.nodeName << " -> " << toNode.nodeName << '\n';
            }
        }

        std::cout << "\n";
        // mLF->visualiseArtpiece("debug_DFSL.txt", true);
        return true;
    }

    // todo: turn into template function
    bool removeFromVector(int a, std::vector<int>& vec){
        for (std::vector<int>::iterator it = vec.begin() ; it != vec.end(); ++it){
            if (*it == a){
                vec.erase(it);
                return true;
            }
        }
        return false;
    }

    bool removeFromVector(Tile* a, std::vector<Tile*>& vec){
        for (std::vector<Tile*>::iterator it = vec.begin() ; it != vec.end(); ++it){
            if (*it == a){
                vec.erase(it);
                return true;
            }
        }
        return false;
    }

    void DFSLegalizer::dfs(DFSLEdge edge, double currentCost){
        int toIndex = edge.toIndex;
        double edgeCost = getEdgeCost(edge);
        currentCost += edgeCost;
        mCurrentPath.push_back(edge);
        int blankStart = mFixedTessNum + mSoftTessNum + mOverlapNum;
        int blankEnd = blankStart + mBlankNum;
        if (blankStart <= toIndex && toIndex < blankEnd){
            if (currentCost < mBestCost){
                mBestPath = mCurrentPath;
                mBestCost = currentCost;
            }
        }

        if (currentCost < config.maxCostCutoff){
            for (DFSLEdge nextEdge: mAllNodes[toIndex].edgeList){
                if (!inVector(nextEdge.toIndex, mCurrentPath)){
                    dfs(nextEdge, currentCost);
                }
            }
        }

        mCurrentPath.pop_back();
    }

    double DFSLegalizer::getEdgeCost(DFSLEdge edge){
        enum class EDGETYPE : unsigned char { OB, BB, BW, WW, BAD_EDGE };
        EDGETYPE edgeType; 

        if (mAllNodes[edge.fromIndex].nodeType == DFSLTessType::OVERLAP && mAllNodes[edge.toIndex].nodeType == DFSLTessType::SOFT){
            edgeType = EDGETYPE::OB;
        }
        else if (mAllNodes[edge.fromIndex].nodeType == DFSLTessType::SOFT && mAllNodes[edge.toIndex].nodeType == DFSLTessType::SOFT){
            edgeType = EDGETYPE::BB;
        } 
        else if (mAllNodes[edge.fromIndex].nodeType == DFSLTessType::SOFT && mAllNodes[edge.toIndex].nodeType == DFSLTessType::BLANK){
            edgeType = EDGETYPE::BW;
        }
        else if (mAllNodes[edge.fromIndex].nodeType == DFSLTessType::BLANK && mAllNodes[edge.toIndex].nodeType == DFSLTessType::BLANK){
            edgeType = EDGETYPE::WW;
        }
        else {
            std::cout << "[DFSL] Warning: Edge shouldn't exist\n";
            edgeType = EDGETYPE::BAD_EDGE;
        }

        DFSLNode& fromNode = mAllNodes[edge.fromIndex];
        DFSLNode& toNode = mAllNodes[edge.toIndex];
        double edgeCost = 0.0;
        
        switch (edgeType)
        {
        case EDGETYPE::OB:
        {
            Tessera* toTess = mLF->softTesserae[edge.toIndex - mFixedTessNum];
            std::set<Tile*> overlap;
            std::set<Tile*> withoutOverlap;
            std::set<Tile*> withOverlap;

            for (Tile* tile: toTess->TileArr){
                withoutOverlap.insert(tile); 
                withOverlap.insert(tile);
            }
            for (Tile* tile: toTess->OverlapArr){
                withoutOverlap.insert(tile); 
                withOverlap.insert(tile);
            }
            
            for (Tile* tile: fromNode.tileList){
                overlap.insert(tile);
                withoutOverlap.erase(tile);
            }

            LegalInfo overlapInfo = getLegalInfo(overlap);
            LegalInfo withOverlapInfo = getLegalInfo(withOverlap);
            LegalInfo withoutOverlapInfo = getLegalInfo(withoutOverlap);

            // get area
            double overlapArea = overlapInfo.actualArea;
            double blockArea = withoutOverlapInfo.actualArea;

            // get util
            double withOverlapUtil = withOverlapInfo.util;
            double withoutOverlapUtil = withoutOverlapInfo.util;

            // get aspect ratio without overlap
            double aspectRatio = withoutOverlapInfo.aspectRatio;

            edgeCost += (overlapArea / blockArea) * config.OBAreaWeight + 
                        (withOverlapUtil - withoutOverlapUtil) * config.OBUtilWeight +
                        // tan(pow(withOverlapUtil - withoutOverlapUtil, 2.0) * PI / 180.0) * config.OBUtilWeight +
                        tan(pow(aspectRatio - 1.0, 2.0) * PI / 180.0) * config.OBAspWeight; 

            break;
        }

        case EDGETYPE::BB:
            edgeCost += config.BBFlatCost;
            break;

        case EDGETYPE::BW:
        {
            std::set<Tile*> oldBlock;
            std::set<Tile*> newBlock;

            for (Tile* tile: fromNode.tileList){
                oldBlock.insert(tile);
                newBlock.insert(tile);
            }

            // predict new tile
            Cord BL;
            int width;
            int height;
            if (edge.direction == DIRECTION::TOP){
                width = abs(edge.commonEdge.segEnd.x - edge.commonEdge.segStart.x); 
                int requiredHeight = ceil((double) mMigratingArea / (double) width);
                height = toNode.tileList[0]->getHeight() > requiredHeight ? requiredHeight : toNode.tileList[0]->getHeight();
                BL.x = edge.commonEdge.segStart.x < edge.commonEdge.segEnd.x ? edge.commonEdge.segStart.x : edge.commonEdge.segEnd.x ;
                BL.y = edge.commonEdge.segStart.y;
            }
            else if (edge.direction == DIRECTION::RIGHT){
                height = abs(edge.commonEdge.segEnd.y - edge.commonEdge.segStart.y); 
                int requiredWidth = ceil((double) mMigratingArea / (double) height);
                width = toNode.tileList[0]->getWidth() > requiredWidth ? requiredWidth : toNode.tileList[0]->getWidth();
                BL.x = edge.commonEdge.segStart.x;
                BL.y = edge.commonEdge.segStart.y < edge.commonEdge.segEnd.y ? edge.commonEdge.segStart.y : edge.commonEdge.segEnd.y ;
            }
            else if (edge.direction == DIRECTION::DOWN){
                width = abs(edge.commonEdge.segEnd.x - edge.commonEdge.segStart.x); 
                int requiredHeight = ceil((double) mMigratingArea / (double) width);
                height = toNode.tileList[0]->getHeight() > requiredHeight ? requiredHeight : toNode.tileList[0]->getHeight();
                BL.x = edge.commonEdge.segStart.x < edge.commonEdge.segEnd.x ? edge.commonEdge.segStart.x : edge.commonEdge.segEnd.x ;
                BL.y = edge.commonEdge.segStart.y - height;
            }
            else if (edge.direction == DIRECTION::LEFT) {
                height = abs(edge.commonEdge.segEnd.y - edge.commonEdge.segStart.y); 
                int requiredWidth = ceil((double) mMigratingArea / (double) height);
                width = toNode.tileList[0]->getWidth() > requiredWidth ? requiredWidth : toNode.tileList[0]->getWidth();
                BL.x = edge.commonEdge.segStart.x - width;
                BL.y = edge.commonEdge.segStart.y < edge.commonEdge.segEnd.y ? edge.commonEdge.segStart.y : edge.commonEdge.segEnd.y ;
            }
            else {
                std::cout << "[DFSL] ERROR: BW edge ( " << fromNode.nodeName << " -> " << toNode.tileList[0]->getLowerLeft() << " ) must have DIRECTION\n";
                return (double) INT_MAX;
            }

            Tile newTile(tileType::BLOCK, BL, width, height);
            newBlock.insert(&newTile);

            LegalInfo oldBlockInfo = getLegalInfo(oldBlock);
            LegalInfo newBlockInfo = getLegalInfo(newBlock);

            // get util
            double oldBlockUtil = oldBlockInfo.util;
            double newBlockUtil = newBlockInfo.util;

            // get aspect ratio with new area
            double aspectRatio = newBlockInfo.aspectRatio;

            edgeCost += (oldBlockUtil - newBlockUtil) * config.OBUtilWeight +
                        tan(pow(aspectRatio - 1.0, 4.0) * PI / 180.0) * config.OBAspWeight; 

            break;
        }

        case EDGETYPE::WW:
            edgeCost += config.WWFlatCost;
            break;

        default:
            edgeCost += config.maxCostCutoff * 100;
            break;
        }

        return edgeCost;
    }


    inline bool inVector(int a, std::vector<DFSLEdge>& vec){
        for (DFSLEdge& edge: vec){
            if (edge.fromIndex == a || edge.toIndex == a) {
                return true;
            }
        }
        return false;
    } 

    Config::Config(): maxCostCutoff(1000000.0),
                        OBAreaWeight(750.0),
                        OBUtilWeight(1000.0),
                        OBAspWeight(100.0),
                        BWUtilWeight(1500.0),
                        BWAspWeight(100.0),
                        BBFlatCost(1000000.0),
                        WWFlatCost(1000000.0) { ; }

    LegalInfo DFSLegalizer::getLegalInfo(std::vector<Tile*>& tiles){
        LegalInfo legal;
        int min_x, max_x, min_y, max_y;
        min_x = min_y = INT_MAX;
        max_x = max_y = -INT_MAX;
        legal.actualArea = 0;
        for (Tile* tile: tiles){
            if (tile->getLowerLeft().x < min_x){
                min_x = tile->getLowerLeft().x;
            }            
            if (tile->getLowerLeft().y < min_y){
                min_y = tile->getLowerLeft().y;
            }            
            if (tile->getUpperRight().x > max_x){
                max_x = tile->getUpperRight().x;
            }            
            if (tile->getUpperRight().y > max_y){
                max_y = tile->getUpperRight().y;
            }
            legal.actualArea += tile->getArea();   
        }
        legal.width = max_x - min_x;
        legal.height = max_y - min_y;

        legal.bbArea = legal.width * legal.height; 
        legal.BL = Cord(min_x, min_y);  
        legal.aspectRatio = ((double) legal.width) / ((double) legal.height);
        legal.util = ((double) legal.actualArea) / ((double) legal.bbArea);

        return legal;
    } 

    LegalInfo DFSLegalizer::getLegalInfo(std::set<Tile*>& tiles){
        LegalInfo legal;
        int min_x, max_x, min_y, max_y;
        min_x = min_y = INT_MAX;
        max_x = max_y = -INT_MAX;
        legal.actualArea = 0;
        for (Tile* tile: tiles){
            if (tile->getLowerLeft().x < min_x){
                min_x = tile->getLowerLeft().x;
            }            
            if (tile->getLowerLeft().y < min_y){
                min_y = tile->getLowerLeft().y;
            }            
            if (tile->getUpperRight().x > max_x){
                max_x = tile->getUpperRight().x;
            }            
            if (tile->getUpperRight().y > max_y){
                max_y = tile->getUpperRight().y;
            }
            legal.actualArea += tile->getArea();   
        }
        legal.width = max_x - min_x;
        legal.height = max_y - min_y;

        legal.bbArea = legal.width * legal.height; 
        legal.BL = Cord(min_x, min_y);  
        legal.aspectRatio = ((double) legal.width) / ((double) legal.height);
        legal.util = ((double) legal.actualArea) / ((double) legal.bbArea);

        return legal;
    } 

    static bool compareSegment(Segment a, Segment b){
        return (a.segStart.x < b.segStart.x || a.segStart.y < b.segStart.y);
    }

}