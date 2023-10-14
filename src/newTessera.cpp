#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Tessera.h"
#include "boost/polygon/polygon.hpp"

namespace gtl = boost::polygon;

Tessera::Tessera()
    : mType(tesseraType::EMPTY) {}

Tessera::Tessera(tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height)
    : mType(type), mName(name), mLegalArea(area), 
    mInitLowerLeft(lowerleft), mInitWidth(width), mInitHeight(height) {
        Tile *defaultTess = new Tile(tileType::BLOCK, lowerleft, width, height);
        TileArr.push_back(defaultTess);
        calBoundingBox();
    }

Tessera::Tessera(const Tessera &other)
    : mType(other.getType()), mName(other.getName()), mLegalArea(other.getLegalArea()),
    mInitLowerLeft(other.getInitLowerLeft()), mInitWidth(other.getInitWidth()), mInitHeight(other.getInitHeight()) {
        TileArr.assign(other.TileArr.begin(), other.TileArr.end());
        OverlapArr.assign(other.OverlapArr.begin(), other.OverlapArr.end());
    }

Tessera& Tessera::operator = (const Tessera &other){
    if(this == &other) return (*this);

    this->mType = other.getType();
    this->mName = other.getName();
    this->mLegalArea = other.getLegalArea();

    this->mInitLowerLeft = other.getInitLowerLeft();
    this->mInitWidth = other.getInitWidth();
    this->mInitHeight = other.getInitHeight();

    TileArr.assign(other.TileArr.begin(), other.TileArr.end());
    OverlapArr.assign(other.OverlapArr.begin(), other.OverlapArr.end());
    
    return (*this);
}

bool Tessera::operator ==(const Tessera &tess) const{
    
    if(mType != tess.getType()) return false;
    if(mName != tess.getName()) return false;
    if(mLegalArea != tess.getLegalArea()) return false;
    if(mInitLowerLeft != tess.getInitLowerLeft()) return false;
    if((mInitWidth != tess.getInitWidth()) || (mInitHeight != tess.getInitHeight())) return false;

    if(TileArr.size() != tess.TileArr.size()) return false;
    for(int i = 0; i < TileArr.size(); ++i){
        if(TileArr[i] != tess.TileArr[i]) return false;
    }

    if(OverlapArr.size() != tess.OverlapArr.size()) return false;
    for(int i = 0; i < OverlapArr.size(); ++i){
        if(OverlapArr[i] != tess.OverlapArr[i]) return false;
    }
    return true;
}


std::string Tessera::getName () const{
    return this->mName;
}

area_t Tessera::getLegalArea () const{
    return this->mLegalArea;
}

tesseraType Tessera::getType () const{
    return this->mType;
}

Cord Tessera::getInitLowerLeft () const{
    return this->mInitLowerLeft;
}

len_t Tessera::getInitWidth() const{
    return this->mInitWidth;
}

len_t Tessera::getInitHeight() const{
    return this->mInitHeight;
}

void Tessera::calBBCentre(double &CentreX, double &CentreY){
    calBoundingBox();
    Cord LL = getBBLowerLeft();
    Cord UR = getBBUpperRight();
    CentreX = ((double)(LL.x + UR.x))/2;
    CentreY = ((double)(LL.y + UR.y))/2;

}

Cord Tessera::getBBLowerLeft(){
    calBoundingBox();
    return this->mBBLowerLeft;
}
Cord Tessera::getBBUpperRight(){
    calBoundingBox();
    return this->mBBUpperRight;
}
len_t Tessera::getBBWidth(){
    calBoundingBox();
    return this->mBBUpperRight.x - this->mBBLowerLeft.x;
}
len_t Tessera::getBBHeight(){
    calBoundingBox();
    return this->mBBUpperRight.y - this->mBBLowerLeft.y;
}

