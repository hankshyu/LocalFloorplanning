#include <iostream>
#include "LFLegaliser.h"

LFLegaliser::LFLegaliser(len_t chipWidth, len_t chipHeight)
    : mCanvasWidth(chipWidth), mCanvasHeight(chipHeight) {}

LFLegaliser::~LFLegaliser() {
    for ( int i = 0; i < softTesserae.size(); i++ )
        delete softTesserae[i];
    for ( int i = 0; i < fixedTesserae.size(); i++ )
        delete fixedTesserae[i];
    //std::cerr << "LFLegaliser Deleted Successfully\n";
}

bool LFLegaliser::checkTesseraInCanvas(Cord lowerLeft, len_t width, len_t height) const {
    bool x_valid, y_valid;
    x_valid = (lowerLeft.x >= 0) && (lowerLeft.x + width <= this->mCanvasWidth);
    y_valid = (lowerLeft.y >= 0) && (lowerLeft.y + height <= this->mCanvasHeight);
    return (x_valid && y_valid);
}

bool LFLegaliser::checkTileInCanvas(Tile &tile) const{
    bool x_valid, y_valid;
    x_valid = (tile.getLowerLeft().x >= 0) && (tile.getUpperLeft().x <= this->mCanvasWidth);
    y_valid = (tile.getLowerLeft().y >= 0) && (tile.getLowerRight().y <= this->mCanvasHeight);
    return (x_valid && y_valid);
}

Tile *LFLegaliser::getRandomTile() const{
    assert(!(fixedTesserae.empty() && softTesserae.empty()));
    
    if(!fixedTesserae.empty()){
        if(!fixedTesserae[0]->TileArr.empty()){
            return fixedTesserae[0]->TileArr[0];        
        }else{
            return fixedTesserae[0]->OverlapArr[0];        
        }
    }else{
        if(!softTesserae[0]->TileArr.empty()){
            return softTesserae[0]->TileArr[0];
        }else{
            return softTesserae[0]->OverlapArr[0];
        }
    }
}

len_t LFLegaliser::getCanvasWidth() const{
    return this->mCanvasWidth;
}

len_t LFLegaliser::getCanvasHeight() const {
    return this->mCanvasHeight;
}

void LFLegaliser::translateGlobalFloorplanning(const PPSolver &solver) {
    // You could define the I/O of this function
    // To create a soft Tessera:
    // Tessera *newTess = new Tessera(tesseraType::SOFT, "Name", 456, Cord(4,5), 3, 4);
    // softTesserae.push_back(newTess);
    // The constructor would automatically create a default tile for you.

    float aspect_ratio = (float) mCanvasHeight / mCanvasWidth;
    for ( int i = 0; i < solver.moduleNum; i++ ) {
        //std::cout << solver.modules[i]->name << ": ";
        //std::cout << solver.modules[i]->x << ", " << solver.modules[i]->y << std::endl;
        PPModule *curModule = solver.modules[i];
        if ( curModule->fixed ) {
            Tessera *newTess = new Tessera(tesseraType::HARD, curModule->name, curModule->area,
                Cord(curModule->fx, curModule->fy), curModule->fw, curModule->fh);
            fixedTesserae.push_back(newTess);
        }
        else {
            // len_t width = (len_t) std::ceil(std::sqrt(curModule->area / aspect_ratio));
            // len_t height = (len_t) std::ceil(std::sqrt(curModule->area * aspect_ratio));
            len_t width = (len_t) std::ceil(std::sqrt(curModule->area));
            len_t height = (len_t) std::ceil(std::sqrt(curModule->area));
            Tessera *newTess = new Tessera(tesseraType::SOFT, curModule->name, curModule->area,
                Cord(curModule->x - (len_t) width / 2, (len_t) curModule->y - height / 2), width, height);
            softTesserae.push_back(newTess);
        }
    }

}

void LFLegaliser::translateGlobalFloorplanning(const RGSolver &solver) {
    // You could define the I/O of this function
    // To create a soft Tessera:
    // Tessera *newTess = new Tessera(tesseraType::SOFT, "Name", 456, Cord(4,5), 3, 4);
    // softTesserae.push_back(newTess);
    // The constructor would automatically create a default tile for you.

    for ( int i = 0; i < solver.moduleNum; i++ ) {
        RGModule *curModule = solver.modules[i];
        if ( curModule->fixed ) {
            Tessera *newTess = new Tessera(tesseraType::HARD, curModule->name, curModule->area,
                Cord(curModule->x, curModule->y), curModule->width, curModule->height);
            fixedTesserae.push_back(newTess);
        }
        else {
            curModule->updateCord(mCanvasWidth, mCanvasHeight, 1.);
            Tessera *newTess = new Tessera(tesseraType::SOFT, curModule->name, curModule->area,
                Cord((len_t)curModule->x, (len_t) curModule->y), curModule->width, curModule->height);
            softTesserae.push_back(newTess);
        }
    }

}

bool hasCycle3(std::vector< std::set<len_t> > graph, std::vector<bool> visited, int curr, int parent, int depth, int nodes[3]) {
    if ( depth == 3 ) {
        for ( int neighbor : graph[curr] ) {
            // std::cout << parent << " " << curr << " " << neighbor << std::endl;
            if ( neighbor == parent )
                continue;
            if ( visited[neighbor] ) {
                nodes[2] = curr;
                return true;
            }
        }
        return false;
    }
    visited[curr] = true;
    for ( int neighbor : graph[curr] ) {
        if ( neighbor == parent )
            continue;
        if ( !visited[neighbor] ) {
            if ( hasCycle3(graph, visited, neighbor, curr, depth + 1, nodes) ) {
                nodes[depth - 1] = curr;
                return true;
            }
        }
    }
    visited[curr] = false;
    return false;
}

