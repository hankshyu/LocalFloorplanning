#include <iostream>
#include "newTile.h"

Tile::Tile()
    : type(tileType::BLANK), mTessIndex(-1),
        rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {
            mRectangle = gtl::construct<Rectangle,int>(0, 0, 0, 0);
        }

// Tile::Tile(tileType t, Cord LL, len_t w, len_t h) 
//     : type(t), mLowerLeft(LL), mWidth(w), mHeight(h),
//         rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {}

Tile::Tile(tileType t, Rectangle& r, int tessIndex = -1) 
    : type(t), mTessIndex(tessIndex), rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {
        mRectangle = r;
    }

Tile::Tile(const Tile &other)
    : type(other.type), mRectangle(other.mRectangle), mTessIndex(other.mTessIndex), rt(other.rt), tr(other.tr), bl(other.bl), lb(other.lb) {  }

Tile& Tile::operator = (const Tile &other){
    if(this == &other) return (*this);

    this->type = other.type;
    this->mTessIndex = other.mTessIndex;
    this->mRectangle = other.mRectangle;
    
    this->rt = other.rt;
    this->tr = other.tr;
    this->bl = other.bl;
    this->lb = other.lb;

    return (*this);
}

void Tile::setType(tileType type) {
    this->type = type;
}

tileType Tile::getType() const {
    return this->type;
}

int Tile::getTessIndex() const {
    return this->mTessIndex;
}

Rectangle Tile::getRectangle() const {
    return this->mRectangle;
}

Cord Tile::getLowerLeft() const {
    return Cord(gtl::xl(mRectangle), gtl::yl(mRectangle));
};
Cord Tile::getUpperLeft() const {
    return Cord(gtl::xl(mRectangle), gtl::yh(mRectangle));
};
Cord Tile::getLowerRight() const {
    return Cord(gtl::xh(mRectangle), gtl::yl(mRectangle));
};
Cord Tile::getUpperRight() const {
    return Cord(gtl::xh(mRectangle), gtl::yh(mRectangle));
};
void Tile::setLowerLeft(Cord lowerLeft){
    int xDist = lowerLeft.x - gtl::ll(mRectangle).x();
    int yDist = lowerLeft.y - gtl::ll(mRectangle).y();

    gtl::move(mRectangle, gtl::orientation_2d_enum::HORIZONTAL, xDist); 
    gtl::move(mRectangle, gtl::orientation_2d_enum::VERTICAL, yDist); 
};

len_t Tile::getWidth() const {
    return gtl::delta(mRectangle, gtl::orientation_2d_enum::HORIZONTAL);
};

len_t Tile::getHeight() const {
    return gtl::delta(mRectangle, gtl::orientation_2d_enum::HORIZONTAL);
};

void Tile::setTessOwnership(int tessIndex){
    mTessIndex = tessIndex;
}

void Tile::setWidth(len_t width){
    gtl::xh(mRectangle, gtl::xl(mRectangle)+width);
};
void Tile::setHeight(len_t height){
    gtl::yh(mRectangle, gtl::yl(mRectangle)+height);
};

float Tile::getAspectRatio() const {
    return (float)getWidth() / (float)getHeight();
};

area_t Tile::getArea() const {
    return gtl::area(mRectangle);
};

bool Tile::operator == (const Tile &comp) const{
    return ((type == comp.getType()) && (mRectangle == comp.mRectangle));
}

inline bool Tile::checkXCordInTile(const Cord &point) const{
    // return (point.x >= this->mLowerLeft.x) && (point.x < this->mLowerLeft.x + this->mWidth);
    return gtl::contains(gtl::horizontal(mRectangle), point.x) && (point.x != gtl::horizontal(mRectangle).high()); 
}

inline bool Tile::checkYCordInTile(const Cord &point) const{
    return gtl::contains(gtl::vertical(mRectangle), point.y) && (point.y != gtl::vertical(mRectangle).high()); 
}

inline bool Tile::checkCordInTile(const Cord &point) const{
    return gtl::contains(mRectangle, Point(point.x,point.y)) && (point.x != gtl::horizontal(mRectangle).high()) && (point.y != gtl::vertical(mRectangle).high());
}

bool Tile::checkTRLLTouch(Tile *right) const{
    Cord rightLL = right->getLowerLeft();
    
    bool xAligned = ((getUpperRight().x) == (rightLL.x));
    bool yInRange = ((rightLL.y >= getLowerRight().y) && (rightLL.y < getUpperRight().y));

    return (xAligned && yInRange);
}

bool Tile::cutHeight(len_t cut) const{
    return ((cut > getLowerLeft().y) && (cut < getUpperLeft().y));
}

void Tile::show(std::ostream &os) const {
    if(this->type == tileType::BLOCK) os <<"Type: BLOCK ";
    else if(this->type == tileType::BLANK) os <<"Type: BLANK ";
    else if(this->type == tileType::OVERLAP) os <<"Type: OVERLAP ";
    
    os << "(" << getLowerLeft().x << ", " << getLowerLeft().y << ") ";
    os << "W=" << getWidth() <<" H=" << getHeight() << std::endl;
}

void Tile::show(std::ostream &os, bool printNewLine) const {
    if(this->type == tileType::BLOCK) os <<"Type: BLOCK ";
    else if(this->type == tileType::BLANK) os <<"Type: BLANK ";
    else if(this->type == tileType::OVERLAP) os <<"Type: OVERLAP ";
    
    os << "(" << getLowerLeft().x << ", " << getLowerLeft().y << ") ";
    os << "W=" << getWidth() <<" H=" << getHeight() << std::endl;
    if(printNewLine) os << std::endl;
}