void Tessera::calBoundingBox(){
    if(TileArr.empty() && OverlapArr.empty()) return;

    //The lowerleft and upper right tiles
    Cord BBLL, BBUR;

    if(!TileArr.empty()){
        BBLL = TileArr[0]->getLowerLeft();
        BBUR = TileArr[0]->getUpperRight();
    }else{
        BBLL = OverlapArr[0]->getLowerLeft();
        BBUR = OverlapArr[0]->getUpperRight();
    }

    for(Tile *t : TileArr){
        Cord tLL = t->getLowerLeft();
        Cord tUR = t->getUpperRight();

        if(tLL.x < BBLL.x) BBLL.x = tLL.x;
        if(tLL.y < BBLL.y) BBLL.y = tLL.y;
        
        if(tUR.x > BBUR.x) BBUR.x = tUR.x;
        if(tUR.y > BBUR.y) BBUR.y = tUR.y;
    }
    for(Tile *t : OverlapArr){
        Cord tLL = t->getLowerLeft();
        Cord tUR = t->getUpperRight();

        if(tLL.x < BBLL.x) BBLL.x = tLL.x;
        if(tLL.y < BBLL.y) BBLL.y = tLL.y;
        
        if(tUR.x > BBUR.x) BBUR.x = tUR.x;
        if(tUR.y > BBUR.y) BBUR.y = tUR.y;
    }

    this->mBBLowerLeft = BBLL;
    this->mBBUpperRight = BBUR;
}

area_t Tessera::calRealArea(){
    area_t realArea = 0;
    for(Tile *t : this->TileArr){
        realArea += t->getArea();
    }
    for(Tile *t : this->OverlapArr){
        realArea += t->getArea();
    }
    return realArea;
}

area_t Tessera::calAreaMargin() {
    return calRealArea() - this->mLegalArea;
}

// TODO: This may need boost lib...
bool Tessera::checkLegalNoHole(){
    return true;
}

// TODO: This may need boost lib...
bool Tessera::checkLegalNoEnclave(){
    return true;
}

bool Tessera::checkLegalEnoughArea(){
    return (calRealArea() >= this->mLegalArea);
}

bool Tessera::checkLegalAspectRatio(){
    calBoundingBox();
    double aspectRatio = ((double)this->getBBHeight()) / ((double)this->getBBWidth());

    return (aspectRatio <= 2) && (aspectRatio >= 0.5);

}

bool Tessera::checkLegalStuffedRatio(){
    calBoundingBox();
    area_t BBArea = (area_t)getBBHeight() * (area_t)getBBWidth();
    double stuffedRatio = ((double)this->getLegalArea()) / ((double) BBArea);
    return (stuffedRatio >= 0.8);

}

int Tessera::checkLegal(){
    if(!checkLegalNoHole()) return 1;
    if(!checkLegalNoEnclave()) return 2;
    if(!checkLegalEnoughArea()) return 3;
    if(!checkLegalAspectRatio()) return 4;
    if(!checkLegalStuffedRatio()) return 5;
    return 0;
}

int Tessera::insertTiles(Tile *tile){
    assert(tile->getType() != tileType::BLANK);
    switch (tile->getType()){
        case tileType::BLOCK:
            /* code */
            TileArr.push_back(tile);
            break;
        case tileType::OVERLAP:
            /* code */
            OverlapArr.push_back(tile);
            break;
        default:
            break;
    }
    calBoundingBox();

    return 1;
}



