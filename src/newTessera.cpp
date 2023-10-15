#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include "newTessera.h"
#include "boost/polygon/polygon.hpp"

Tessera::Tessera(FPManager& FP, tesseraType type, std::string name, area_t area, Cord lowerleft, len_t width, len_t height)
    : mFPM(FP), mType(type), mName(name), mLegalArea(area), 
    mInitLowerLeft(lowerleft), mInitWidth(width), mInitHeight(height) {
        mShape.clear();
        _addArea(lowerleft, width, height);
    }

Tessera::Tessera(const Tessera &other)
    : mFPM(other.mFPM), mType(other.getType()), mName(other.getName()), mLegalArea(other.getLegalArea()),
    mInitLowerLeft(other.getInitLowerLeft()), mInitWidth(other.getInitWidth()), mInitHeight(other.getInitHeight()) {
        mShape = Polygon90Set(other.mShape);
        OverlapArr.assign(other.OverlapArr.begin(), other.OverlapArr.end());
    }

Tessera& Tessera::operator = (const Tessera &other){
    if(this == &other) return (*this);

    this->mFPM = other.mFPM;

    this->mType = other.getType();
    this->mName = other.getName();
    this->mLegalArea = other.getLegalArea();

    this->mInitLowerLeft = other.getInitLowerLeft();
    this->mInitWidth = other.getInitWidth();
    this->mInitHeight = other.getInitHeight();

    mShape = Polygon90Set(other.mShape);
    OverlapArr.assign(other.OverlapArr.begin(), other.OverlapArr.end());
    
    return (*this);
}

bool Tessera::operator ==(const Tessera &tess) const{
    
    if(mType != tess.getType()) return false;
    if(mName != tess.getName()) return false;
    if(mLegalArea != tess.getLegalArea()) return false;
    if(mInitLowerLeft != tess.getInitLowerLeft()) return false;
    if((mInitWidth != tess.getInitWidth()) || (mInitHeight != tess.getInitHeight())) return false;

    if(mShape != tess.mShape) return false;

    if(OverlapArr.size() != tess.OverlapArr.size()) return false;
    for(int i = 0; i < OverlapArr.size(); ++i){
        // TODO: modify this?
        if(OverlapArr[i] != tess.OverlapArr[i]) return false;
    }
    return true;
}

// returns the entire polygon (non-overlapping areas + overlapping areas)
void Tessera::getFullShape(Polygon90Set& poly){
    poly.clear();
    poly += mShape;
    for(int overlapIndex : OverlapArr){
        Polygon90WithHoles& overlap = mFPM.allOverlaps[overlapIndex].getShape();
        poly += overlap;
    }
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
    Polygon90Set shape; 
    getFullShape(shape);
    if(shape.empty()) return;

    Rectangle BBox;
    bg::extents(BBox, shape); 
    Cord BBLL = Cord(bg::xl(BBox), bg::yl(BBox));
    Cord BBUR = Cord(bg::xh(BBox), bg::yh(BBox));

    this->mBBLowerLeft = BBLL;
    this->mBBUpperRight = BBUR;
}

area_t Tessera::calRealArea(){
    Polygon90Set shape; 
    getFullShape(shape);

    return bg::area(shape);
}

area_t Tessera::calAreaMargin() {
    return calRealArea() - this->mLegalArea;
}

void Tessera::splitRectliearDueToOverlap(){
    for(int overlapIndex : OverlapArr){
        Polygon90WithHoles& overlap = mFPM.allOverlaps[overlapIndex].getShape();
        mShape -= overlap;
    }    
}