void Tile::showLink(std::ostream &os) const{
    os << "rt: ";
    if(this->rt == nullptr) os << "nullptr" << std::endl;
    else this->rt->show(os);

    os << "tr: ";
    if(this->tr == nullptr) os << "nullptr" << std::endl;
    else this->tr->show(os);

    os << "bl: ";
    if(this->bl == nullptr) os << "nullptr" << std::endl;
    else this->bl->show(os);

    os << "lb: ";
    if(this->lb == nullptr) os << "nullptr" << std::endl;
    else this->lb->show(os);
    os << std::endl;
}

std::ostream &operator << (std::ostream &os, const Tile &t){
    os << "(" << t.getLowerLeft() << ", W=" << t.getWidth() << ", H=" << t.getHeight();
    os << ", ID=" << t.getTessIndex();
    return os;
}

std::ostream &operator << (std::ostream &o, const Point &pt) {
    o << "(" << gtl::x(pt) << ", " << gtl::y(pt) << ")";
    return o;
}

std::ostream &operator << (std::ostream &o, const Polygon90WithHoles &poly) {
    o << "poly (";

    for ( Point pt : poly ) {
        o << pt << ", ";
    }
    o << ")";
    return o;
}

std::ostream &operator << (std::ostream &o, const Polygon90Set &polys) {
    std::vector<Polygon90WithHoles> poly90Container;
    polys.get(poly90Container);
    for ( const Polygon90WithHoles &poly : poly90Container ) {
        o << poly << "\n";
    }
    return o;
}

std::ostream &operator << (std::ostream &o, const Rectangle &rect) {
    o << "poly (";
    o << gtl::ll(rect) << ", " << gtl::lr(rect) << ", " << gtl::ur(rect) << ", " << gtl::ul(rect);
    o << ")";
    return o;
}

std::vector<Tile> cutTile(Tile bigTile, Tile smallTile) {
    using namespace boost::polygon::operators;
    
    Polygon90Set cutPoly;
    cutPoly += bigTile.getRectangle() - smallTile.getRectangle();

    std::vector<Tile> cuttedTiles;

    std::vector<Rectangle> rectangleContainer;
    gtl::get_rectangles(rectangleContainer, cutPoly);

    for (Rectangle& rect : rectangleContainer){
        cuttedTiles.push_back(Tile(tileType::OVERLAP, rect));
    }

    // for ( Polygon &poly : cutPoly ) {
        
    //     for ( const Point &point : poly ) {
    //         yCord.insert(point.y());
    //     }
    //     std::vector<len_t> yCordVec(yCord.begin(), yCord.end());
    //     std::sort(yCordVec.begin(), yCordVec.end());
    //     for ( int i = 0; i < yCordVec.size() - 1; i++ ) {
    //         int lowY = yCordVec[i];
    //         int highY = yCordVec[i + 1];
    //         Rectangle mask(0, lowY, 100000000, highY);
    //         PolygonSet maskedPoly;
    //         maskedPoly += poly & mask;

    //         Rectangle maskedRect;
    //         gtl::extents(maskedRect, maskedPoly);
    //         cuttedTiles.push_back(Tile(tileType::OVERLAP, Cord(gtl::xl(maskedRect), gtl::yl(maskedRect)), gtl::xh(maskedRect) - gtl::xl(maskedRect), gtl::yh(maskedRect) - gtl::yl(maskedRect)));
    //     }
    // }

    return cuttedTiles;
}

std::vector<Tile> mergeTile(Tile tile1, Tile tile2) {
    using namespace boost::polygon::operators;
    
    Polygon90Set mergePoly;
    mergePoly += tile1.getRectangle() + tile2.getRectangle();

    std::vector<Tile> mergedTiles;

    std::vector<Rectangle> rectangleContainer;
    gtl::get_rectangles(rectangleContainer, mergePoly);

    for (Rectangle& rect : rectangleContainer){
        mergedTiles.push_back(Tile(tileType::OVERLAP, rect));
    }

    // for ( Polygon &poly : mergePoly ) {
    //     for ( const Point &point : poly ) {
    //         yCord.insert(point.y());
    //     }
    //     std::vector<len_t> yCordVec(yCord.begin(), yCord.end());
    //     std::sort(yCordVec.begin(), yCordVec.end());
    //     for ( int i = 0; i < yCordVec.size() - 1; i++ ) {
    //         int lowY = yCordVec[i];
    //         int highY = yCordVec[i + 1];
    //         Rectangle mask(0, lowY, 100000000, highY);
    //         PolygonSet maskedPoly;
    //         maskedPoly += poly & mask;

    //         Rectangle maskedRect;
    //         gtl::extents(maskedRect, maskedPoly);
    //         mergedTiles.push_back(Tile(tileType::OVERLAP, Cord(gtl::xl(maskedRect), gtl::yl(maskedRect)), gtl::xh(maskedRect) - gtl::xl(maskedRect), gtl::yh(maskedRect) - gtl::yl(maskedRect)));
    //     }
    // }

    return mergedTiles;
}

std::vector<Tile> mergeCutTiles(std::vector<Tile> toMerge, std::vector<Tile> toCut) {
    using namespace boost::polygon::operators;

    Polygon90Set manipPoly;
    for ( Tile &tile : toMerge ) {
        manipPoly += tile.getRectangle();
    }
    for ( Tile &tile : toCut ) {
        manipPoly -= tile.getRectangle();
    }

    std::vector<Tile> manipTiles;

    std::vector<Rectangle> rectangleContainer;
    gtl::get_rectangles(rectangleContainer, manipPoly);

    for (Rectangle& rect : rectangleContainer){
        manipTiles.push_back(Tile(tileType::OVERLAP, rect));
    }

    return manipTiles;
}