/*
    Notes for Ryan Lin...
    We would call translateGlobalFloorplanning() from LFLegaliser.h to translate cirlces from 
    global floorplanning to Rectangles (Tessera.h), and it would include a default Tile (Tile.h)
    We would then call detectfloorplanningOverlaps() to detect overlaps. Overlaps would be record as
    another Tile(label with tileType::OVERLAP)
    Then call splitFloorplanningOverlaps() from LFLegaliser.h
    it would :
    for(Tessera t : all Tesserae vector){
        t.splitRectliearDueToOverlap()
    }
    
    to acquire the correct tile distribution with overlap tiles correctly split.
*/
void Tessera::splitRectliearDueToOverlap(){
    
    /*
    The translated info is stored in:
    - Cord mInitLowerLeft;
    - len_t mInitWidth;
    - len_t mInitHeight;

    There should be "ONE" default Tile in TileArr, which is the default tile(= InitXXX...)
    Overlap tiles are calculated as pushed into overlapArr
    You should read the entire overlap Arr and mark off the area indicated, this would lead you to 
    a rectlinear shape.
    Then you should further split the acquired rectilinear, and split them into smaller tiles. you "MUST"
    remove the default tile if new smaller splits are pushed in. GOOD LUCK!!

    p.s. Just insert the smaller tiles like so:
    Tile *newsmallersplit = new Tile(tileType::BLOCK, Cord(x, y), width, height);
    
    You don't have to maintain *up *donw *left right pointers, they will be taken care of when insertion happen
    */

    // if overlap exists, default tile will always be split
    // delete default tile, insert tile size of block
    if (OverlapArr.size() > 0 ){
        Tile* defaultTile = TileArr.back();
        Cord lowerLeft = defaultTile->getLowerLeft();
        len_t width = defaultTile->getWidth();
        len_t height = defaultTile->getHeight();
        // Do I need to delete pointer????
        delete defaultTile; 
        Tile* newDefault = new Tile(tileType::BLOCK, lowerLeft, width, height);
        TileArr[0] = newDefault;
    }
    

    // iterate thru all overlaps
    for (int o = 0; o < OverlapArr.size(); o++){
        std::vector <Tile *> influencedTiles;
        Tile* currentOverlap = OverlapArr[o];
        int overlapRightBoundary = currentOverlap->getUpperRight().x;
        int overlapUpperBoundary = currentOverlap->getUpperRight().y;
        int overlapLeftBoundary = currentOverlap->getLowerLeft().x;
        int overlapLowerBoundary = currentOverlap->getLowerLeft().y;

        // step 1: find all block Tiles that will be influenced
        for (int t = 0; t < TileArr.size(); ++t){
            // check overlap
            bool xOverlap = TileArr[t]->getLowerLeft().x < currentOverlap->getUpperRight().x &&
                            currentOverlap->getLowerLeft().x < TileArr[t]->getUpperRight().x;
            bool yOverlap = TileArr[t]->getLowerLeft().y < currentOverlap->getUpperRight().y &&
                            currentOverlap->getLowerLeft().y < TileArr[t]->getUpperRight().y;
            if (xOverlap && yOverlap){
                influencedTiles.push_back(TileArr[t]);
            }
        }

        // step 2: find Tiles that include the upper/lower boundary of overlap,  
        // split those tiles
        for (int i = 0; i < influencedTiles.size(); ++i){
            Tile* currentTile = influencedTiles[i];
            int currentRightBoundary = currentTile->getUpperRight().x;
            int currentUpperBoundary = currentTile->getUpperRight().y;
            int currentLeftBoundary = currentTile->getLowerLeft().x;
            int currentLowerBoundary = currentTile->getLowerLeft().y;
            bool includeUpperBoundary = currentLowerBoundary < overlapUpperBoundary && overlapUpperBoundary < currentUpperBoundary;
            if (includeUpperBoundary){
                // change hieght of currentTile, add a new tile to tile array
                Cord newLL = Cord(currentLeftBoundary, overlapUpperBoundary);
                int newWidth = currentTile->getWidth();
                int newHeight = currentUpperBoundary - overlapUpperBoundary;
                Tile* newTile = new Tile(tileType::BLOCK, newLL, newWidth, newHeight);
                TileArr.push_back(newTile);

                int alterHeight = overlapUpperBoundary - currentLowerBoundary;
                currentTile->setHeight(alterHeight);
                currentUpperBoundary = currentTile->getUpperRight().y;
            }

            bool includeLowerBoundary = currentLowerBoundary < overlapLowerBoundary && overlapLowerBoundary < currentUpperBoundary;
            if (includeLowerBoundary){
                // change LL and height of currentTile, add a new tile to tile array
                Cord newLL = Cord(currentLeftBoundary, currentLowerBoundary);
                int newWidth = currentTile->getWidth();
                int newHeight = overlapLowerBoundary - currentLowerBoundary;
                Tile* newTile = new Tile(tileType::BLOCK, newLL, newWidth, newHeight);
                TileArr.push_back(newTile);

                int alterHeight = currentUpperBoundary - overlapLowerBoundary;
                Cord alterLL = Cord(currentLeftBoundary, overlapLowerBoundary);
                currentTile->setHeight(alterHeight);
                currentTile->setLowerLeft(alterLL);
            }
        }

        // step 3: split all influenced tiles into 2 segments (left, right) if possible
        for (int i = 0; i < influencedTiles.size(); ++i){
            Tile* currentTile = influencedTiles[i];
            int currentRightBoundary = currentTile->getUpperRight().x;
            int currentUpperBoundary = currentTile->getUpperRight().y;
            int currentLeftBoundary = currentTile->getLowerLeft().x;
            int currentLowerBoundary = currentTile->getLowerLeft().y;
            bool includeLeftBoundary = currentLeftBoundary < overlapLeftBoundary && overlapLeftBoundary < currentRightBoundary;
            bool includeRightBoundary = currentLeftBoundary < overlapRightBoundary && overlapRightBoundary < currentRightBoundary;

            // case 1: tile only includes left boundary
            // shrink tile width
            if (includeLeftBoundary && !includeRightBoundary){
                int alterWidth = overlapLeftBoundary - currentLeftBoundary;
                currentTile->setWidth(alterWidth);
            }
            // case 2: tile only includes right boundary
            // adjust LL and shrink tile width
            else if (!includeLeftBoundary && includeRightBoundary){
                Cord alterLL = Cord(overlapRightBoundary, currentLowerBoundary);
                int alterWidth = currentRightBoundary - overlapRightBoundary;
                currentTile->setLowerLeft(alterLL);
                currentTile->setWidth(alterWidth);
            }
            // case 3: tile includes entire overlap interval 
            // shrink tile width, create new tile
            else if (includeLeftBoundary && includeRightBoundary){
                int alterWidth = overlapLeftBoundary - currentLeftBoundary;
                currentTile->setWidth(alterWidth);

                Cord newLL = Cord(overlapRightBoundary, currentLowerBoundary);
                int newWidth = currentRightBoundary - overlapRightBoundary;
                int newHeight = currentTile->getHeight();
                Tile* newTile = new Tile(tileType::BLOCK, newLL, newWidth, newHeight);
                TileArr.push_back(newTile);
                // influencedTiles.push_back(newTile);
            }
            // case 4: overlap is same size or wider than tile
            // reduce width to 0, will clean up later
            else {
                currentTile->setWidth(0);
            }
        }

        // step 4: merge up down neighboring tiles with same width 
        for (int t1 = 0; t1 < TileArr.size(); ++t1){
            Tile* tile1 = TileArr[t1];
            if (tile1->getWidth() == 0){
                continue;
            }
            for (int t2 = 0; t2 < TileArr.size(); ++t2){
                Tile* tile2 = TileArr[t2];
                if (tile1 == tile2 || tile2->getWidth() == 0){
                    continue;
                }
                // 1 is on top of 2
                bool canMerge1 = (tile1->getLowerLeft() == tile2->getUpperLeft()) && (tile1->getLowerRight() == tile2->getUpperRight()); 
                // 2 is on top of 1
                bool canMerge2 = (tile1->getUpperLeft() == tile2->getLowerLeft()) && (tile2->getLowerRight() == tile1->getUpperRight()); 
                
                // shrink 2, expand 1
                if (canMerge1){
                    Cord alterLL = tile2->getLowerLeft();
                    int alterHeight = tile1->getHeight() + tile2->getHeight();
                    tile1->setLowerLeft(alterLL);
                    tile1->setHeight(alterHeight);

                    tile2->setWidth(0);
                }
                else if (canMerge2){
                    int alterHeight = tile1->getHeight() + tile2->getHeight();
                    tile1->setHeight(alterHeight);

                    tile2->setWidth(0);
                }
            }
        }

        // step 5: clean up, remove tiles with 0 width
        std::vector<Tile*> deleteTiles;
        for (auto it = TileArr.begin(); it != TileArr.end(); it++){
            if ((*it)->getWidth() == 0){
                deleteTiles.push_back(*it);
                TileArr.erase(it);
                it--;
            }
        }

        for (int i = 0; i < deleteTiles.size(); i++){
            delete deleteTiles[i];
        }
    }
    
}

