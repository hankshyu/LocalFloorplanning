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
    x_valid = ( lowerLeft.x >= 0 ) && ( lowerLeft.x + width <= this->mCanvasWidth );
    y_valid = ( lowerLeft.y >= 0 ) && ( lowerLeft.y + height <= this->mCanvasHeight );
    return ( x_valid && y_valid );
}

bool LFLegaliser::checkTileInCanvas(Tile &tile) const {
    bool x_valid, y_valid;
    x_valid = ( tile.getLowerLeft().x >= 0 ) && ( tile.getUpperLeft().x <= this->mCanvasWidth );
    y_valid = ( tile.getLowerLeft().y >= 0 ) && ( tile.getLowerRight().y <= this->mCanvasHeight );
    return ( x_valid && y_valid );
}

Tile *LFLegaliser::getRandomTile() const {
    assert(!( fixedTesserae.empty() && softTesserae.empty() ));

    if ( !fixedTesserae.empty() ) {
        if ( !fixedTesserae[0]->TileArr.empty() ) {
            return fixedTesserae[0]->TileArr[0];
        }
        else {
            return fixedTesserae[0]->OverlapArr[0];
        }
    }
    else {
        if ( !softTesserae[0]->TileArr.empty() ) {
            return softTesserae[0]->TileArr[0];
        }
        else {
            return softTesserae[0]->OverlapArr[0];
        }
    }
}


len_t LFLegaliser::getCanvasWidth() const {
    return this->mCanvasWidth;
}

len_t LFLegaliser::getCanvasHeight() const {
    return this->mCanvasHeight;
}

//TODO: For cyuyang

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
            len_t width = (len_t) std::ceil(std::sqrt(curModule->area / aspect_ratio));
            len_t height = (len_t) std::ceil(std::sqrt(curModule->area * aspect_ratio));
            // ? which area should be filled ?
            Tessera *newTess = new Tessera(tesseraType::SOFT, curModule->name, width * height,
                Cord(curModule->x - (len_t) width / 2, (len_t) curModule->y - height / 2), width, height);
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