void LFLegaliser::detectfloorplanningOverlaps() {
    // If an overlap is detected, You should:
    // 1. Locate the overlap and crate a new Tile marking the overlap, the tile should include the spacing info and the voerlap Tessera idx
    // Tile *overlapTile = new Tile(tileType::OVERLAP, Cord(1,3), 4, 5);
    // overlapTile->OverlapFixedTesseraeIdx.pushback()....
    // overlapTile->OverlapSoftTesseraeIdx.pushback()....

    // 2. Split (both) the Tesserae into smaller tiles if it become rectlinear.
    // 3. Update (both) the Tesserae's tile list.

    //first we create an object to do the connectivity extraction
    using namespace boost::polygon::operators;

    gtl::connectivity_extraction_90<int> ce;

    std::vector<Rectangle> test_data;
    for ( Tessera *curTes : softTesserae ) {
        Cord lowerLeft = curTes->TileArr[0]->getLowerLeft();
        Cord upperRight = curTes->TileArr[0]->getUpperRight();
        test_data.push_back(Rectangle(lowerLeft.x, lowerLeft.y, upperRight.x, upperRight.y));
    }

    for ( Tessera *curTes : fixedTesserae ) {
        Cord lowerLeft = curTes->TileArr[0]->getLowerLeft();
        Cord upperRight = curTes->TileArr[0]->getUpperRight();
        test_data.push_back(Rectangle(lowerLeft.x, lowerLeft.y, upperRight.x, upperRight.y));
    }

    for ( unsigned int i = 0; i < test_data.size(); ++i ) {
        //insert returns an id starting at zero and incrementing
        //with each call
        assert(ce.insert(test_data[i]) == i);
    }
    //notice that ids returned by ce.insert happen to match
    //index into vector of inputs in this case

    //make sure the vector graph has elements for our nodes
    std::vector< std::set<len_t> > graph(test_data.size());

    //populate the graph with edge data
    ce.extract(graph);

    // 3 overlapped
    overlap3 = false;
    std::vector< std::vector<int> > overlap3Vec;
    std::vector<Tile> overlap3TileVec;
    for ( int i = 0; i < test_data.size(); i++ ) {
        std::vector<bool> visited(test_data.size(), 0);
        int nodes[3] = { -1 };
        if ( hasCycle3(graph, visited, i, -1, 1, nodes) ) {
            overlap3 = true;
            std::vector<int> overlapNodes(3);
            overlapNodes[0] = nodes[0];
            overlapNodes[1] = nodes[1];
            overlapNodes[2] = nodes[2];
            std::sort(overlapNodes.begin(), overlapNodes.end());
            bool existed = false;
            for ( std::vector<int> &o3 : overlap3Vec ) {
                if ( o3[0] == overlapNodes[0] && o3[1] == overlapNodes[1] && o3[2] == overlapNodes[2] ) {
                    existed = true;
                    break;
                }
            }
            if ( !existed ) {
                overlap3Vec.push_back(overlapNodes);
            }
        }
    }

    for ( std::vector<int> &o3 : overlap3Vec ) {
        PolygonSet intersection;
        intersection += test_data[o3[0]] & test_data[o3[1]] & test_data[o3[2]];
        Rectangle intersectBox;
        gtl::extents(intersectBox, intersection);
        len_t x = gtl::xl(intersectBox);
        len_t y = gtl::yl(intersectBox);
        len_t w = gtl::xh(intersectBox) - gtl::xl(intersectBox);
        len_t h = gtl::yh(intersectBox) - gtl::yl(intersectBox);
        Tile overlapTileRef(tileType::OVERLAP, Cord(x, y), w, h);
        overlap3TileVec.push_back(overlapTileRef);

        if ( intersection.empty() ) {
            continue;
        }

        std::cout << "3 Modules Intersection: " << intersection << std::endl;

        Tile *overlapTile = new Tile(tileType::OVERLAP, Cord(x, y), w, h);
        for ( int i : o3 ) {
            bool isSoft = i < softTesserae.size();
            int id = ( isSoft ) ? i : i - softTesserae.size();
            Tessera *curTess = ( isSoft ) ? softTesserae[id] : fixedTesserae[id];

            ( isSoft ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(id)
                : overlapTile->OverlapFixedTesseraeIdx.push_back(id);
            curTess->insertTiles(overlapTile);
        }
    }

    // 2 overlapped
    for ( int i = 0; i < test_data.size(); i++ ) {
        bool isCurSoft = i < softTesserae.size();
        int curId = ( isCurSoft ) ? i : i - softTesserae.size();
        Tessera *curTess = ( isCurSoft ) ? softTesserae[curId] : fixedTesserae[curId];
        for ( int n : graph[i] ) {
            if ( n < i )
                continue;
            bool isTarSoft = n < softTesserae.size();
            int tarId = ( isTarSoft ) ? n : n - softTesserae.size();
            Tessera *tarTess = ( isTarSoft ) ? softTesserae[tarId] : fixedTesserae[tarId];
            PolygonSet intersection;
            intersection += test_data[i] & test_data[n];
            Rectangle intersectBox;
            gtl::extents(intersectBox, intersection);
            len_t x = gtl::xl(intersectBox);
            len_t y = gtl::yl(intersectBox);
            len_t w = gtl::xh(intersectBox) - gtl::xl(intersectBox);
            len_t h = gtl::yh(intersectBox) - gtl::yl(intersectBox);
            Tile interection2Tile(tileType::OVERLAP, Cord(x, y), w, h);

            std::vector<Tile> interection3TileVec, interection2TileVec;
            for ( int j = 0; j < overlap3TileVec.size(); ++j ) {
                std::vector<int> o3 = overlap3Vec[j];
                if ( o3[0] == i || o3[1] == i || o3[2] == i ) {
                    interection3TileVec.push_back(overlap3TileVec[j]);
                }
            }
            interection2TileVec.push_back(interection2Tile);

            std::vector<Tile> cuttedTiles;

            if ( interection3TileVec.empty() ) {
                cuttedTiles.push_back(interection2Tile);
            }
            else {
                cuttedTiles = mergeCutTiles(interection2TileVec, interection3TileVec);
            }

            for ( auto &tile : cuttedTiles ) {
                Tile *overlapTile = new Tile(tileType::OVERLAP, tile.getLowerLeft(), tile.getWidth(), tile.getHeight());
                ( isCurSoft ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(curId)
                    : overlapTile->OverlapFixedTesseraeIdx.push_back(curId);

                ( isTarSoft ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(tarId)
                    : overlapTile->OverlapFixedTesseraeIdx.push_back(tarId);

                curTess->insertTiles(overlapTile);
                tarTess->insertTiles(overlapTile);
            }
        }
    }
}

bool LFLegaliser::has3overlap() {
    return overlap3;
}

void LFLegaliser::splitTesseraeOverlaps(){
    // Soft&Hard block overlap are located and split if necessary in OverlapArr of each Tessera
    // now cut rectlinear blank space of each Tessera into multiple blank tiles.

    for(Tessera *fixedTess : this->fixedTesserae){
        fixedTess->splitRectliearDueToOverlap();
    }

    for(Tessera *softTess : this->softTesserae){
        softTess->splitRectliearDueToOverlap();
    }
}

Tile *LFLegaliser::findPoint(const Cord &key) const{
    assert(key >= Cord(0, 0));
    assert(key.x < getCanvasWidth());
    assert(key.y < getCanvasHeight());

    Tile *index = getRandomTile();
    
    while(!(index->checkCordInTile(key))){
        if(!index->checkYCordInTile(key)){
            // Adjust vertical range
            if(key.y >= index->getLowerLeft().y){
                assert(index->rt != nullptr);
                index = index->rt;
            }else{
                assert(index->lb != nullptr);
                index = index->lb;
            }
        }else{
            // Vertical range correct! adjust horizontal range
            if(key.x >= index->getLowerLeft().x){
                assert(index->tr != nullptr);
                index = index->tr;
            }else{
                assert(index->bl != nullptr);
                index = index->bl;
            }
        }
    }
    
    return index;
}

Tile *LFLegaliser::findPoint(const Cord &key, Tile *initTile) const{
    assert(key >= Cord(0, 0));
    assert(key.x < getCanvasWidth());
    assert(key.y < getCanvasHeight());

    Tile *index = initTile;
    
    while(!(index->checkCordInTile(key))){
        if(!index->checkYCordInTile(key)){
            // Adjust vertical range
            if(key.y >= index->getLowerLeft().y){
                assert(index->rt != nullptr);
                index = index->rt;
            }else{
                assert(index->lb != nullptr);
                index = index->lb;
            }
        }else{
            // Vertical range correct! adjust horizontal range
            if(key.x >= index->getLowerLeft().x){
                assert(index->tr != nullptr);
                index = index->tr;
            }else{
                assert(index->bl != nullptr);
                index = index->bl;
            }
        }
    }
    
    return index;
}

void LFLegaliser::findTopNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    if(centre->rt == nullptr) return;
    Tile *n = centre->rt;
    while(n->getLowerLeft().x > centre->getLowerLeft().x){
        neighbors.push_back(n);
        n = n->bl;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findDownNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    if(centre->lb == nullptr) return;
    Tile *n = centre->lb;
    while(n->getUpperRight().x < centre->getUpperRight().x){
        neighbors.push_back(n);
        n = n->tr;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findLeftNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    if(centre->bl == nullptr) return;
    Tile *n = centre->bl;
    while(n->getUpperRight().y < centre->getUpperRight().y){
        neighbors.push_back(n);
        n = n->rt;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findRightNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    if(centre->tr == nullptr) return;
    Tile *n = centre->tr;
    // the last neighbor is the first tile encountered whose lower y cord <= lower y cord of starting tile
    while(n->getLowerLeft().y > centre->getLowerLeft().y){
        neighbors.push_back(n);
        n = n->lb;
    }
    neighbors.push_back(n);
    
}

void LFLegaliser::findAllNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const{
    findTopNeighbors(centre, neighbors);
    findDownNeighbors(centre, neighbors);
    findLeftNeighbors(centre, neighbors);
    findRightNeighbors(centre, neighbors);
}

bool LFLegaliser::searchArea(Cord lowerleft, len_t width, len_t height, Tile &target) const{

    assert(checkTesseraInCanvas(lowerleft, width, height));

    // Use point-finding algo to locate the tile containin the upperleft corner of AOI
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *currentFind = findPoint(Cord(lowerleft.x, searchRBorderHeight));
    std::cout << "Init found:" <<std::endl;

    while(currentFind->getUpperLeft().y > lowerleft.y){
        // See if the tile is solid
        if(currentFind->getType() != tileType::BLANK){
            // This is an edge of a solid tile
            target = *currentFind;
            return true;
        }else if(currentFind->getUpperRight().x < lowerleft.x + width){
            // See if the right edge within AOI, right must be a tile
            target = *(currentFind->tr);
            return true;
        }else{
            // Move down to the next tile touching the left edge of AOI
            if(currentFind->getLowerLeft().y <= 1){
                break;
            }
            currentFind = findPoint(Cord(lowerleft.x, currentFind->getLowerLeft().y -1));
        }
    }

    return false;

}

bool LFLegaliser::searchArea(Cord lowerleft, len_t width, len_t height) const{
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *currentFind = findPoint(Cord(lowerleft.x, searchRBorderHeight));
    
    while(currentFind->getUpperLeft().y > lowerleft.y){
        // See if the tile is solid
        if(currentFind->getType() != tileType::BLANK){
            // This is an edge of a solid tile
            return true;
        }else if(currentFind->getUpperRight().x < lowerleft.x + width){
            // See if the right edge within AOI, right must be a tile
            return true;
        }else{
            if(currentFind->getLowerLeft().y <= 1){
                break;
            }
            currentFind = findPoint(Cord(lowerleft.x, currentFind->getLowerLeft().y -1));
        }
    }

    return false;
}

void LFLegaliser::enumerateDirectArea(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles) const{
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *leftTouchTile = findPoint(Cord(lowerleft.x, searchRBorderHeight));

    
    while(leftTouchTile->getUpperLeft().y > lowerleft.y){
        enumerateDirectAreaRProcess(lowerleft, width, height, allTiles, leftTouchTile);
        if(leftTouchTile->getLowerLeft().y <= 1) break;
        leftTouchTile = findPoint(Cord(lowerleft.x, leftTouchTile->getLowerLeft().y -1));
    }
}

void LFLegaliser::enumerateDirectAreaRProcess(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles, Tile *targetTile) const{
    
    // R1) Enumerate the tile
    if(targetTile->getType() == tileType::BLOCK || targetTile->getType() == tileType::OVERLAP){
        allTiles.push_back(targetTile);

    }

    // R2) If the right edge of the tile is outside of the seearch area, return
    if(targetTile->getLowerRight().x >= (lowerleft.x + width)){
        return;
    }

    // R3) Use neighbor-finding algo to locate all the tiles that touch the right side of the current tile and also intersect the search area
    std::vector<Tile *> rightNeighbors;
    findRightNeighbors(targetTile, rightNeighbors);
    for(Tile *t : rightNeighbors){

        // R4) If bottom left corner of the neighbor touches the current tile
        bool R4 = targetTile->checkTRLLTouch(t);
        // R5) If the bottom edge ofthe search area cuts both the urrent tile and the neighbor
        bool R5 = (targetTile->cutHeight(lowerleft.y)) && (t->cutHeight(lowerleft.y));

        if(R4 || R5){
            enumerateDirectAreaRProcess(lowerleft, width, height, allTiles, t);
        }
    }

}

void LFLegaliser::insertFirstTile(Tile &newTile){
    assert(this->checkTileInCanvas(newTile));
    // cut the canvas into four parts: above, below, left, rifht
    
    Tile *tdown, *tup, *tleft, *tright;
    bool hasDownTile = (newTile.getLowerLeft().y != 0);
    bool hasUpTile = (newTile.getUpperRight().y != this->mCanvasHeight);
    bool hasLeftTile = (newTile.getLowerLeft().x != 0);
    bool hasRightTile = (newTile.getLowerRight().x != this->mCanvasWidth);


    if(hasDownTile){
        tdown = new Tile(tileType::BLANK, Cord(0,0),
                            this->mCanvasWidth, newTile.getLowerLeft().y);
        newTile.lb = tdown;
    }

    if(hasUpTile){
        tup = new Tile(tileType::BLANK, Cord(0,newTile.getUpperRight().y), 
                            this->mCanvasWidth, (this->mCanvasHeight - newTile.getUpperRight().y));
        newTile.rt = tup;
    }

    if(hasLeftTile){
        tleft = new Tile(tileType::BLANK, Cord(0, newTile.getLowerLeft().y),
                            newTile.getLowerLeft().x, (newTile.getUpperLeft().y - newTile.getLowerLeft().y));
        newTile.bl = tleft;
        tleft->tr = &newTile;

        
        if(hasDownTile) tleft->lb = tdown;
        if(hasUpTile) tleft->rt = tup;
        
        if(hasUpTile) tup->lb = tleft;
    }else{
        if(hasUpTile) tup->lb = &newTile;
    }

    if(hasRightTile){
        tright = new Tile(tileType::BLANK, newTile.getLowerRight(), 
                            (this->mCanvasWidth - newTile.getUpperRight().x), (newTile.getUpperLeft().y - newTile.getLowerLeft().y));
        newTile.tr = tright;
        tright->bl = &newTile;

        if(hasDownTile) tright->lb = tdown;
        if(hasUpTile) tright->rt = tup;

        if(hasDownTile) tdown->rt = tright;
    }else{
        if(hasDownTile) tdown->rt = &newTile;
    }

}

void LFLegaliser::insertTile(Tile &tile){
    assert(checkTesseraInCanvas(tile.getLowerLeft(), tile.getWidth(), tile.getHeight()));
    assert(!searchArea(tile.getLowerLeft(), tile.getWidth(), tile.getHeight()));

    /* STEP 1) Find the space Tile containing the top edge of the aera to be occupied, process */
    bool tileTouchesSky = (tile.getUpperRight().y == getCanvasHeight());
    bool cleanTopCut = true;
    Tile *origTop;
    
    if(!tileTouchesSky){
        origTop = findPoint(tile.getUpperLeft());
        cleanTopCut = (origTop->getLowerLeft().y == tile.getUpperRight().y);
    }
    
    if((!tileTouchesSky)&&(!cleanTopCut)){

        
        Tile *newDown = new Tile(tileType::BLANK, origTop->getLowerLeft(),origTop->getWidth(), (tile.getUpperLeft().y - origTop->getLowerLeft().y));
        newDown->rt = origTop;
        newDown->lb = origTop->lb;
        newDown->bl = origTop->bl;

        // manipulate neighbors around the split tiles

        // change lower-neighbors' rt pointer to newly created tile
        std::vector <Tile *> origDownNeighbors;
        findDownNeighbors(origTop, origDownNeighbors);
        for(Tile *t : origDownNeighbors){
            if(t->rt == origTop){
                t->rt = newDown;
            }
        }
        // 1. find the correct tr for newDown
        // 2. change right neighbors to point their bl to the correct tile (one of the split)
        std::vector <Tile *> origRightNeighbors;
        findRightNeighbors(origTop, origRightNeighbors);
        
        bool rightModified = false;
        for(int i = 0; i < origRightNeighbors.size(); ++i){
            if(origRightNeighbors[i]->getLowerLeft().y < newDown->getUpperRight().y){
                if(!rightModified){
                    rightModified = true;
                    newDown->tr = origRightNeighbors[i];
                }
                // bug fix: change "tile" -> "newDown", add break statement to terminate unecessary serarch early
                // 08/06/2023
                if(origRightNeighbors[i]->getLowerLeft().y >= newDown->getLowerLeft().y){
                    origRightNeighbors[i]->bl = newDown;
                }else{
                    break;
                }
                
            }
        }

        // change Left neighbors to point their tr to the correct tiles
        std::vector <Tile *> origLeftNeighbors;
        findLeftNeighbors(origTop, origLeftNeighbors);

        bool leftModified = false;
        for(int i = 0; i < origLeftNeighbors.size(); ++i){
            if(origLeftNeighbors[i]->getUpperLeft().y > newDown->getUpperLeft().y){
                if(!leftModified){
                    leftModified = true;
                    origTop->bl = origLeftNeighbors[i];
                    // add break statement to terminate unecessary serarch early
                    break;
                }
            }else{
                origLeftNeighbors[i]->tr = newDown;
            }
        }
        len_t oUpperLeft = origTop->getUpperLeft().y;
        origTop->setCord(Cord(origTop->getLowerLeft().x, tile.getUpperLeft().y));
        origTop->setHeight(oUpperLeft - tile.getUpperLeft().y);
        origTop->lb = newDown;

    }
    
    /* STEP 2) Find the space Tile containing the bottom edge of the aera to be occupied, process */
    
    bool tileTouchesGround = (tile.getLowerLeft().y == 0);
    bool cleanBottomCut = true;
    Tile *origBottom;
    if(!tileTouchesGround){
        origBottom = findPoint(tile.getLowerLeft() - Cord(0, 1));
        cleanBottomCut = (origBottom->getUpperRight().y == tile.getLowerLeft().y);
    }

    if((!tileTouchesGround) && (!cleanBottomCut)){
        
        Tile *newUp = new Tile(tileType::BLANK, Cord(origBottom->getLowerLeft().x, tile.getLowerLeft().y)
                                , origBottom->getWidth(), (origBottom->getUpperLeft().y - tile.getLowerLeft().y));         
        
        newUp->rt = origBottom->rt;
        newUp->lb = origBottom;
        newUp->tr = origBottom->tr;

        // manipulate neighbors around the split tiles

        // change the upper-neighbors' lb pointer to newly created tile
        std::vector <Tile *> origUpNeighbors;
        findTopNeighbors(origBottom, origUpNeighbors);
        for(Tile *t : origUpNeighbors){
            if(t->lb == origBottom){
                t->lb = newUp;
            }
        }

        // change right neighbors to point their bl to the correct tle (one of the split)
        std::vector <Tile *> origRightNeighbors;
        findRightNeighbors(origBottom, origRightNeighbors);


        bool rightModified = false;
        for(int i = 0; i < origRightNeighbors.size(); ++i){
            if(origRightNeighbors[i]->getLowerLeft().y < newUp->getLowerLeft().y){
                // bug found, "!" rightModified
                // 8/5/2023
                if(!rightModified){
                    rightModified = true;
                    origBottom->tr = origRightNeighbors[i];
                    // add break statement to terminate unecessary serarch early
                    break;
                }
            }else{
                origRightNeighbors[i]->bl = newUp;
            }
        }
        
        // change left neighbors to point their tr to the correct tiles
        std::vector <Tile *> origLeftNeighbors;
        findLeftNeighbors(origBottom, origLeftNeighbors);

        bool leftModified = false;
        for(int i = 0; i < origLeftNeighbors.size(); ++i){
            if(origLeftNeighbors[i]->getUpperLeft().y > newUp->getLowerLeft().y){
                if(!leftModified){
                    leftModified = true;
                    newUp->bl = origLeftNeighbors[i];
                }
                // bug fix: change "tile" -> "newUp", add break statement to terminate unecessary serarch early
                // 08/06/2023
                if(origLeftNeighbors[i]->getUpperLeft().y <= newUp->getUpperLeft().y){
                    origLeftNeighbors[i]->tr = newUp;
                }else{
                    break;
                }
            }
        }

        // origBottom->setCord(Cord(origBottom->getLowerLeft().x, tile.getLowerLeft().y));
        origBottom->setHeight(tile.getLowerLeft().y - origBottom->getLowerLeft().y);
        origBottom->rt = newUp;
        
    }
    

    // STEP 3) The area of the new tile is traversed from top to bottom, splitting and joining space tiles on either side
    //        and pointing their stiches at the new tile

    // locate the topmost blank-tile, split in 3 pieces, left, mid and right

    Tile *splitTile = findPoint(Cord(tile.getUpperLeft() - Cord(0, 1)));
    len_t findTileY = splitTile->getLowerLeft().y;
    Tile *oldsplitTile;

    // Merge helping indexes
    len_t leftMergeWidth = 0, rightMergeWidth = 0;

    bool topMostMerge = true;
    while(true){

        // split into three pieces
        len_t blankLeftBorder = splitTile->getLowerLeft().x;
        len_t tileLeftBorder = tile.getLowerLeft().x;
        len_t tileRightBorder = tile.getLowerRight().x;
        len_t blankRightBorder = splitTile->getLowerRight().x;

        // The middle piece (must have)
        // This should change to tile.type
        Tile *newMid = new Tile(tileType::BLANK, Cord(tileLeftBorder, splitTile->getLowerLeft().y), tile.getWidth(), splitTile->getHeight());
        newMid->bl = splitTile->bl;
        newMid->tr = splitTile->tr;

        std::vector<Tile *> topNeighbors;
        findTopNeighbors(splitTile, topNeighbors);
        std::vector<Tile *>bottomNeighbors;
        findDownNeighbors(splitTile, bottomNeighbors);
        std::vector<Tile *>leftNeighbors;
        findLeftNeighbors(splitTile, leftNeighbors);
        std::vector<Tile *>rightNeighbors;
        findRightNeighbors(splitTile, rightNeighbors);

        // split the left piece if necessary, maintain tr, bl pointer integrity
        bool leftSplitNecessary = (blankLeftBorder != tileLeftBorder);
        if(leftSplitNecessary){
            Tile *newLeft = new Tile(tileType::BLANK, splitTile->getLowerLeft(),(tileLeftBorder - blankLeftBorder) ,splitTile->getHeight());
            // visualiseAddMark(newLeft);
            newLeft->tr = newMid;
            newLeft->bl = splitTile->bl;

            newMid->bl = newLeft;

            // maintain rt, lb pointers of the newtile and top-Neighbors of new Tiles
            bool rtModified = false;
            for(int i = 0; i < topNeighbors.size(); ++i){
                if(topNeighbors[i]->getLowerLeft().x < tileLeftBorder){
                    if(!rtModified){
                        rtModified = true;
                        newLeft->rt = topNeighbors[i];
                    }
                    if(topNeighbors[i]->getLowerLeft().x >= blankLeftBorder){
                        topNeighbors[i]->lb = newLeft;
                    }else{
                        break;
                    }
                }
            }
            
            // maintain rt, lb pointers of the newtile and bottom-Neighbors of new Tiles
            bool lbModified = false;
            for(int i = 0; i < bottomNeighbors.size(); ++i){
                if(bottomNeighbors[i]->getLowerLeft().x < tileLeftBorder){
                    if(!lbModified){
                        lbModified = true;
                        newLeft->lb = bottomNeighbors[i];
                    }
                    if(bottomNeighbors[i]->getLowerRight().x <= tileLeftBorder){
                        bottomNeighbors[i]->rt = newLeft;
                    }
                    
                }else{
                    break;
                }
            }

            // also change tr pointers of left neighbors to the newly created right tile
            for(int i = 0; i < leftNeighbors.size(); ++i){
                if(leftNeighbors[i]->tr == splitTile){
                    leftNeighbors[i]->tr = newLeft;
                }
            }

        }else{
            // change the tr pointers of the left neighbors to newMid
            for(int i = 0; i < leftNeighbors.size(); ++i){
                if(leftNeighbors[i]->tr == splitTile){
                    leftNeighbors[i]->tr = newMid;
                }
            }
        }

        // split the right piece if necessary, maintain tr, bl pointer integrity
        bool rightSplitNecessary = (tileRightBorder != blankRightBorder);
        if(rightSplitNecessary){
            Tile *newRight = new Tile(tileType::BLANK, newMid->getLowerRight(),(blankRightBorder- tileRightBorder) ,newMid->getHeight());
            // visualiseAddMark(newRight);
            newRight->tr = splitTile->tr;
            newRight->bl = newMid;

            newMid->tr = newRight;

            //maintain rt, lb poointers of the newtile and top-Neighbors of new Tiles
            bool rtModified = false;
            for(int i = 0; i < topNeighbors.size(); ++i){
                if(topNeighbors[i]->getLowerRight().x > tileRightBorder){
                    if(!rtModified){
                        rtModified = true;
                        newRight->rt = topNeighbors[i];
                    }
                    if(topNeighbors[i]->getLowerLeft().x >= tileRightBorder){
                        topNeighbors[i]->lb = newRight;
                    }
                }else{
                    break;
                }
            }

            //maintain rt, lb poointers of the newtile and bottom-Neighbors of new Tiles

            bool lbModified = false;
            for(int i = 0 ; i < bottomNeighbors.size(); ++i){
                if(bottomNeighbors[i]->getLowerRight().x > tileRightBorder){
                    if(!lbModified){
                        lbModified = true;
                        newRight->lb = bottomNeighbors[i];
                    }
                    if(bottomNeighbors[i]->getLowerRight().x <= blankRightBorder){
                        bottomNeighbors[i]->rt = newRight;
                    }else{
                        break;
                    }
                }
            }

            // also change bl pointers of right neighbors to the newly created left tile
            for(int i = 0; i < rightNeighbors.size(); ++i){
                if(rightNeighbors[i]->bl == splitTile){
                    rightNeighbors[i]->bl = newRight;
                }
            }
            
        }else{
            // change bl pointers of right neighbors back to newMid
            for(int i = 0; i < rightNeighbors.size(); ++i){
                if(rightNeighbors[i]->bl == splitTile){
                    rightNeighbors[i]->bl = newMid;
                }
            }
        }

        // maintain rt & lb pointers integrity for newMid
        bool rtModified = false;
        for(int i = 0; i < topNeighbors.size(); ++i){
            if(topNeighbors[i]->getLowerLeft().x < tileRightBorder){
                if(!rtModified){
                    rtModified = true;
                    newMid->rt = topNeighbors[i];
                }
                if(topNeighbors[i]->getLowerLeft().x >= tileLeftBorder){
                    topNeighbors[i]->lb = newMid;
                }else{
                    break;
                }
            }
        }

        bool lbModified = false;
        for(int i = 0; i < bottomNeighbors.size(); ++i){
            if(bottomNeighbors[i]->getLowerRight().x > tileLeftBorder){
                if(!lbModified){
                    lbModified = true;
                    newMid->lb = bottomNeighbors[i];
                }
                if(bottomNeighbors[i]->getLowerRight().x <= tileRightBorder){
                    bottomNeighbors[i]->rt = newMid;
                }else{
                    break;
                }
            }
        }


        // Start Merging process
        
        // Merge the left blocks if necessary
        bool initTopLeftMerge = false;
        if(topMostMerge){
            Tile *initTopLeftUp, *initTopLeftDown;
            if(leftSplitNecessary){
                
                initTopLeftDown = newMid->bl;
                if(initTopLeftDown->rt != nullptr){
                    initTopLeftUp= initTopLeftDown->rt;
                    bool sameWidth = (initTopLeftUp->getWidth() == initTopLeftDown->getWidth());
                    bool xAligned = (initTopLeftUp->getLowerLeft().x == initTopLeftDown->getLowerLeft().x);
                    if((initTopLeftUp->getType() == tileType::BLANK) &&  sameWidth && xAligned){
                        initTopLeftMerge = true;
                    }
                }
            }
        }

        bool leftNeedsMerge = ((leftMergeWidth != 0) && (leftMergeWidth == (tileLeftBorder - blankLeftBorder))) || initTopLeftMerge;
        if(leftNeedsMerge){
            Tile *mergeUp = newMid->bl->rt;
            Tile *mergeDown = newMid->bl;

            std::vector<Tile *> mergedownLeftNeighbors;
            findLeftNeighbors(mergeDown, mergedownLeftNeighbors);
            for(int i = 0; i < mergedownLeftNeighbors.size(); ++i){
                if(mergedownLeftNeighbors[i]->tr == mergeDown){
                    mergedownLeftNeighbors[i]->tr = mergeUp;
                }
            }

            std::vector<Tile *> mergedownDownNeighbors;
            findDownNeighbors(mergeDown, mergedownDownNeighbors);
            for(int i = 0; i < mergedownDownNeighbors.size(); ++i){
                if(mergedownDownNeighbors[i]->rt == mergeDown){
                    mergedownDownNeighbors[i]->rt = mergeUp;
                }
            }
            std::vector<Tile *> mergedownRightNeighbors;
            findRightNeighbors(mergeDown, mergedownRightNeighbors);
            for(int i = 0; i < mergedownRightNeighbors.size(); ++i){
                if(mergedownRightNeighbors[i]->bl == mergeDown){
                    mergedownRightNeighbors[i]->bl = mergeUp;
                }
            }
            
            mergeUp->bl = mergeDown->bl;
            mergeUp->lb = mergeDown->lb;
            
            mergeUp->setLowerLeft(mergeDown->getLowerLeft());
            mergeUp->setHeight(mergeUp->getHeight() + mergeDown->getHeight());
            
            delete(mergeDown);
        }
        // update merge width for latter blocks
        leftMergeWidth = tileLeftBorder - blankLeftBorder;


        // Merge the right blocks if possible
        bool initTopRightMerge = false;
        if(topMostMerge){
            Tile *initTopRightUp, *initTopRightDown;
            if(rightSplitNecessary){
                initTopRightDown = newMid->tr;
                if(initTopRightDown->rt != nullptr){
                    initTopRightUp = initTopRightDown->rt;
                    bool sameWidth = (initTopRightUp->getWidth() == initTopRightDown->getWidth());
                    bool xAligned = (initTopRightUp->getLowerLeft().x == initTopRightDown->getLowerLeft().x);
                    if((initTopRightUp->getType() == tileType::BLANK) && (sameWidth) && (xAligned)){
                        initTopRightMerge = true;
                    }
                }
            }
        }
        
        bool rightNeedsMerge = ((rightMergeWidth != 0 ) && (rightMergeWidth == (blankRightBorder - tileRightBorder))) || initTopRightMerge;        
        if(rightNeedsMerge){
            Tile *mergeUp = newMid->tr->rt;
            Tile *mergeDown = newMid->tr;

            std::vector<Tile *> mergedownLeftNeighbors;
            findLeftNeighbors(mergeDown, mergedownLeftNeighbors);
            for(int i = 0; i < mergedownLeftNeighbors.size(); ++i){
                if(mergedownLeftNeighbors[i]->tr == mergeDown){
                    mergedownLeftNeighbors[i]->tr = mergeUp;
                }
            }

            std::vector<Tile *> mergedownDownNeighbors;
            findDownNeighbors(mergeDown, mergedownDownNeighbors);
            for(int i = 0; i < mergedownDownNeighbors.size(); ++i){
                if(mergedownDownNeighbors[i]->rt == mergeDown){
                    mergedownDownNeighbors[i]->rt = mergeUp;
                }
            }
            std::vector<Tile *> mergedownRightNeighbors;
            findRightNeighbors(mergeDown, mergedownRightNeighbors);
            for(int i = 0; i < mergedownRightNeighbors.size(); ++i){
                if(mergedownRightNeighbors[i]->bl == mergeDown){
                    mergedownRightNeighbors[i]->bl = mergeUp;
                }
            }
            mergeUp->bl = mergeDown->bl;
            mergeUp->lb = mergeDown->lb;
            
            mergeUp->setLowerLeft(mergeDown->getLowerLeft());
            mergeUp->setHeight(mergeUp->getHeight() + mergeDown->getHeight());
            
            delete(mergeDown);
        }
        // update right merge width for latter blocks
        rightMergeWidth = blankRightBorder - tileRightBorder;


        // Finally, merge the middle tile, it MUST merge after the first time
        
        if(!topMostMerge){
            Tile *mergeUp = newMid->rt;
            Tile *mergeDown = newMid;

            std::vector<Tile *> mergedownLeftNeighbors;
            findLeftNeighbors(mergeDown, mergedownLeftNeighbors);
            for(int i = 0; i < mergedownLeftNeighbors.size(); ++i){
                if(mergedownLeftNeighbors[i]->tr == mergeDown){
                    mergedownLeftNeighbors[i]->tr = mergeUp;
                }
            }

            std::vector<Tile *> mergedownDownNeighbors;
            findDownNeighbors(mergeDown, mergedownDownNeighbors);
            for(int i = 0; i < mergedownDownNeighbors.size(); ++i){
                if(mergedownDownNeighbors[i]->rt == mergeDown){
                    mergedownDownNeighbors[i]->rt = mergeUp;
                }
            }
            std::vector<Tile *> mergedownRightNeighbors;
            findRightNeighbors(mergeDown, mergedownRightNeighbors);
            for(int i = 0; i < mergedownRightNeighbors.size(); ++i){
                if(mergedownRightNeighbors[i]->bl == mergeDown){
                    mergedownRightNeighbors[i]->bl = mergeUp;
                }
            }
            mergeUp->bl = mergeDown->bl;
            mergeUp->lb = mergeDown->lb;
            
            mergeUp->setLowerLeft(mergeDown->getLowerLeft());
            mergeUp->setHeight(mergeUp->getHeight() + mergeDown->getHeight());
            
            delete(mergeDown);
            // link newMid back
            newMid = mergeUp;
        }

        oldsplitTile = splitTile;
        if(findTileY == tile.getLowerLeft().y){
            // Merging the bottom most split blank tile and the blank tile below if necessary

            // Detect & Merging left bottom & the blank below
            bool lastDownLeftMerge = false;
            Tile *lastBotLeftUp, *lastBotLeftDown;
            if(leftSplitNecessary){
                lastBotLeftUp = newMid->bl;
                if(lastBotLeftUp->lb != nullptr){
                    lastBotLeftDown = lastBotLeftUp->lb;
                    bool sameWidth = (lastBotLeftUp->getWidth() == lastBotLeftDown->getWidth());
                    bool xAligned = (lastBotLeftUp->getLowerLeft().x == lastBotLeftDown->getLowerLeft().x);
                    if(lastBotLeftDown->getType() == tileType::BLANK && sameWidth && xAligned){
                        lastDownLeftMerge = true;
                    }
                }
            }

            if(lastDownLeftMerge){
                std::vector<Tile *> mergedownLeftNeighbors;
                findLeftNeighbors(lastBotLeftDown, mergedownLeftNeighbors);
                for(int i = 0; i < mergedownLeftNeighbors.size(); ++i){
                    if(mergedownLeftNeighbors[i]->tr == lastBotLeftDown){
                        mergedownLeftNeighbors[i]->tr = lastBotLeftUp;
                    }
                }

                std::vector<Tile *> mergedownDownNeighbors;
                findDownNeighbors(lastBotLeftDown, mergedownDownNeighbors);
                for(int i = 0; i < mergedownDownNeighbors.size(); ++i){
                    if(mergedownDownNeighbors[i]->rt == lastBotLeftDown){
                        mergedownDownNeighbors[i]->rt = lastBotLeftUp;
                    }
                }
                std::vector<Tile *> mergedownRightNeighbors;
                findRightNeighbors(lastBotLeftDown, mergedownRightNeighbors);
                for(int i = 0; i < mergedownRightNeighbors.size(); ++i){
                    if(mergedownRightNeighbors[i]->bl == lastBotLeftDown){
                        mergedownRightNeighbors[i]->bl = lastBotLeftUp;
                    }
                }

                lastBotLeftUp->bl = lastBotLeftDown->bl;
                lastBotLeftUp->lb = lastBotLeftDown->lb;

                lastBotLeftUp->setLowerLeft(lastBotLeftDown->getLowerLeft());
                lastBotLeftUp->setHeight(lastBotLeftUp->getHeight() + lastBotLeftDown->getHeight());

                delete(lastBotLeftDown);
            }

            
            // Detect & Merging right bottom & the blank below            

            bool lastDownRightmerge = false;
            Tile *lastBotRightUp, *lastBotRightDown;
            if(rightSplitNecessary){
                lastBotRightUp = newMid->tr;
                if(lastBotRightUp->lb != nullptr){
                    lastBotRightDown = lastBotRightUp->lb;
                    bool sameWidth = (lastBotRightUp->getWidth() == lastBotRightDown->getWidth());
                    bool xAligned = (lastBotRightUp->getLowerLeft().x == lastBotRightDown->getLowerLeft().x);
                    if(lastBotRightDown->getType() == tileType::BLANK && sameWidth && xAligned){
                        lastDownRightmerge = true;
                    }

                }
            }

            if(lastDownRightmerge){
                std::vector<Tile *> mergedownLeftNeighbors;
                findLeftNeighbors(lastBotRightDown, mergedownLeftNeighbors);
                for(int i = 0; i < mergedownLeftNeighbors.size(); ++i){
                    if(mergedownLeftNeighbors[i]->tr == lastBotRightDown){
                        mergedownLeftNeighbors[i]->tr = lastBotRightUp;
                    }
                }

                std::vector<Tile *> mergedownDownNeighbors;
                findDownNeighbors(lastBotRightDown, mergedownDownNeighbors);
                for(int i = 0; i < mergedownDownNeighbors.size(); ++i){
                    if(mergedownDownNeighbors[i]->rt == lastBotRightDown){
                        mergedownDownNeighbors[i]->rt = lastBotRightUp;
                    }
                }
                std::vector<Tile *> mergedownRightNeighbors;
                findRightNeighbors(lastBotRightDown, mergedownRightNeighbors);
                for(int i = 0; i < mergedownRightNeighbors.size(); ++i){
                    if(mergedownRightNeighbors[i]->bl == lastBotRightDown){
                        mergedownRightNeighbors[i]->bl = lastBotRightUp;
                    }
                }
                lastBotRightUp->bl = lastBotRightDown->bl;
                lastBotRightUp->lb = lastBotRightDown->lb;
                
                lastBotRightUp->setLowerLeft(lastBotRightDown->getLowerLeft());
                lastBotRightUp->setHeight(lastBotRightUp->getHeight() + lastBotRightDown->getHeight());
                
                delete(lastBotRightDown);
            }
            
            // substitute the middle tile with the input "tile"
            tile.rt = newMid->rt;
            tile.tr = newMid->tr;
            tile.bl = newMid->bl;
            tile.lb = newMid->lb;
            
            // relink the neighbors
            std::vector <Tile *> midUpNeighbors;
            findTopNeighbors(newMid, midUpNeighbors);
            for(int i = 0; i < midUpNeighbors.size(); ++i){
                if(midUpNeighbors[i]->lb == newMid){
                    midUpNeighbors[i]->lb = &tile;
                }
            }

            std::vector <Tile *> midDownNeighbors;
            findDownNeighbors(newMid, midDownNeighbors);
            for(int i = 0; i < midDownNeighbors.size(); ++i){
                if(midDownNeighbors[i]->rt == newMid){
                    midDownNeighbors[i]->rt = &tile;
                }
            }

            std::vector <Tile *> midLeftNeighbors;
            findLeftNeighbors(newMid, midLeftNeighbors);

            for(int i = 0; i < midLeftNeighbors.size(); ++i){

                if(midLeftNeighbors[i]->tr == newMid){
                    midLeftNeighbors[i]->tr = &tile;
                }
            }

            std::vector <Tile *> midRightNeighbors;
            findRightNeighbors(newMid, midRightNeighbors);
            for(int i = 0; i < midRightNeighbors.size(); ++i){
                if(midRightNeighbors[i]->bl == newMid){
                    midRightNeighbors[i]->bl = &tile;
                }
            }

            delete(newMid);
            delete(oldsplitTile);

            break;
        }else{
            splitTile = findPoint(Cord(tile.getLowerLeft().x, findTileY) - Cord(0,1));
            findTileY= splitTile->getLowerLeft().y;
            delete(oldsplitTile);
        }

        // mark this is not the top most merge
        topMostMerge = false;
    }
    
}

void LFLegaliser::visualiseArtpiece(const std::string outputFileName, bool checkBlankTile) {

    std::cout << "print to file..."<< outputFileName <<std::endl;

    std::ofstream ofs(outputFileName);
    ofs << "BLOCK " << fixedTesserae.size() + softTesserae.size() << std::endl;
    ofs << this->mCanvasWidth << " " << this->mCanvasHeight << std::endl;

    if(fixedTesserae.size() == 0 && softTesserae.size() == 0){
        //there is no blocks
        ofs.close();
        return;
    }

    for(Tessera *tess : softTesserae){
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " " << "SOFT_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for(Tile *t : tess->TileArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for(Tile *t : tess->OverlapArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" <<std::endl;
        }
    }

    for(Tessera *tess : fixedTesserae){
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " " << "HARD_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for(Tile *t : tess->TileArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for(Tile *t : tess->OverlapArr){
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" <<std::endl;
        }
    }

    // DFS Traverse through all balnk tiles

    // Bug found, 8/1, 2023
    // Warning!! At this stage, no Empty tiles are created, DFS would cause error to the markings of tiles.

    std::vector <Cord> record;

    if(checkBlankTile){
        if(fixedTesserae.size() !=0 ){
            if(this->fixedTesserae[0]->TileArr.size() != 0){
                traverseBlank(ofs, *(this->fixedTesserae[0]->TileArr[0]), record);
            }else{
                traverseBlank(ofs, *(this->fixedTesserae[0]->OverlapArr[0]), record);
            }
        }else{
            if(softTesserae.size() != 0){
                traverseBlank(ofs, *(this->softTesserae[0]->TileArr[0]), record);
            }else{
                traverseBlank(ofs, *(this->softTesserae[0]->OverlapArr[0]), record);
            }
        }
    }
    ofs << "CONNECTION 0" << std::endl;
    
    // print all the marked tiles
    for(Tile *t : this->mMarkedTiles){
        ofs << t->getLowerLeft().x <<" "<< t->getLowerLeft().y <<" "<< t->getWidth() <<" "<< t->getHeight() << " MARKED" << std::endl;
    }

    ofs.close();
}

void LFLegaliser::traverseBlank(std::ofstream &ofs,  Tile &t, std::vector <Cord> &record) {

    record.push_back(t.getLowerLeft());
    
    if(t.getType() == tileType::BLANK){

        ofs << t.getLowerLeft().x << " " << t.getLowerLeft().y << " ";
        ofs << t.getWidth() << " " << t.getHeight() << " ";
        ofs << "BLANK_TILE" << std::endl;
    }

    if(t.rt != nullptr){        
        if(!checkVectorInclude(record, t.rt->getLowerLeft())){
            traverseBlank(ofs, *(t.rt), record);
        }
    }

    if(t.lb != nullptr){
        if(!checkVectorInclude(record, t.lb->getLowerLeft())){
            traverseBlank(ofs, *(t.lb), record);
        }
    }

    if(t.bl != nullptr){
        if(!checkVectorInclude(record, t.bl->getLowerLeft())){
            traverseBlank(ofs, *(t.bl), record);
        }
    }

    if(t.tr != nullptr){
        if(!checkVectorInclude(record,t.tr->getLowerLeft())){
            traverseBlank(ofs, *(t.tr), record);
        }
    }
    
    return;
}

void LFLegaliser::visualiseAddMark(Tile * markTile){
    this->mMarkedTiles.push_back(markTile);
}

void LFLegaliser::visualiseRemoveAllmark(){
    this->mMarkedTiles.clear();
}


void LFLegaliser::arrangeTesseraetoCanvas(){

    std::vector <Cord> record;
    
    std::cout << "Painting Fixed Tessera to Canvas:" << std::endl;
    for(Tessera *tess : this->fixedTesserae){
        std::cout << tess->getName()<<": Tiles->" << tess->TileArr.size() << ", Overlaps->" << tess->OverlapArr.size() << std::endl;
        
        for(Tile *tile : tess->TileArr){
            assert(!checkVectorInclude(record, tile->getLowerLeft()));

            if(record.empty()) insertFirstTile(*tile);
            else insertTile(*tile);

            record.push_back(tile->getLowerLeft());
        }
        
        for(Tile *tile : tess->OverlapArr){
            // for overlap tiles, only push when it's never met
            if(!checkVectorInclude(record, tile->getLowerLeft()) && (tile->getWidth() != 0) && (tile->getHeight() != 0)){
                
                if(record.empty()) insertFirstTile(*tile);
                else insertTile(*tile);
                
                record.push_back(tile->getLowerLeft());
            }
        }
    }

    // visualiseDebug("outputs/debug.txt");
    std::cout << "Painting Soft Tessera to Canvas:" << std::endl;
    Tessera *tess;
    for(int i = 0; i < this->softTesserae.size(); ++i){
        tess = this->softTesserae[i];

        std::cout << tess->getName()<<": Tiles->" << tess->TileArr.size() << ", Overlaps->" << tess->OverlapArr.size() << std::endl;
        
        Tile *tile;
        for(int j = 0; j < tess->TileArr.size(); ++j){
            tile = tess->TileArr[j];
            assert(!checkVectorInclude(record, tile->getLowerLeft()));

            if(record.empty()) insertFirstTile(*tile);
            else insertTile(*tile);

            record.push_back(tile->getLowerLeft());
        }
        for(int j = 0; j < tess->OverlapArr.size(); ++j){
            // for overlap tiles, only push when it's never met
            tile = tess->OverlapArr[j];
            if(!checkVectorInclude(record, tile->getLowerLeft()) && (tile->getWidth() != 0) && (tile->getHeight() != 0)){
                if(record.empty()) insertFirstTile(*tile);
                else insertTile(*tile);

                record.push_back(tile->getLowerLeft());
            }
        }

    }
}

void LFLegaliser::visualiseDebug(const std::string outputFileName){

    std::vector <Cord> record;
    std::cout << "print DEBUGS to file..."<< outputFileName <<std::endl;

    std::ofstream ofs(outputFileName);
    ofs << this->mCanvasWidth << " " << this->mCanvasHeight << std::endl;

    if(fixedTesserae.size() == 0 && softTesserae.size() == 0){
        //there is no blocks
        ofs.close();
        return;
    }

    if(fixedTesserae.size() !=0 ){
        if(this->fixedTesserae[0]->TileArr.size() != 0){
            visualiseDebugDFS(ofs, *(this->fixedTesserae[0]->TileArr[0]), record);
        }else{
            visualiseDebugDFS(ofs, *(this->fixedTesserae[0]->OverlapArr[0]), record);
        }
    }else{
        if(softTesserae.size() != 0){
            visualiseDebugDFS(ofs, *(this->softTesserae[0]->TileArr[0]), record);
        }else{
            visualiseDebugDFS(ofs, *(this->softTesserae[0]->OverlapArr[0]), record);
        }
    }
    ofs.close();

}

void LFLegaliser::visualiseDebugDFS(std::ofstream &ofs, Tile &t, std::vector <Cord> &record){
    
    record.push_back(t.getLowerLeft());
    
    ofs << t.getLowerLeft().x << " " << t.getLowerLeft().y << " " << t.getWidth() << " " << t.getHeight() << " ";
    if(t.getType() == tileType::BLOCK){
        ofs << "BLOCK";
    }else if(t.getType() == tileType::OVERLAP){
        ofs << "OVERLAP";
    }else if(t.getType() == tileType::BLANK){
        ofs << "BLANK_TILE";
    }else{
        ofs << "ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRROOOOOOOOOOOORRR";
    }
    ofs << std::endl;
    t.show(ofs);
    t.showLink(ofs);

    if(t.rt != nullptr){
        if(!checkVectorInclude(record,t.rt->getLowerLeft())){
            visualiseDebugDFS(ofs, *(t.rt), record);
        }
    }

    if(t.lb != nullptr){
        if(!checkVectorInclude(record,t.lb->getLowerLeft())){
            visualiseDebugDFS(ofs, *(t.lb), record);
        }
    }

    if(t.bl != nullptr){
        if(!checkVectorInclude(record,t.bl->getLowerLeft())){
            visualiseDebugDFS(ofs, *(t.bl), record);
        }
    }

    if(t.tr != nullptr){
        if(!checkVectorInclude(record,t.tr->getLowerLeft())){
            visualiseDebugDFS(ofs, *(t.tr), record);
        }
    }
    
    return;
}

void LFLegaliser::detectCombinableBlanks(std::vector <std::pair<Tile *, Tile *>> &candidateTile){
    
    std::vector <Cord> record;

    if(fixedTesserae.size() !=0 ){
        if(this->fixedTesserae[0]->TileArr.size() != 0){
            detectCombinableBlanksDFS(candidateTile, *(this->fixedTesserae[0]->TileArr[0]), record);
        }else{
            detectCombinableBlanksDFS(candidateTile, *(this->fixedTesserae[0]->OverlapArr[0]), record);
        }
    }else{
        if(softTesserae.size() != 0){
            detectCombinableBlanksDFS(candidateTile, *(this->softTesserae[0]->TileArr[0]), record);
        }else{
            detectCombinableBlanksDFS(candidateTile, *(this->softTesserae[0]->OverlapArr[0]), record);
        }
    }

}

void LFLegaliser::detectCombinableBlanksDFS(std::vector <std::pair<Tile *, Tile *>> &candidateTile, Tile &t, std::vector <Cord> &record){
    
    record.push_back(t.getLowerLeft());

    // if tile & tile.lb are mergable, push in vector and record!
    if(t.lb != nullptr){
        bool typeCorrect = (t.getType() == tileType::BLANK) && (t.lb->getType() == tileType::BLANK);
        bool leftAligned = (t.getLowerLeft().x == t.lb->getLowerLeft().x);
        bool rightAligned = (t.getWidth() == t.lb->getWidth());

        if(typeCorrect && leftAligned && rightAligned){
            candidateTile.push_back(std::make_pair(&t, t.lb));
        }
    }
    

    if(t.rt != nullptr){
        if(!checkVectorInclude(record,t.rt->getLowerLeft())){
            detectCombinableBlanksDFS(candidateTile, *(t.rt), record);
        }
    }

    if(t.lb != nullptr){
        if(!checkVectorInclude(record,t.lb->getLowerLeft())){
            detectCombinableBlanksDFS(candidateTile, *(t.lb), record);
        }
    }

    if(t.bl != nullptr){
        if(!checkVectorInclude(record,t.bl->getLowerLeft())){
            detectCombinableBlanksDFS(candidateTile, *(t.bl), record);
        }
    }

    if(t.tr != nullptr){
        if(!checkVectorInclude(record,t.tr->getLowerLeft())){
            detectCombinableBlanksDFS(candidateTile, *(t.tr), record);
        }
    }
}

void LFLegaliser::combineVerticalMergeableBlanks(Tile *upTile, Tile *downTile){
    std::vector <Tile *> mergeUpUpNeighbors;
    findTopNeighbors(upTile, mergeUpUpNeighbors);
    for(Tile *t : mergeUpUpNeighbors){
        if(t->lb == upTile) t->lb = downTile;
    }

    std::vector <Tile *> mergeUpLeftNeighbors;
    findLeftNeighbors(upTile, mergeUpLeftNeighbors);
    for(Tile *t : mergeUpLeftNeighbors){
        if(t->tr == upTile) t->tr = downTile;
    }

    std::vector <Tile *> mergeUpRightNeighbors;
    findRightNeighbors(upTile, mergeUpRightNeighbors);
    for(Tile *t : mergeUpRightNeighbors){
        if(t->bl == upTile) t->bl = downTile;
    }

    downTile->rt = upTile->rt;
    downTile->tr = upTile->tr;

    downTile->setHeight(downTile->getHeight() + upTile->getHeight());

    delete(upTile);

};


bool checkVectorInclude(std::vector<Cord> &vec, Cord c){
        for(auto const &e : vec){
            if(e == c) return true;
        }
        return false;
}