void Tessera::printCorners(std::ostream& fout){
    typedef gtl::polygon_data<int> Polygon;
    typedef gtl::polygon_90_data<int> Polygon90;
    typedef gtl::polygon_traits<Polygon90>::point_type Point;
    typedef std::vector<gtl::polygon_90_data<int>> Polygon90Set;

    gtl::polygon_90_set_data<int> polygonSet;

    for (int i = 0; i < TileArr.size(); ++i){
        Tile* currentTile = TileArr[i];
        gtl::polygon_90_data<int> boxPolygon;
        const Point box[4] = {
            gtl::construct<Point>(currentTile->getLowerLeft().x, currentTile->getLowerLeft().y),
            gtl::construct<Point>(currentTile->getUpperLeft().x, currentTile->getUpperLeft().y),
            gtl::construct<Point>(currentTile->getUpperRight().x, currentTile->getUpperRight().y),
            gtl::construct<Point>(currentTile->getLowerRight().x, currentTile->getLowerRight().y)
        };
        
        gtl::set_points(boxPolygon, box, box+4);
        gtl::operators::operator+=(polygonSet, boxPolygon); 
    }

    std::vector<gtl::polygon_data<int>> polySetUnionized;
    polySetUnionized.clear();
    polygonSet.get_polygons(polySetUnionized);

    if (polySetUnionized.size() > 1){
        std::cout << "WARNING: Some tiles are disjoint\n";
    }
    else if (polySetUnionized.size() == 0){
        std::cout << "ERROR: TileSet unable to union operation\n";
        return;
    }
    bool isCounterClockwise = gtl::winding(polySetUnionized[0]).to_int() == 1; 

    std::vector<Point> points;
    for (auto it = polySetUnionized[0].begin(); it != polySetUnionized[0].end(); ++it){
        const Point& point_data = *it;
        points.push_back(point_data);
    }

    fout << getName() << ' ' << points.size() << '\n'; 
    if (isCounterClockwise){
        for (int i = points.size() -1 ; i >= 0; --i){
            fout << points[i].x() << ' ' << points[i].y() << '\n';
        }
    }
    else {
        for (int i = 0 ; i < points.size(); i++){
            fout << points[i].x() << ' ' << points[i].y() << '\n';
        }
    }
}