//TODO: For cyuyang
void LFLegaliser::detectfloorplanningOverlaps() {
    // If an overlap is detected, You should:
    // 1. Locate the overlap and crate a new Tile marking the overlap, the tile should include the spacing info and the voerlap Tessera idx
    // Tile *overlapTile = new Tile(tileType::OVERLAP, Cord(1,3), 4, 5);
    // overlapTile->OverlapFixedTesseraeIdx.pushback()....
    // overlapTile->OverlapSoftTesseraeIdx.pushback()....

    // 2. Split (both) the Tesserae into smaller tiles if it become rectlinear.
    // 3. Update (both) the Tesserae's tile list.

    //first we create an object to do the connectivity extraction
    bp::connectivity_extraction_90<int> ce;

    std::vector<Rectangle > test_data;
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

    /*
    std::cout << graph.size() << std::endl;
    for ( int i = 0; i < graph.size(); i++ ) {
        bool isSoft = i < softTesserae.size();
        int id = ( isSoft ) ? i : i - softTesserae.size();
        // std::cout << ( ( isSoft ) ? softTesserae[id]->getName() : fixedTesserae[id]->getName() ) << ": ";
        std::cout << i << ": ";
        for ( auto &n : graph[i] ) {
            bool isSoft = n < softTesserae.size();
            int id = ( isSoft ) ? n : n - softTesserae.size();
            // std::cout << ( ( isSoft ) ? softTesserae[id]->getName() : fixedTesserae[id]->getName() ) << " ";
            std::cout << n << " ";
        }
        std::cout << std::endl;
    }
    */

    // 2 overlapped
    for ( int i = 0; i < test_data.size(); i++ ) {
        bool isSoft = i < softTesserae.size();
        int id = ( isSoft ) ? i : i - softTesserae.size();
        Tessera *curTess = ( isSoft ) ? softTesserae[id] : fixedTesserae[id];
        std::cout << curTess->getName() << std::endl;
        Box curBox;
        curBox.min_corner() = Point(curTess->TileArr[0]->getLowerLeft().x, curTess->TileArr[0]->getLowerLeft().y);
        curBox.max_corner() = Point(curTess->TileArr[0]->getUpperRight().x, curTess->TileArr[0]->getUpperRight().y);
        for ( int n : graph[i] ) {
            bool isSoftOverlap = n < softTesserae.size();
            int overlapId = ( isSoftOverlap ) ? n : n - softTesserae.size();
            Tessera *overlapTess = ( isSoftOverlap ) ? softTesserae[overlapId] : fixedTesserae[overlapId];
            Box overlapBox;
            overlapBox.min_corner() = Point(overlapTess->TileArr[0]->getLowerLeft().x, overlapTess->TileArr[0]->getLowerLeft().y);
            overlapBox.max_corner() = Point(overlapTess->TileArr[0]->getUpperRight().x, overlapTess->TileArr[0]->getUpperRight().y);
            Box interectionBox;
            bg::intersection(curBox, overlapBox, interectionBox);
            len_t x = bg::get<bg::min_corner, 0>(interectionBox);
            len_t y = bg::get<bg::min_corner, 1>(interectionBox);
            len_t w = bg::get<bg::max_corner, 0>(interectionBox) - x;
            len_t h = bg::get<bg::max_corner, 1>(interectionBox) - y;
            for ( Tile *tile : curTess->OverlapArr ) {
                Box overlap3Box;
                overlap3Box.min_corner() = Point(tile->getLowerLeft().x, tile->getLowerLeft().y);
                overlap3Box.max_corner() = Point(tile->getLowerLeft().x, tile->getLowerLeft().y);
                Box interection3Box;
                bg::intersection(overlapBox, overlap3Box, interection3Box);
                if ( interection3Box.max_corner().x() == interection3Box.min_corner().x() && interection3Box.max_corner().y() == interection3Box.min_corner().y() ) {
                    std::cout << "nothing overlap\n";
                }
                else {
                    std::cout << "Overlapped Detected: ";
                    std::cout << "(" << bg::get<bg::min_corner, 0>(interection3Box) << ", " << bg::get<bg::min_corner, 1>(interection3Box) << "), ";
                    std::cout << "(" << bg::get<bg::max_corner, 0>(interection3Box) << ", " << bg::get<bg::max_corner, 1>(interection3Box) << ")\n";
                }

            }
            Tile *overlapTile = new Tile(tileType::OVERLAP, Cord(x, y), w, h);
            ( isSoft ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(id)
                : overlapTile->OverlapFixedTesseraeIdx.push_back(id);

            ( isSoftOverlap ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(overlapId)
                : overlapTile->OverlapFixedTesseraeIdx.push_back(overlapId);

            curTess->insertTiles(tileType::OVERLAP, overlapTile);
        }
    }

    // 3 overlapped
    for ( int i = 0; i < test_data.size(); i++ ) {
        std::vector<bool> visited(test_data.size(), 0);
        int nodes[3] = { -1 };
        if ( hasCycle3(graph, visited, i, -1, 1, nodes) ) {
            Box box[3];
            for ( int n = 0; n < 3; n++ ) {
                int curNode = nodes[n];
                bool isSoft = curNode < softTesserae.size();
                int id = ( isSoft ) ? curNode : curNode - softTesserae.size();
                Tessera *curTess = ( isSoft ) ? softTesserae[id] : fixedTesserae[id];
                box[n].min_corner() = Point(curTess->TileArr[0]->getLowerLeft().x, curTess->TileArr[0]->getLowerLeft().y);
                box[n].max_corner() = Point(curTess->TileArr[0]->getUpperRight().x, curTess->TileArr[0]->getUpperRight().y);
            }
            Box interectionBox;
            bg::intersection(box[0], box[1], interectionBox);
            bg::intersection(interectionBox, box[2], interectionBox);
            std::cout << "3 Modules Intersection: ";
            std::cout << "(" << bg::get<bg::min_corner, 0>(interectionBox) << ", " << bg::get<bg::min_corner, 1>(interectionBox) << "), ";
            std::cout << "(" << bg::get<bg::max_corner, 0>(interectionBox) << ", " << bg::get<bg::max_corner, 1>(interectionBox) << ")\n";
            for ( int n = 0; n < 3; n++ ) {
                int curNode = nodes[n];
                bool isSoft = curNode < softTesserae.size();
                int id = ( isSoft ) ? curNode : curNode - softTesserae.size();
                Tessera *curTess = ( isSoft ) ? softTesserae[id] : fixedTesserae[id];
                len_t x = bg::get<bg::min_corner, 0>(interectionBox);
                len_t y = bg::get<bg::min_corner, 1>(interectionBox);
                len_t w = bg::get<bg::max_corner, 0>(interectionBox) - x;
                len_t h = bg::get<bg::max_corner, 1>(interectionBox) - y;
                bool existed = false;
                for ( Tile *tile : curTess->OverlapArr ) {
                    if ( tile->getLowerLeft().x == x && tile->getLowerLeft().y == y && tile->getWidth() == w && tile->getHeight() == h ) {
                        existed = true;
                        break;
                    }
                }
                if ( !existed ) {
                    Tile *overlapTile = new Tile(tileType::OVERLAP, Cord(x, y), w, h);
                    ( nodes[0] < softTesserae.size() ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(nodes[0]) :
                        overlapTile->OverlapFixedTesseraeIdx.push_back(nodes[0] - softTesserae.size());

                    ( nodes[1] < softTesserae.size() ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(nodes[1]) :
                        overlapTile->OverlapFixedTesseraeIdx.push_back(nodes[1] - softTesserae.size());

                    ( nodes[2] < softTesserae.size() ) ? overlapTile->OverlapSoftTesseraeIdx.push_back(nodes[2]) :
                        overlapTile->OverlapFixedTesseraeIdx.push_back(nodes[2] - softTesserae.size());

                    curTess->insertTiles(tileType::OVERLAP, overlapTile);
                }
            }
        }
    }
}

void LFLegaliser::splitFloorplanningOverlaps() {
    // Soft&Hard block overlap are located and split if necessary in OverlapArr of each Tessera
    // now cut rectlinear blank space of each Tessera into multiple blank tiles.

    for ( Tessera *fixedTess : this->fixedTesserae ) {
        fixedTess->splitRectliearDueToOverlap();
    }

    for ( Tessera *softTess : this->softTesserae ) {
        softTess->splitRectliearDueToOverlap();
    }
}

Tile *LFLegaliser::findPoint(const Cord &key) const {
    assert(key >= Cord(0, 0));
    assert(key.x < getCanvasWidth());
    assert(key.y < getCanvasHeight());

    Tile *index = getRandomTile();

    while ( !( index->checkCordInTile(key) ) ) {
        if ( !index->checkYCordInTile(key) ) {
            // Adjust vertical range
            if ( key.y >= index->getLowerLeft().y ) {
                assert(index->rt != nullptr);
                index = index->rt;
            }
            else {
                assert(index->lb != nullptr);
                index = index->lb;
            }
        }
        else {
            // Vertical range correct! adjust horizontal range
            if ( key.x >= index->getLowerLeft().x ) {
                assert(index->tr != nullptr);
                index = index->tr;
            }
            else {
                assert(index->bl != nullptr);
                index = index->bl;
            }
        }
    }

    return index;
}

Tile *LFLegaliser::findPoint(const Cord &key, Tile *initTile) const {
    assert(key >= Cord(0, 0));
    assert(key.x < getCanvasWidth());
    assert(key.y < getCanvasHeight());

    Tile *index = initTile;

    while ( !( index->checkCordInTile(key) ) ) {
        if ( !index->checkYCordInTile(key) ) {
            // Adjust vertical range
            if ( key.y >= index->getLowerLeft().y ) {
                assert(index->rt != nullptr);
                index = index->rt;
            }
            else {
                assert(index->lb != nullptr);
                index = index->lb;
            }
        }
        else {
            // Vertical range correct! adjust horizontal range
            if ( key.x >= index->getLowerLeft().x ) {
                assert(index->tr != nullptr);
                index = index->tr;
            }
            else {
                assert(index->bl != nullptr);
                index = index->bl;
            }
        }
    }

    return index;
}

void LFLegaliser::findTopNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const {
    if ( centre->rt == nullptr ) return;
    Tile *n = centre->rt;
    while ( n->getLowerLeft().x > centre->getLowerLeft().x ) {
        neighbors.push_back(n);
        n = n->bl;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findDownNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const {
    if ( centre->lb == nullptr ) return;
    Tile *n = centre->lb;
    while ( n->getUpperRight().x < centre->getUpperRight().x ) {
        neighbors.push_back(n);
        n = n->tr;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findLeftNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const {
    if ( centre->bl == nullptr ) return;
    Tile *n = centre->bl;
    while ( n->getUpperRight().y < centre->getUpperRight().y ) {
        neighbors.push_back(n);
        n = n->rt;
    }
    neighbors.push_back(n);
}

void LFLegaliser::findRightNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const {
    if ( centre->tr == nullptr ) return;
    Tile *n = centre->tr;
    // the last neighbor is the first tile encountered whose lower y cord <= lower y cord of starting tile
    while ( n->getLowerLeft().y > centre->getLowerLeft().y ) {
        neighbors.push_back(n);
        n = n->lb;
    }
    neighbors.push_back(n);

}

void LFLegaliser::findAllNeighbors(Tile *centre, std::vector<Tile *> &neighbors) const {
    findTopNeighbors(centre, neighbors);
    findDownNeighbors(centre, neighbors);
    findLeftNeighbors(centre, neighbors);
    findRightNeighbors(centre, neighbors);
}

bool LFLegaliser::searchArea(Cord lowerleft, len_t width, len_t height, Tile &target) const {

    assert(checkTesseraInCanvas(lowerleft, width, height));

    // Use point-finding algo to locate the tile containin the upperleft corner of AOI
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *currentFind = findPoint(Cord(lowerleft.x, searchRBorderHeight));
    std::cout << "Init found:" << std::endl;

    while ( currentFind->getUpperLeft().y > lowerleft.y ) {
        // See if the tile is solid
        if ( currentFind->getType() != tileType::BLANK ) {
            // This is an edge of a solid tile
            target = *currentFind;
            return true;
        }
        else if ( currentFind->getUpperRight().x < lowerleft.x + width ) {
            // See if the right edge within AOI, right must be a tile
            target = *( currentFind->tr );
            return true;
        }
        else {
            // Move down to the next tile touching the left edge of AOI
            if ( currentFind->getLowerLeft().y <= 1 ) {
                break;
            }
            currentFind = findPoint(Cord(lowerleft.x, currentFind->getLowerLeft().y - 1));
        }
    }

    return false;

}

bool LFLegaliser::searchArea(Cord lowerleft, len_t width, len_t height) const {
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *currentFind = findPoint(Cord(lowerleft.x, searchRBorderHeight));

    while ( currentFind->getUpperLeft().y > lowerleft.y ) {
        // See if the tile is solid
        if ( currentFind->getType() != tileType::BLANK ) {
            // This is an edge of a solid tile
            return true;
        }
        else if ( currentFind->getUpperRight().x < lowerleft.x + width ) {
            // See if the right edge within AOI, right must be a tile
            return true;
        }
        else {
            if ( currentFind->getLowerLeft().y <= 1 ) {
                break;
            }
            currentFind = findPoint(Cord(lowerleft.x, currentFind->getLowerLeft().y - 1));
        }
    }

    return false;
}

void LFLegaliser::enumerateDirectArea(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles) const {
    len_t searchRBorderHeight = lowerleft.y + height - 1;
    Tile *leftTouchTile = findPoint(Cord(lowerleft.x, searchRBorderHeight));


    while ( leftTouchTile->getUpperLeft().y > lowerleft.y ) {
        enumerateDirectAreaRProcess(lowerleft, width, height, allTiles, leftTouchTile);
        if ( leftTouchTile->getLowerLeft().y <= 1 ) break;
        leftTouchTile = findPoint(Cord(lowerleft.x, leftTouchTile->getLowerLeft().y - 1));
    }
}

void LFLegaliser::enumerateDirectAreaRProcess(Cord lowerleft, len_t width, len_t height, std::vector <Tile *> &allTiles, Tile *targetTile) const {

    // R1) Enumerate the tile
    if ( targetTile->getType() == tileType::BLOCK || targetTile->getType() == tileType::OVERLAP ) {
        allTiles.push_back(targetTile);

    }

    // R2) If the right edge of the tile is outside of the seearch area, return
    if ( targetTile->getLowerRight().x >= ( lowerleft.x + width ) ) {
        return;
    }

    // R3) Use neighbor-finding algo to locate all the tiles that touch the right side of the current tile and also intersect the search area
    std::vector<Tile *> rightNeighbors;
    findRightNeighbors(targetTile, rightNeighbors);
    for ( Tile *t : rightNeighbors ) {

        // R4) If bottom left corner of the neighbor touches the current tile
        bool R4 = targetTile->checkTRLLTouch(t);
        // R5) If the bottom edge ofthe search area cuts both the urrent tile and the neighbor
        bool R5 = ( targetTile->cutHeight(lowerleft.y) ) && ( t->cutHeight(lowerleft.y) );

        if ( R4 || R5 ) {
            enumerateDirectAreaRProcess(lowerleft, width, height, allTiles, t);
        }
    }

}

void LFLegaliser::insertFirstTile(Tile &newTile) {
    assert(this->checkTileInCanvas(newTile));
    // cut the canvas into four parts: above, below, left, rifht

    if ( newTile.getLowerLeft().y != 0 ) {
        Tile *tdown = new Tile(tileType::BLANK, Cord(0, 0),
            this->mCanvasWidth, newTile.getLowerLeft().y);
        newTile.lb = tdown;
        tdown->rt = &newTile;
    }

    if ( newTile.getUpperRight().y <= this->mCanvasHeight ) {
        Tile *tup = new Tile(tileType::BLANK, Cord(0, newTile.getUpperRight().y),
            this->mCanvasWidth, ( this->mCanvasHeight - newTile.getUpperRight().y ));
        newTile.rt = tup;
        tup->lb = &newTile;
    }

    if ( newTile.getLowerLeft().x != 0 ) {
        Tile *tleft = new Tile(tileType::BLANK, Cord(0, newTile.getLowerLeft().y),
            newTile.getLowerLeft().x, ( newTile.getUpperLeft().y - newTile.getLowerLeft().y ));
        newTile.bl = tleft;
        tleft->tr = &newTile;
    }

    if ( newTile.getLowerRight().x != this->mCanvasWidth ) {
        Tile *tright = new Tile(tileType::BLANK, newTile.getLowerRight(),
            ( this->mCanvasWidth - newTile.getUpperRight().x ), ( newTile.getUpperLeft().y - newTile.getLowerLeft().y ));
        newTile.tr = tright;
        tright->bl = &newTile;
    }
}

// Not yet complete.... 
void LFLegaliser::insertTile(Tile &tile) {
    assert(checkTesseraInCanvas(tile.getLowerLeft(), tile.getWidth(), tile.getHeight()));
    assert(!searchArea(tile.getLowerLeft(), tile.getWidth(), tile.getHeight()));

    /* STEP 1) Find the space Tile containing the top edge of the aera to be occupied, process */
    bool tileTouchesSky = ( tile.getUpperRight().y == getCanvasHeight() );
    bool cleanTopCut = true;
    Tile *origTop;

    if ( !tileTouchesSky ) {
        origTop = findPoint(tile.getUpperLeft());
        cleanTopCut = ( origTop->getLowerLeft().y == tile.getUpperRight().y );
    }

    if ( ( !tileTouchesSky ) && ( !cleanTopCut ) ) {


        Tile *newDown = new Tile(tileType::BLANK, origTop->getLowerLeft(), origTop->getWidth(), ( tile.getUpperLeft().y - origTop->getLowerLeft().y ));
        newDown->rt = origTop;
        newDown->lb = origTop->lb;
        newDown->bl = origTop->bl;

        // manipulate neighbors around the split tiles

        // change lower-neighbors' rt pointer to newly created tile
        std::vector <Tile *> origDownNeighbors;
        findDownNeighbors(origTop, origDownNeighbors);
        for ( Tile *t : origDownNeighbors ) {
            if ( t->rt == origTop ) {
                t->rt = newDown;
            }
        }

        // change right neighbors to point their bl to the correct tile (one of the split)
        std::vector <Tile *> origRightNeighbors;
        findRightNeighbors(origTop, origRightNeighbors);

        bool rightModified = false;
        for ( int i = 0; i < origRightNeighbors.size(); ++i ) {
            if ( origRightNeighbors[i]->getLowerLeft().y < newDown->getUpperRight().y ) {
                if ( !rightModified ) {
                    rightModified = true;
                    newDown->tr = origRightNeighbors[i];
                }
                origRightNeighbors[i]->bl = newDown;

            }
        }

        // change Left neighbors to point their tr to the correct tiles
        std::vector <Tile *> origLeftNeighbors;
        findLeftNeighbors(origTop, origLeftNeighbors);

        bool leftModified = false;
        for ( int i = 0; i < origLeftNeighbors.size(); ++i ) {
            if ( origLeftNeighbors[i]->getUpperLeft().y > tile.getUpperLeft().y ) {
                if ( !leftModified ) {
                    leftModified = true;
                    origTop->bl = origLeftNeighbors[i];
                }
            }
            else {
                origLeftNeighbors[i]->tr = newDown;
            }
        }
        len_t oUpperLeft = origTop->getUpperLeft().y;
        origTop->setCord(Cord(origTop->getLowerLeft().x, tile.getUpperLeft().y));
        origTop->setHeight(oUpperLeft - tile.getUpperLeft().y);
        origTop->lb = newDown;

    }

    /* STEP 2) Find the space Tile containing the bottom edge of the aera to be occupied, process */

    bool tileTouchesGround = ( tile.getLowerLeft().y == 0 );
    bool cleanBottomCut = true;
    Tile *origBottom;
    if ( !tileTouchesGround ) {
        origBottom = findPoint(tile.getLowerLeft() - Cord(0, 1));
        cleanBottomCut = ( origBottom->getUpperRight().y == tile.getLowerLeft().y );
    }

    if ( ( !tileTouchesGround ) && ( !cleanBottomCut ) ) {

        Tile *newUp = new Tile(tileType::BLANK, Cord(origBottom->getLowerLeft().x, tile.getLowerLeft().y)
            , origBottom->getWidth(), ( origBottom->getUpperLeft().y - tile.getLowerLeft().y ));

        newUp->rt = origBottom->rt;
        newUp->lb = origBottom;
        newUp->tr = origBottom->tr;

        // manipulate neighbors around the split tiles

        // change the upper-neighbors' lb pointer to newly created tile
        std::vector <Tile *> origUpNeighbors;
        findTopNeighbors(origBottom, origUpNeighbors);
        for ( Tile *t : origUpNeighbors ) {
            if ( t->lb == origBottom ) {
                t->lb = newUp;
            }
        }

        // change right neighbors to point their bl to the correct tle (one of the split)
        std::vector <Tile *> origRightNeighbors;
        findRightNeighbors(origBottom, origRightNeighbors);

        bool rightModified = false;
        for ( int i = 0; i < origRightNeighbors.size(); ++i ) {
            if ( origRightNeighbors[i]->getLowerLeft().y < tile.getLowerLeft().y ) {
                if ( rightModified ) {
                    rightModified = true;
                    origBottom->tr = origRightNeighbors[i];
                }
            }
            else {
                origRightNeighbors[i]->bl = newUp;
            }
        }

        // change left neighbors to point their tr to the correct tiles
        std::vector <Tile *> origLeftNeighbors;
        findLeftNeighbors(origBottom, origLeftNeighbors);

        bool leftModified = false;
        for ( int i = 0; i < origLeftNeighbors.size(); ++i ) {
            if ( origLeftNeighbors[i]->getUpperLeft().y > tile.getLowerLeft().y ) {
                if ( !leftModified ) {
                    leftModified = true;
                    newUp->bl = origLeftNeighbors[i];
                }
                origLeftNeighbors[i]->tr = newUp;
            }
        }

        // origBottom->setCord(Cord(origBottom->getLowerLeft().x, tile.getLowerLeft().y));
        origBottom->setHeight(tile.getLowerLeft().y - origBottom->getLowerLeft().y);
        origBottom->rt = newUp;

    }

    // STEP3) The area of the new tile is traversed from top to bottom, splitting and joining space tiles on either side
    //        and pointing their stiches at the new tile

    // locate the topmost blank-tile, split in 3 pieces, left, mid and right

    Tile *topBlank = findPoint(Cord(tile.getUpperLeft() - Cord(0, 1)));

    // Merge helping indexes
    len_t leftMergeWidth = 0, rightMergeWidth = 0;
    Tile *mergeLeft = nullptr, *mergeMid = nullptr, *mergeRight = nullptr;

    // split into three pieces
    len_t blankLeftBorder = topBlank->getLowerLeft().x;
    len_t tileLeftBorder = tile.getLowerLeft().x;
    len_t tileRightBorder = tile.getLowerRight().x;
    len_t blankRightBorder = topBlank->getLowerRight().x;

    // The middle piece (must have)
    //Tile *newMid = new Tile(tile.getType(), Cord(tileLeftBorder, topBlank->getLowerLeft().y), tile.getWidth(), topBlank->getHeight());
    //newMid

    // split the left piece

    //if(blankLeftBorder != tileLeftBorder){
    //    Tile *newLeft = new Tile(tileType::BLANK, topBlank->getLowerLeft(),(tileLeftBorder - blankLeftBorder) ,topBlank->getHeight());
    //    visualiseAddMark(newLeft);
    //    newLeft->tr = topBlank;
    //    new
    //    
    //}


    // if(topBlank->getLowerRight().x != tile.getLowerRight().x){

    // }




}

void LFLegaliser::visualiseArtpiece(const std::string outputFileName) {

    std::cout << "print to file..." << outputFileName << std::endl;

    std::ofstream ofs(outputFileName);
    ofs << "BLOCK " << fixedTesserae.size() + softTesserae.size() << std::endl;
    ofs << this->mCanvasWidth << " " << this->mCanvasHeight << std::endl;

    if ( fixedTesserae.size() == 0 && softTesserae.size() == 0 ) {
        //there is no blocks
        ofs.close();
        return;
    }

    for ( Tessera *tess : softTesserae ) {
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " " << "SOFT_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for ( Tile *t : tess->TileArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for ( Tile *t : tess->OverlapArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" << std::endl;
        }
    }

    for ( Tessera *tess : fixedTesserae ) {
        ofs << tess->getName() << " " << tess->getLegalArea() << " ";
        ofs << tess->getBBLowerLeft().x << " " << tess->getBBLowerLeft().y << " ";
        ofs << tess->getBBWidth() << " " << tess->getBBHeight() << " " << "HARD_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for ( Tile *t : tess->TileArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for ( Tile *t : tess->OverlapArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" << std::endl;
        }
    }

    // DFS Traverse through all balnk tiles
    if ( fixedTesserae.size() != 0 ) {
        if ( this->fixedTesserae[0]->TileArr.size() != 0 ) {
            traverseBlank(ofs, *( this->fixedTesserae[0]->TileArr[0] ));
        }
        else {
            traverseBlank(ofs, *( this->fixedTesserae[0]->OverlapArr[0] ));
        }
    }
    else {
        if ( softTesserae.size() != 0 ) {
            traverseBlank(ofs, *( this->softTesserae[0]->TileArr[0] ));
        }
        else {
            traverseBlank(ofs, *( this->softTesserae[0]->OverlapArr[0] ));
        }
    }
    ofs << "CONNECTION 0" << std::endl;

    // print all the marked tiles
    for ( Tile *t : this->mMarkedTiles ) {
        ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " MARKED" << std::endl;
    }

    ofs.close();
}

void LFLegaliser::visualiseArtpieceCYY(const std::string outputFileName) {

    std::cout << "print to file..." << outputFileName << std::endl;

    std::ofstream ofs(outputFileName);
    ofs << "BLOCK " << fixedTesserae.size() + softTesserae.size() << std::endl;
    ofs << this->mCanvasWidth << " " << this->mCanvasHeight << std::endl;

    if ( fixedTesserae.size() == 0 && softTesserae.size() == 0 ) {
        //there is no blocks
        ofs.close();
        return;
    }


    for ( int i = 0; i < softTesserae.size(); i++ ) {
        Tessera *tess = softTesserae[i];
        ofs << tess->getName() << " ";
        // ofs << i << " ";
        ofs << tess->TileArr[0]->getLowerLeft().x << " " << tess->TileArr[0]->getLowerLeft().y << " ";
        ofs << tess->TileArr[0]->getWidth() << " " << tess->TileArr[0]->getHeight() << " " << "SOFT_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for ( Tile *t : tess->TileArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for ( Tile *t : tess->OverlapArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" << std::endl;
        }
    }

    for ( int i = 0; i < fixedTesserae.size(); i++ ) {
        Tessera *tess = fixedTesserae[i];
        ofs << tess->getName() << " ";
        // ofs << i + softTesserae.size() << " ";
        ofs << tess->TileArr[0]->getLowerLeft().x << " " << tess->TileArr[0]->getLowerLeft().y << " ";
        ofs << tess->TileArr[0]->getWidth() << " " << tess->TileArr[0]->getHeight() << " " << "HARD_BLOCK" << std::endl;
        ofs << tess->TileArr.size() << " " << tess->OverlapArr.size() << std::endl;
        for ( Tile *t : tess->TileArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " BLOCK" << std::endl;
        }
        for ( Tile *t : tess->OverlapArr ) {
            ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " OVERLAP" << std::endl;
        }
    }

    // DFS Traverse through all balnk tiles
    if ( fixedTesserae.size() != 0 ) {
        if ( this->fixedTesserae[0]->TileArr.size() != 0 ) {
            traverseBlank(ofs, *( this->fixedTesserae[0]->TileArr[0] ));
        }
        else {
            traverseBlank(ofs, *( this->fixedTesserae[0]->OverlapArr[0] ));
        }
    }
    else {
        if ( softTesserae.size() != 0 ) {
            traverseBlank(ofs, *( this->softTesserae[0]->TileArr[0] ));
        }
        else {
            traverseBlank(ofs, *( this->softTesserae[0]->OverlapArr[0] ));
        }
    }
    ofs << "CONNECTION 0" << std::endl;

    // print all the marked tiles
    for ( Tile *t : this->mMarkedTiles ) {
        ofs << t->getLowerLeft().x << " " << t->getLowerLeft().y << " " << t->getWidth() << " " << t->getHeight() << " MARKED" << std::endl;
    }

    ofs.close();
}

void LFLegaliser::traverseBlank(std::ofstream &ofs, Tile &t) {

    t.printLabel = ( !t.printLabel );

    if ( t.getType() == tileType::BLANK ) {

        ofs << t.getLowerLeft().x << " " << t.getLowerLeft().y << " ";
        ofs << t.getWidth() << " " << t.getHeight() << " ";
        ofs << "BLANK_TILE" << std::endl;
    }

    if ( t.rt != nullptr ) {
        if ( t.rt->printLabel != t.printLabel ) {
            traverseBlank(ofs, *( t.rt ));
        }
    }

    if ( t.lb != nullptr ) {
        if ( t.lb->printLabel != t.printLabel ) {
            traverseBlank(ofs, *( t.lb ));
        }
    }

    if ( t.bl != nullptr ) {
        if ( t.bl->printLabel != t.printLabel ) {
            traverseBlank(ofs, *( t.bl ));
        }
    }

    if ( t.tr != nullptr ) {
        if ( t.tr->printLabel != t.printLabel ) {
            traverseBlank(ofs, *( t.tr ));
        }
    }

    return;
}

void LFLegaliser::visualiseAddMark(Tile *markTile) {
    this->mMarkedTiles.push_back(markTile);
}