void Tessera::printCorners(std::ostream& fout){
    Polygon90Set poly;
    std::vector<Polygon90WithHoles> poly90Container;
    poly += mShape;
    if (OverlapArr.size() > 0){
        std::cout << "WARNING: Tess " << mName << " still has overlaps (in Tess::printCorners)\n";
        for (int overlapIndex : OverlapArr){
            Polygon90WithHoles& overlap = mFPM.allOverlaps[overlapIndex].getShape();
            poly += overlap;
        }   
    }

    poly.get(poly90Container);

    if (poly90Container.size() > 1){
        std::cout << "WARNING: Some tiles are disjoint (in Tess::printCorners)\n";
    }
    else if (poly90Container.size() == 0){
        std::cout << "ERROR: TileSet unable to union operation (in Tess::printCorners)\n";
        return;
    }
    bool isCounterClockwise = gtl::winding(poly90Container[0]).to_int() == 1; 

    std::vector<Point> points;
    for (auto it = poly90Container[0].begin(); it != poly90Container[0].end(); ++it){
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
    Polygon90Set poly;
    std::vector<Polygon90WithHoles> poly90Container;
    getFullShape(poly);
    poly.get(poly90Container);
    

    // check whether this Tessera is connected or not
    if ( poly90Container.size() > 1 ) {
        std::cout << "Fragmented\n";
        return false;
    }

    // check whether this Tessera has holes or not
    Polygon90WithHoles tessPolygon = poly90Container[0];
    if ( tessPolygon.size_holes() > 0 ) {
        std::cout << "Has holes\n";
        return false;
    }

    // check whether this Tessera violates area constraint or not
    if ( gtl::area(tessPolygon) < this->mLegalArea ) {
        std::cout << gtl::area(tessPolygon) << ' ' << this->mLegalArea <<' ';
        std::cout << "Area not legal\n";
        return false;
    }

    // check whether this Tessera violates aspect ratio or not
    Rectangle boundingBox;
    gtl::extents(boundingBox, tessPolygon);
    len_t width = gtl::xh(boundingBox) - gtl::xl(boundingBox);
    len_t height = gtl::yh(boundingBox) - gtl::yl(boundingBox);
    double aspectRatio = ((double) width) / ((double) height);
    if ( aspectRatio > 2. || aspectRatio < 0.5 ) {
        std::cout << aspectRatio << ' ';
        std::cout << "aspect ratio not legal\n";
        return false;
    }

    // check whether this Tessera violates rectangle ratio or not
    double rectRatio = (double) gtl::area(tessPolygon) / gtl::area(boundingBox);
    if ( rectRatio < 0.8 ) {
        std::cout << "Util not legal\n";
        return false;
    }

    return true;
}

bool Tessera::isLegal(int &errorCode) {
    Polygon90Set poly;
    std::vector<Polygon90WithHoles> poly90Container;
    getFullShape(poly);
    poly.get(poly90Container);
    

    // check whether this Tessera is connected or not
    if ( poly90Container.size() > 1 ) {
        std::cout << "Fragmented\n";
        errorCode = 1;
        return false;
    }

    // check whether this Tessera has holes or not
    Polygon90WithHoles tessPolygon = poly90Container[0];
    if ( tessPolygon.size_holes() > 0 ) {
        std::cout << "Has holes\n";
        errorCode = 2;
        return false;
    }

    // check whether this Tessera violates area constraint or not
    if ( gtl::area(tessPolygon) < this->mLegalArea ) {
        std::cout << gtl::area(tessPolygon) << ' ' << this->mLegalArea <<' ';
        std::cout << "Area not legal\n";
        errorCode = 3;
        return false;
    }

    // check whether this Tessera violates aspect ratio or not
    Rectangle boundingBox;
    gtl::extents(boundingBox, tessPolygon);
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
    double rectRatio = (double) gtl::area(tessPolygon) / gtl::area(boundingBox);
    if ( rectRatio < 0.8 ) {
        std::cout << "Util not legal\n";
        errorCode = 5;
        return false;
    }

    return true;
}


void Tessera::_addArea(Cord lowerleft, len_t width, len_t height){
    Polygon90WithHoles newArea;
    std::vector<Point> newAreaVertices = {
        {lowerleft.x, lowerleft.y}, 
        {lowerleft.x + width, lowerleft.y }, 
        {lowerleft.x + width, lowerleft.y + height}, 
        {lowerleft.x, lowerleft.y + height}
    };
    bg::set_points(newArea, newAreaVertices.begin(), newAreaVertices.end());
    
    // check for overlaps, output warning if overlap exists
    Polygon90Set overlap = newArea & mShape;
    if (overlap.empty())

    mShape += newArea;
    if (_checkHole()){
        std::cout << "WARNING: Hole in Tessera " << getName() << std::endl;
    }
}

bool Tessera::_checkHole(){
    std::vector<Polygon90WithHoles> poly90Container;
    mShape.get_polygons(poly90Container);
    for (auto it = poly90Container.begin(); it != poly90Container.end(); ++it) {
        if (it->size_holes() > 0){
            return true;
        }
    }
    return false;
}

std::ostream &operator << (std::ostream &os, const Tessera &tes){
    os << tes.mName << " LA=" << tes.mLegalArea << std::endl;

    os << "mShape:" << std::endl;
    os << tes.mShape << std::endl;

    os << "OverlapArr:" << std::endl;
    for(int overlapIndex : tes.OverlapArr){
        Polygon90WithHoles& overlap = tes.mFPM.allOverlaps[overlapIndex].getShape();
        os << overlap << std::endl;
    }

    return os;
}

std::ostream &operator << (std::ostream &o, const Point &pt) {
    o << "(" << gtl::x(pt) << ", " << gtl::y(pt) << ")";
    return o;
}

std::ostream &operator << (std::ostream &o, const Polygon &poly) {
    o << "poly (";

    for ( Point pt : poly ) {
        o << pt << ", ";
    }
    o << ")";
    return o;
}

std::ostream &operator << (std::ostream &o, const PolygonSet &polys) {
    for ( const Polygon &poly : polys ) {
        o << poly << "\n";
    }
    return o;
}

Overlap::Overlap(Polygon90WithHoles& shape, std::vector<int> overlapIndices): 
mShape(shape), overlaps(overlapIndices) {}

Polygon90WithHoles& Overlap::getShape(){
    return mShape;
}