bool Tessera::isLegal() {
    typedef gtl::polygon_90_with_holes_data<len_t> PolygonHole;
    typedef std::vector<PolygonHole>               PolygonHoleSet;
    using namespace gtl::operators;
    PolygonHoleSet curTessSet;

    for ( auto &tile : this->TileArr ) {
        Rectangle box = Rectangle(tile->getLowerLeft().x, tile->getLowerLeft().y, tile->getUpperRight().x, tile->getUpperRight().y);
        curTessSet += box;
    }
    for ( auto &tile : this->OverlapArr ) {
        Rectangle box = Rectangle(tile->getLowerLeft().x, tile->getLowerLeft().y, tile->getUpperRight().x, tile->getUpperRight().y);
        curTessSet += box;
    }

    // check whether this Tessera is connected or not
    if ( curTessSet.size() > 1 ) {
        std::cout << "Fragmented\n";
        return false;
    }

    // check whether this Tessera has holes or not
    PolygonHole curTess = curTessSet[0];
    if ( curTess.begin_holes() != curTess.end_holes() ) {
        std::cout << "Has holes\n";
        return false;
    }

    // check whether this Tessera violates area constraint or not
    if ( gtl::area(curTess) < this->mLegalArea ) {
        std::cout << gtl::area(curTess) << ' ' << this->mLegalArea <<' ';
        std::cout << "Area not legal\n";
        return false;
    }

    // check whether this Tessera violates aspect ratio or not
    Rectangle boundingBox;
    gtl::extents(boundingBox, curTessSet);
    len_t width = gtl::xh(boundingBox) - gtl::xl(boundingBox);
    len_t height = gtl::yh(boundingBox) - gtl::yl(boundingBox);
    double aspectRatio = ((double) width) / ((double) height);
    if ( aspectRatio > 2. || aspectRatio < 0.5 ) {
        std::cout << aspectRatio << ' ';
        std::cout << "aspect ratio not legal\n";
        return false;
    }

    // check whether this Tessera violates rectangle ratio or not
    double rectRatio = (double) gtl::area(curTess) / gtl::area(boundingBox);
    if ( rectRatio < 0.8 ) {
        std::cout << "Util not legal\n";
        return false;
    }

    return true;
}

