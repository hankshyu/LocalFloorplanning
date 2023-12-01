#include "DFSLegalizer.h"
#include "Tile.h"
#include "LFLegaliser.h"
#include <vector>
#include <utility>
#include <cstdint>
#include <numeric>
#include "boost/polygon/polygon.hpp"

namespace DFSL {

namespace gtl = boost::polygon;
typedef gtl::rectangle_data<len_t> Rectangle;
typedef gtl::polygon_90_set_data<len_t> Polygon90Set;
typedef gtl::polygon_90_with_holes_data<len_t> Polygon90WithHoles;
typedef gtl::point_data<len_t> Point;
using namespace boost::polygon::operators;

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
    // This line would cuase bug...
    DFSLTraverseBlank(mLF->getRandomTile(), blankRecord);
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

    // this takes care of the case where a block is completely covered by overlap tiles,
    // no edge from overlap to block
    if (fromNode.tileList.size() == 0 || toNode.tileList.size() == 0){
        return;
    }
    std::vector<Segment> allTangentSegments;
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
                            tangent.direction = DIRECTION::TOP;
                        }
                        else if (dir == 1){
                            // neighbor is right of fromNode 
                            fromStart = tile->getLowerRight();
                            fromEnd = tile->getUpperRight();
                            toStart = neighbor->getLowerLeft(); 
                            toEnd = neighbor->getUpperLeft();
                            tangent.direction = DIRECTION::RIGHT;
                        }
                        else if (dir == 2){
                            // neighbor is bottom of fromNode 
                            fromStart = tile->getLowerLeft();
                            fromEnd = tile->getLowerRight();
                            toStart = neighbor->getUpperLeft(); 
                            toEnd = neighbor->getUpperRight();
                            tangent.direction = DIRECTION::DOWN;
                        }
                        else {
                            // neighbor is left of fromNode 
                            fromStart = tile->getLowerLeft();
                            fromEnd = tile->getUpperLeft();
                            toStart = neighbor->getLowerRight(); 
                            toEnd = neighbor->getUpperRight();
                            tangent.direction = DIRECTION::LEFT;
                        }
                        tangent.segStart = fromStart <= toStart ? toStart : fromStart;
                        tangent.segEnd = fromEnd <= toEnd ? fromEnd : toEnd;
                        currentSegment.push_back(tangent);
                    }
                }
            }
        }
        
        // check segment, splice touching segments together and add to allTangentSegments
        int segLength = 0;
        if (currentSegment.size() > 0){
            if (dir == 0 || dir == 2){
                std::sort(currentSegment.begin(), currentSegment.end(), compareXSegment);
            }
            else {
                std::sort(currentSegment.begin(), currentSegment.end(), compareXSegment);
            }
            Cord segBegin = currentSegment.front().segStart;
            for (int j = 1; j < currentSegment.size(); j++){
                if (currentSegment[j].segStart != currentSegment[j-1].segEnd){
                    Cord segEnd = currentSegment[j-1].segEnd;
                    Segment splicedSegment;
                    splicedSegment.segStart = segBegin;
                    splicedSegment.segEnd = segEnd;
                    splicedSegment.direction = currentSegment[j-1].direction;
                    allTangentSegments.push_back(splicedSegment);
                    segLength += (segEnd.x - segBegin.x) + (segEnd.y - segBegin.y);  

                    segBegin = currentSegment[j].segStart;
                }
            }
            Cord segEnd = currentSegment.back().segEnd;
            Segment splicedSegment;
            splicedSegment.segStart = segBegin;
            splicedSegment.segEnd = segEnd;
            splicedSegment.direction = currentSegment.back().direction;
            allTangentSegments.push_back(splicedSegment);
            segLength += (segEnd.x - segBegin.x) + (segEnd.y - segBegin.y);  
        }
    }

    // construct edge in graph
    DFSLEdge newEdge;
    newEdge.tangentSegments = allTangentSegments;
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