bool Tessera::isLegal(int &errorCode) {
    typedef gtl::polygon_90_with_holes_data<len_t> PolygonHole;
    typedef std::vector<PolygonHole>               PolygonHoleSet;
    using namespace gtl::operators;
    PolygonHoleSet curTessSet;

    for ( auto &tile : this->TileArr ) {
        Rectangle box = Rectangle(tile->getLowerLeft().x, tile->getLowerLeft().y, tile->getUpperRight().x, tile->getUpperRight().y);
        curTessSet += box;
    }
    for ( auto &tile : this->OverlapArr ) {
        Rectangle box = Rectangle(tile->getLowerLeft().x, tile->getLowerLeft().y, tile->getUpperRight().x, tile->getUpperRight().y);
        curTessSet += box;
    }

    // check whether this Tessera is connected or not
    if ( curTessSet.size() > 1 ) {
        std::cout << "Fragmented\n";
        errorCode = 1;
        return false;
    }

    // check whether this Tessera has holes or not
    PolygonHole curTess = curTessSet[0];
    if ( curTess.begin_holes() != curTess.end_holes() ) {
        std::cout << "Has holes\n";
        errorCode = 2;
        return false;
    }

    // check whether this Tessera violates area constraint or not
    if ( gtl::area(curTess) < this->mLegalArea ) {
        std::cout << gtl::area(curTess) << ' ' << this->mLegalArea <<' ';
        std::cout << "Area not legal\n";
        errorCode = 3;
        return false;
    }

    // check whether this Tessera violates aspect ratio or not
    Rectangle boundingBox;
    gtl::extents(boundingBox, curTessSet);
    len_t width = gtl::xh(boundingBox) - gtl::xl(boundingBox);
    len_t height = gtl::yh(boundingBox) - gtl::yl(boundingBox);
    double aspectRatio = ((double) width) / ((double) height);
    if ( aspectRatio > 2. || aspectRatio < 0.5 ) {
        std::cout << aspectRatio << ' ';
        std::cout << "aspect ratio not legal\n";
        errorCode = 4;
        return false;
    }

    // check whether this Tessera violates rectangle ratio or not
    double rectRatio = (double) gtl::area(curTess) / gtl::area(boundingBox);
    if ( rectRatio < 0.8 ) {
        std::cout << "Util not legal\n";
        errorCode = 5;
        return false;
    }

    return true;
}

std::ostream &operator << (std::ostream &os, const Tessera &tes){
    os << tes.mName << " LA=" << tes.mLegalArea << std::endl;

    os << "TileArr:" << std::endl;
    for(Tile *t : tes.TileArr){
        os << (*t) << std::endl;
    }
    os << "OverlapArr:" << std::endl;
    for(Tile *t : tes.OverlapArr){
        os << (*t) << std::endl;
    }

    return os;
}