// mode 0: resolve area big -> area small
// mode 1: resolve area small -> area big
// mode 2: resolve overlaps near center -> outer edge
RESULT DFSLegalizer::legalize(int mode){
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
            int bestMetric;
            if (mode == 1 || mode == 2){
                bestMetric = INT_MAX;
            }
            else {
                bestMetric = -INT_MAX;
            }
            int bestIndex = -1;
            for (int i = overlapStart; i < overlapEnd; i++){
                DFSLNode& currentOverlap = mAllNodes[i];
                switch (mode)
                {
                case 1:
                    if (currentOverlap.area < bestMetric && solveable[i]){
                        bestMetric = currentOverlap.area;
                        bestIndex = i;
                    }
                    break;
                
                case 2:{
                    int chipCenterx = mLF->getCanvasWidth() / 2;
                    int chipCentery = mLF->getCanvasHeight() / 2;
                    
                    int min_x, max_x, min_y, max_y;
                    min_x = min_y = INT_MAX;
                    max_x = max_y = -INT_MAX;

                    for (Tile* tile: currentOverlap.tileList){
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
                    }
                    int overlapCenterx = (min_x + max_x) / 2;
                    int overlapCentery = (min_y + max_y) / 2;
                    int distSquared = pow(overlapCenterx - chipCenterx, 2) + pow(overlapCentery - chipCentery, 2);

                    if (distSquared < bestMetric && solveable[i]){
                        bestMetric = currentOverlap.area;
                        bestIndex = i;
                    }

                    break;
                }
                default:
                    // area big -> small, default
                    
                    if (currentOverlap.area > bestMetric && solveable[i]){
                        bestMetric = currentOverlap.area;
                        bestIndex = i;
                    }
                    break;
                }
            }
            if (bestIndex == -1){
                std::cout << "[DFSL] ERROR: Overlaps unable to resolve\n";
                result = RESULT::OVERLAP_NOT_RESOLVED;
                return result;
            }
            else {
                solveable[bestIndex] = overlapResolved = migrateOverlap(bestIndex);
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

bool DFSLegalizer::splitOverlap(MigrationEdge& edge, int resolvableArea){
    DFSLNode& fromNode = mAllNodes[edge.fromIndex];
    DFSLNode& toNode = mAllNodes[edge.toIndex];
    if (!(fromNode.nodeType == DFSLTessType::OVERLAP && toNode.nodeType == DFSLTessType::SOFT)){
        return false;
    }

    // Find union of overlap
    // chances are it is a rectangle
    Polygon90Set overlapPolygonSet;
    std::vector<Rectangle> rectangleContainer;
    overlapPolygonSet.clear();
    for (Tile* overlapTile: fromNode.tileList){
        Polygon90WithHoles tileRectangle;
        std::vector<Point> newAreaVertices = {
            {overlapTile->getLowerLeft().x, overlapTile->getLowerLeft().y}, 
            {overlapTile->getLowerRight().x, overlapTile->getLowerRight().y }, 
            {overlapTile->getUpperRight().x, overlapTile->getUpperRight().y}, 
            {overlapTile->getUpperLeft().x, overlapTile->getUpperLeft().y}
        };
        gtl::set_points(tileRectangle, newAreaVertices.begin(), newAreaVertices.end());
        overlapPolygonSet += tileRectangle;
    }
    overlapPolygonSet.get_rectangles(rectangleContainer);

    if (rectangleContainer.size() == 0){
        std::cout << "[DFSL] Error: Unable to find overlap area\n";
        return false;
    }
    if (rectangleContainer.size() != 1){
        std::cout << "[DFSL] Warning: Overlap not rectangle, reverting to default method.\n";
        return false; 
    }
    
    Rectangle overlapRec = rectangleContainer[0];
    
    // find segment directions
    // if overlap flows x area to A, then x amount of area should be assigned to B in overlap
    DFSLEdge* fullEdge = NULL;
    for (DFSLEdge& fromEdge: fromNode.edgeList){
        if (fromEdge.fromIndex == edge.fromIndex && fromEdge.toIndex == edge.toIndex){
            fullEdge = &fromEdge;
        }
    }
    if (fullEdge == NULL){
        std::cout << "[DFSL] Error: OB edge not found.\n";
        return false;
    }

    std::vector<bool> sideOccupied(4, false);
    for (Segment& segment: fullEdge->tangentSegments){
        switch (segment.direction){
        case DIRECTION::TOP:
            sideOccupied[0] = true;
            break;
        case DIRECTION::RIGHT:
            sideOccupied[1] = true;
            break;
        case DIRECTION::DOWN:
            sideOccupied[2] = true;
            break;
        case DIRECTION::LEFT:
            sideOccupied[3] = true;
            break;
        default:
            std::cout << "[DFSL] Warning: OB edge segment has no direction\n";
            break;
        }
    }

    // for each unoccupied side of the rectangle,
    // calculate the area of the rectangle extending from B
    // keep the one with the area closest to the actual resolvableArea
    int closestArea = -1;
    DIRECTION bestDirection = DIRECTION::NONE;
    Rectangle bestRectangle;
    if (!sideOccupied[0]){
        // grow from top side
        int width = gtl::delta(overlapRec, gtl::orientation_2d_enum::HORIZONTAL);
        int height = (int) floor((double) resolvableArea / (double) width);
        int thisArea = width * height;
        if (thisArea > closestArea){
            closestArea = thisArea;
            bestDirection = DIRECTION::TOP;
            int newYl = gtl::yh(overlapRec) - height;
            gtl::assign(bestRectangle, overlapRec);
            gtl::yl(bestRectangle, newYl);
        }
    }
    if (!sideOccupied[1]){
        // grow from right side
        int height = gtl::delta(overlapRec, gtl::orientation_2d_enum::VERTICAL);
        int width = (int) floor((double) resolvableArea / (double) height);
        int thisArea = width * height;
        if (thisArea > closestArea){
            closestArea = thisArea;
            bestDirection = DIRECTION::RIGHT;
            int newXl = gtl::xh(overlapRec) - width;
            gtl::assign(bestRectangle, overlapRec);
            gtl::xl(bestRectangle, newXl);
        }
    }
    if (!sideOccupied[2]){
        // grow from bottom side
        int width = gtl::delta(overlapRec, gtl::orientation_2d_enum::HORIZONTAL);
        int height = (int) floor((double) resolvableArea / (double) width);
        int thisArea = width * height;
        if (thisArea > closestArea){
            closestArea = thisArea;
            bestDirection = DIRECTION::DOWN;
            int newYh = gtl::yl(overlapRec) + height;
            gtl::assign(bestRectangle, overlapRec);
            gtl::yh(bestRectangle, newYh);
        }
    }
    if (!sideOccupied[3]){
        // grow from left side
        int height = gtl::delta(overlapRec, gtl::orientation_2d_enum::VERTICAL);
        int width = (int) floor((double) resolvableArea / (double) height);
        int thisArea = width * height;
        if (thisArea > closestArea){
            closestArea = thisArea;
            bestDirection = DIRECTION::LEFT;
            int newXh = gtl::xl(overlapRec) + width;
            gtl::assign(bestRectangle, overlapRec);
            gtl::xh(bestRectangle, newXh);
        }
    }
    
    if (closestArea == 0){
        return true;
    }
    
    // start splitting tiles

    assert(bestDirection != DIRECTION::NONE);
    assert(gtl::contains(overlapRec, bestRectangle));

    for (Tile* overlapTile: fromNode.tileList){
        Rectangle tileRect(overlapTile->getLowerLeft().x, overlapTile->getLowerLeft().y,
                           overlapTile->getUpperRight().x, overlapTile->getUpperRight().y);
        
        bool intersects = gtl::intersect(tileRect, bestRectangle);
        if (intersects){
            int direction;
            switch (bestDirection){
            case DIRECTION::TOP:
                direction = 0;
                break;
            case DIRECTION::RIGHT:
                direction = 1;
                break;
            case DIRECTION::DOWN:
                direction = 2;
                break;
            case DIRECTION::LEFT:
                direction = 3;
                break;
            default:
                direction = 0;
                break;
            }
            Tile* newTile = mLF->simpleSplitTile(*(overlapTile), tileRect, direction);
            if (newTile == NULL){
                std::cout << "[DFSL] Error: Simple split tile failed\n";
            }
            else if (newTile == overlapTile){
                // remove from original tessera, add to new Tessera
                int indexToRemove = edge.toIndex;
                removeIndexFromOverlap(indexToRemove, overlapTile);
            }
            else {
                newTile->OverlapFixedTesseraeIdx = overlapTile->OverlapFixedTesseraeIdx;
                newTile->OverlapSoftTesseraeIdx = overlapTile->OverlapSoftTesseraeIdx;
                for (int fixedTessIndex: newTile->OverlapFixedTesseraeIdx){
                    Tessera* fixedTess = mLF->fixedTesserae[fixedTessIndex];
                    fixedTess->OverlapArr.push_back(newTile);
                }
                for (int softTessIndex: newTile->OverlapSoftTesseraeIdx){
                    Tessera* softTess = mLF->softTesserae[softTessIndex];
                    softTess->OverlapArr.push_back(newTile);
                }

                int indexToRemove = edge.toIndex;
                removeIndexFromOverlap(indexToRemove, newTile);
            }
        }
    }
    return true;
}

bool DFSLegalizer::migrateOverlap(int overlapIndex){
    mBestPath.clear();
    mCurrentPath.clear();
    mBestCost = (double) INT_MAX;
    mMigratingArea = mAllNodes[overlapIndex].area;

    std::cout << "[DFSL] Info: Migrating Overlap: " << mAllNodes[overlapIndex].nodeName << '\n';
    for (DFSLEdge& edge: mAllNodes[overlapIndex].edgeList){
        dfs(edge, 0.0);
    }

    if (mBestPath.size() == 0){
        std::cout << "Path not found. Layout unchanged\n\n";
        return false;
    }

    for (MigrationEdge& edge: mBestPath){
        if (mAllNodes[edge.toIndex].nodeType == DFSLTessType::BLANK){
            std::string direction;
            switch (edge.segment.direction)
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
        MigrationEdge& edge = mBestPath[i];
        DFSLNode& fromNode = mAllNodes[edge.fromIndex];
        DFSLNode& toNode = mAllNodes[edge.toIndex];

        if (fromNode.nodeType == DFSLTessType::OVERLAP && toNode.nodeType == DFSLTessType::SOFT){
            if (resolvableArea < mMigratingArea){
                // create transient overlap area

                bool result = splitOverlap(edge, resolvableArea);

                std::cout << "Overlap not completely resolvable (overlap area: " << mMigratingArea << " whitespace area: " << resolvableArea << ")\n";
                if (result){
                    std::cout << "Splitting overlap tile(s).\n";
                }
                else {
                    std::cout << "Split failed. Storing in value of remaining overlap area.\n";
                    OverlapArea tempArea;
                    tempArea.index1 = *(fromNode.overlaps.begin());
                    tempArea.index2 = *(fromNode.overlaps.rbegin());
                    tempArea.area = mMigratingArea - resolvableArea;

                    mTransientOverlapArea.push_back(tempArea);
                }
            }
            else {
                std::cout << "Removing " << toNode.nodeName << " attribute from " << fromNode.tileList.size() << " tiles\n";
                int indexToRemove = edge.toIndex; // should be soft index
                for (Tile* overlapTile: fromNode.tileList){
                    removeIndexFromOverlap(indexToRemove, overlapTile);
                }
            }
        }
            // todo: deal with block -> block
        else if (fromNode.nodeType == DFSLTessType::SOFT && toNode.nodeType == DFSLTessType::BLANK){
            if (edge.segment.direction == DIRECTION::NONE) {
                std::cout << "[DFSL] ERROR: BW edge ( " << fromNode.nodeName << " -> " << toNode.tileList[0]->getLowerLeft() << " ) must have DIRECTION\n";
                return false;
            }

            resolvableArea = edge.migratedArea.getArea();   
            Tile* newTile = new Tile(edge.migratedArea);
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
    mLF->visualiseArtpiece("debug_DFSL.txt", true);
    return true;
}

void DFSLegalizer::removeIndexFromOverlap(int indexToRemove, Tile* overlapTile){
    if (indexToRemove < mFixedTessNum){
        // should not happen
        std::cout << "[DFSL] ERROR: Migrating overlap to fixed block: " << mLF->fixedTesserae[indexToRemove]->getName() << '\n';
        return;
    }

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

void DFSLegalizer::dfs(DFSLEdge& edge, double currentCost){
    int toIndex = edge.toIndex;
    MigrationEdge edgeResult = getEdgeCost(edge);
    double edgeCost = edgeResult.edgeCost;
    currentCost += edgeCost;
    mCurrentPath.push_back(edgeResult);
    int blankStart = mFixedTessNum + mSoftTessNum + mOverlapNum;
    int blankEnd = blankStart + mBlankNum;
    if (blankStart <= toIndex && toIndex < blankEnd){
        if (currentCost < mBestCost){
            mBestPath = mCurrentPath;
            mBestCost = currentCost;
        }
    }

    if (currentCost < config.maxCostCutoff){
        for (DFSLEdge& nextEdge: mAllNodes[toIndex].edgeList){
            for (MigrationEdge& edge: mCurrentPath){
                if (edge.fromIndex == nextEdge.toIndex || edge.toIndex == nextEdge.toIndex) {
                    continue;
                }
            }
            dfs(nextEdge, currentCost);
        }
    }

    mCurrentPath.pop_back();
}

MigrationEdge DFSLegalizer::getEdgeCost(DFSLEdge& edge){
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
    Tile returnTile;
    Segment bestSegment;
    
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
        double areaWeight = (overlapArea / blockArea) * config.OBAreaWeight;

        // get util
        double withOverlapUtil = withOverlapInfo.util;
        double withoutOverlapUtil = withoutOverlapInfo.util;
        // positive reinforcement if util is improved
        double utilCost = (withOverlapUtil < withoutOverlapUtil) ? (withoutOverlapUtil - withOverlapUtil) * config.OBUtilPosRein :  
                                                        (1.0 - withoutOverlapUtil) * config.OBUtilWeight;


        // get value of long/short side without overlap
        double aspectRatio = withoutOverlapInfo.aspectRatio;
        aspectRatio = aspectRatio > 1.0 ? aspectRatio : 1.0/aspectRatio;
        double aspectCost = pow(aspectRatio - 1.0, 4) * config.OBAspWeight;

        edgeCost += areaWeight + utilCost + aspectCost;

        break;
    }

    case EDGETYPE::BB:
        // DO THIS DO THAT JERMA
        edgeCost += config.BBFlatCost;
        break;

    case EDGETYPE::BW:
    {
        std::vector<Tile*> oldBlock;
        std::vector<Tile*> newBlock;

        for (Tile* tile: fromNode.tileList){
            oldBlock.push_back(tile);
            newBlock.push_back(tile);
        }
        LegalInfo oldBlockInfo = getLegalInfo(oldBlock);

        // predict new tile
        double lowestCost = (double) LONG_MAX;
        Tile bestTile;
        int bestSegmentIndex = -1;
        for (int s = 0; s < edge.tangentSegments.size(); s++){
            Segment seg = edge.tangentSegments[s];
            Cord BL;
            int width;
            int height;
            if (seg.direction == DIRECTION::TOP){
                width = abs(seg.segEnd.x - seg.segStart.x); 
                int requiredHeight = ceil((double) mMigratingArea / (double) width);
                height = toNode.tileList[0]->getHeight() > requiredHeight ? requiredHeight : toNode.tileList[0]->getHeight();
                BL.x = seg.segStart.x < seg.segEnd.x ? seg.segStart.x : seg.segEnd.x ;
                BL.y = seg.segStart.y;
            }
            else if (seg.direction == DIRECTION::RIGHT){
                height = abs(seg.segEnd.y - seg.segStart.y); 
                int requiredWidth = ceil((double) mMigratingArea / (double) height);
                width = toNode.tileList[0]->getWidth() > requiredWidth ? requiredWidth : toNode.tileList[0]->getWidth();
                BL.x = seg.segStart.x;
                BL.y = seg.segStart.y < seg.segEnd.y ? seg.segStart.y : seg.segEnd.y ;
            }
            else if (seg.direction == DIRECTION::DOWN){
                width = abs(seg.segEnd.x - seg.segStart.x); 
                int requiredHeight = ceil((double) mMigratingArea / (double) width);
                height = toNode.tileList[0]->getHeight() > requiredHeight ? requiredHeight : toNode.tileList[0]->getHeight();
                BL.x = seg.segStart.x < seg.segEnd.x ? seg.segStart.x : seg.segEnd.x ;
                BL.y = seg.segStart.y - height;
            }
            else if (seg.direction == DIRECTION::LEFT) {
                height = abs(seg.segEnd.y - seg.segStart.y); 
                int requiredWidth = ceil((double) mMigratingArea / (double) height);
                width = toNode.tileList[0]->getWidth() > requiredWidth ? requiredWidth : toNode.tileList[0]->getWidth();
                BL.x = seg.segStart.x - width;
                BL.y = seg.segStart.y < seg.segEnd.y ? seg.segStart.y : seg.segEnd.y ;
            }
            else {
                std::cout << "[DFSL] WARNING: BW edge ( " << fromNode.nodeName << " -> " << toNode.tileList[0]->getLowerLeft() << " ) has no DIRECTION\n";
            }

            Tile newTile(tileType::BLOCK, BL, width, height);
            newBlock.push_back(&newTile);

            LegalInfo newBlockInfo = getLegalInfo(newBlock);

            newBlock.pop_back();

            // get util
            double oldBlockUtil = oldBlockInfo.util;
            double newBlockUtil = newBlockInfo.util;
            // positive reinforcement if util is improved
            double utilCost;
            utilCost = (oldBlockUtil < newBlockUtil) ? (newBlockUtil - oldBlockUtil) * config.BWUtilPosRein :  
                                                        (1.0 - newBlockUtil) * config.BWUtilWeight;

            // get aspect ratio with new area
            double aspectRatio = newBlockInfo.aspectRatio;
            aspectRatio = aspectRatio > 1.0 ? aspectRatio : 1.0/aspectRatio;
            double arCost;
            arCost = pow(aspectRatio - 1.0, 4.0) * config.BWAspWeight;

            double cost = utilCost + arCost;
                            
            if (cost < lowestCost){
                lowestCost = cost;
                bestSegmentIndex = s;
                bestTile = newTile;
            }
        }

        bestSegment = edge.tangentSegments[bestSegmentIndex];
        returnTile = bestTile;
        edgeCost += lowestCost; 

        break;
    }

    case EDGETYPE::WW:
        edgeCost += config.WWFlatCost;
        break;

    default:
        edgeCost += config.maxCostCutoff * 100;
        break;
    }

    // note: returnTile & bestSegment are not initialized if OB edge
    return MigrationEdge(edge.fromIndex, edge.toIndex, returnTile, bestSegment, edgeCost);
}

Config::Config(): maxCostCutoff(1000000.0),
                    OBAreaWeight(750.0),
                    OBUtilWeight(1000.0),
                    OBAspWeight(100.0),
                    OBUtilPosRein(-500.0),
                    BWUtilWeight(1500.0),
                    BWUtilPosRein(-500.0),
                    BWAspWeight(500.0),
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


MigrationEdge::MigrationEdge(int from, int to, Tile& area, Segment& seg, double cost):
    fromIndex(from), toIndex(to), segment(seg), migratedArea(area), edgeCost(cost){ ; }

MigrationEdge::MigrationEdge():
    fromIndex(-1), toIndex(-1), segment(Segment()), migratedArea(Tile()), edgeCost(0.0) { ; }


static bool compareXSegment(Segment a, Segment b){
    return a.segStart.y == b.segStart.y ? a.segStart.x < b.segStart.x : a.segStart.y < b.segStart.y ;
}

static bool compareYSegment(Segment a, Segment b){
    return a.segStart.x == b.segStart.x ? a.segStart.y < b.segStart.y : a.segStart.x < b.segStart.x ;
}

}