#include <iostream>
#include "Tile.h"

Tile::Tile()
    : type(tileType::BLANK), mLowerLeft(Cord(0,0)), mWidth(0), mHeight(0),
        rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {}

Tile::Tile(tileType t, Cord LL, len_t w, len_t h) 
    : type(t), mLowerLeft(LL), mWidth(w), mHeight(h),
        rt(nullptr), tr(nullptr), bl(nullptr), lb(nullptr) {}

Tile::Tile(const Tile &other)
    : type(other.type), mLowerLeft(other.getLowerLeft()), mWidth(other.getWidth()), mHeight(other.getHeight()),
        rt(other.rt), tr(other.tr), bl(other.bl), lb(other.lb) {
            this->OverlapFixedTesseraeIdx.assign(other.OverlapFixedTesseraeIdx.begin(), other.OverlapFixedTesseraeIdx.end());
            this->OverlapSoftTesseraeIdx.assign(other.OverlapSoftTesseraeIdx.begin(), other.OverlapSoftTesseraeIdx.end());
        }

Tile& Tile::operator = (const Tile &other){
    if(this == &other) return (*this);

    this->type = other.getType();
    this->mLowerLeft = other.getLowerLeft();
    this->mWidth = other.getWidth();
    this->mHeight = other.getHeight();

    this->rt = other.rt;
    this->tr = other.tr;
    this->bl = other.bl;
    this->lb = other.lb;

    this->OverlapFixedTesseraeIdx.assign(other.OverlapFixedTesseraeIdx.begin(), other.OverlapFixedTesseraeIdx.end());
    this->OverlapSoftTesseraeIdx.assign(other.OverlapSoftTesseraeIdx.begin(), other.OverlapSoftTesseraeIdx.end());

    return (*this);
}

void Tile::setType(tileType type) {
    this->type = type;
}

tileType Tile::getType() const {
    return this->type;
}
Cord Tile::getLowerLeft() const {
    return this->mLowerLeft;
};
Cord Tile::getUpperLeft() const {
    return this->mLowerLeft + Cord(0, this->mHeight);
};
Cord Tile::getLowerRight() const {
    return this->mLowerLeft + Cord(this->mWidth, 0);
};
Cord Tile::getUpperRight() const {
    return this->mLowerLeft + Cord(this->mWidth, this->mHeight);
};
void Tile::setLowerLeft(Cord lowerLeft){
    this->mLowerLeft = lowerLeft;
};


len_t Tile::getWidth() const {
    return this->mWidth;
};

len_t Tile::getHeight() const {
    return this->mHeight;
};

void Tile::setCord (Cord cord){
    this->mLowerLeft = cord;
}

void Tile::setWidth(len_t width){
    this->mWidth = width;
};
void Tile::setHeight(len_t height){
    this->mHeight = height;
};

float Tile::getAspectRatio() const {
    return this->mWidth / this->mHeight;
};

area_t Tile::getArea() const {
    return this->mWidth * this->mHeight;
};

bool Tile::operator == (const Tile &comp) const{
    return ((type == comp.getType()) && (mLowerLeft == comp.getLowerLeft()) && (mWidth == comp.getWidth()) && (mHeight == comp.getHeight()));
}

bool Tile::checkTRLLTouch(Tile *right) const{
    Cord rightLL = right->getLowerLeft();
    
    bool xAligned = ((getUpperRight().x) == (rightLL.x));
    bool yInRange = ((rightLL.y >= getLowerRight().y) && (rightLL.y < getUpperRight().y));

    return (xAligned && yInRange);
}

bool Tile::cutHeight(len_t cut) const{
    return ((cut > mLowerLeft.y) && (cut < (mLowerLeft.y + mHeight)));
}

void Tile::show(std::ostream &os) const {
    if(this->type == tileType::BLOCK) os <<"Type: BLOCK ";
    else if(this->type == tileType::BLANK) os <<"Type: BLANK ";
    else if(this->type == tileType::OVERLAP) os <<"Type: OVERLAP ";
    
    os << "(" <<mLowerLeft.x << ", " << mLowerLeft.y << ") ";
    os << "W=" << mWidth <<" H=" << mHeight << std::endl;
}

void Tile::show(std::ostream &os, bool printNewLine) const {
    if(this->type == tileType::BLOCK) os <<"Type: BLOCK ";
    else if(this->type == tileType::BLANK) os <<"Type: BLANK ";
    else if(this->type == tileType::OVERLAP) os <<"Type: OVERLAP ";
    
    os << "(" <<mLowerLeft.x << ", " << mLowerLeft.y << ") ";
    os << "W=" << mWidth <<" H=" << mHeight;
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
    os << "(" << t.mLowerLeft << ", W=" << t.mWidth << ", H=" << t.mHeight;
    os << ", oFt=[";
    for(int i = 0; i < t.OverlapFixedTesseraeIdx.size(); ++i){
        os << t.OverlapFixedTesseraeIdx[i];
        if(i != t.OverlapFixedTesseraeIdx.size() -1) os << ", ";
    }
    os << "], oSt=[";
    for(int i = 0; i < t.OverlapSoftTesseraeIdx.size(); ++i){
        os << t.OverlapSoftTesseraeIdx[i];
        if(i != t.OverlapSoftTesseraeIdx.size() -1) os << ", ";
    }
    os << "])";
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

std::vector<Tile> cutTile(Tile bigTile, Tile smallTile) {
    using namespace boost::polygon::operators;
    Rectangle bigRect(bigTile.getLowerLeft().x, bigTile.getLowerLeft().y, bigTile.getUpperRight().x, bigTile.getUpperRight().y);
    Rectangle smallRect(smallTile.getLowerLeft().x, smallTile.getLowerLeft().y, smallTile.getUpperRight().x, smallTile.getUpperRight().y);
    PolygonSet cutPoly;
    cutPoly += bigRect - smallRect;

    std::vector<Tile> cuttedTiles;
    std::set<len_t> yCord;

    for ( Polygon &poly : cutPoly ) {
        for ( const Point &point : poly ) {
            yCord.insert(point.y());
        }
        std::vector<len_t> yCordVec(yCord.begin(), yCord.end());
        std::sort(yCordVec.begin(), yCordVec.end());
        for ( int i = 0; i < yCordVec.size() - 1; i++ ) {
            int lowY = yCordVec[i];
            int highY = yCordVec[i + 1];
            Rectangle mask(0, lowY, 100000000, highY);
            PolygonSet maskedPoly;
            maskedPoly += poly & mask;

            Rectangle maskedRect;
            gtl::extents(maskedRect, maskedPoly);
            cuttedTiles.push_back(Tile(tileType::OVERLAP, Cord(gtl::xl(maskedRect), gtl::yl(maskedRect)), gtl::xh(maskedRect) - gtl::xl(maskedRect), gtl::yh(maskedRect) - gtl::yl(maskedRect)));
        }
    }

    return cuttedTiles;
}

std::vector<Tile> mergeTile(Tile tile1, Tile tile2) {
    using namespace boost::polygon::operators;
    Rectangle rect1(tile1.getLowerLeft().x, tile1.getLowerLeft().y, tile1.getUpperRight().x, tile1.getUpperRight().y);
    Rectangle rect2(tile2.getLowerLeft().x, tile2.getLowerLeft().y, tile2.getUpperRight().x, tile2.getUpperRight().y);
    PolygonSet mergePoly;
    mergePoly += rect1 + rect2;

    std::vector<Tile> mergedTiles;
    std::set<len_t> yCord;

    for ( Polygon &poly : mergePoly ) {
        for ( const Point &point : poly ) {
            yCord.insert(point.y());
        }
        std::vector<len_t> yCordVec(yCord.begin(), yCord.end());
        std::sort(yCordVec.begin(), yCordVec.end());
        for ( int i = 0; i < yCordVec.size() - 1; i++ ) {
            int lowY = yCordVec[i];
            int highY = yCordVec[i + 1];
            Rectangle mask(0, lowY, 100000000, highY);
            PolygonSet maskedPoly;
            maskedPoly += poly & mask;

            Rectangle maskedRect;
            gtl::extents(maskedRect, maskedPoly);
            mergedTiles.push_back(Tile(tileType::OVERLAP, Cord(gtl::xl(maskedRect), gtl::yl(maskedRect)), gtl::xh(maskedRect) - gtl::xl(maskedRect), gtl::yh(maskedRect) - gtl::yl(maskedRect)));
        }
    }

    return mergedTiles;
}

std::vector<Tile> mergeCutTiles(std::vector<Tile> toMerge, std::vector<Tile> toCut) {
    using namespace boost::polygon::operators;

    PolygonSet manipPoly;
    for ( auto &tile : toMerge ) {
        Rectangle mergeRect(tile.getLowerLeft().x, tile.getLowerLeft().y, tile.getUpperRight().x, tile.getUpperRight().y);
        manipPoly += mergeRect;
    }
    for ( auto &tile : toCut ) {
        Rectangle cutRect(tile.getLowerLeft().x, tile.getLowerLeft().y, tile.getUpperRight().x, tile.getUpperRight().y);
        manipPoly -= cutRect;
    }

    std::vector<Tile> manipTiles;
    std::set<len_t> yCord;

    for ( Polygon &poly : manipPoly ) {
        for ( const Point &point : poly ) {
            yCord.insert(point.y());
        }
        std::vector<len_t> yCordVec(yCord.begin(), yCord.end());
        std::sort(yCordVec.begin(), yCordVec.end());
        for ( int i = 0; i < yCordVec.size() - 1; i++ ) {
            int lowY = yCordVec[i];
            int highY = yCordVec[i + 1];
            Rectangle mask(0, lowY, 100000000, highY);
            PolygonSet maskedPoly;
            maskedPoly += poly & mask;

            Rectangle maskedRect;
            gtl::extents(maskedRect, maskedPoly);
            manipTiles.push_back(Tile(tileType::OVERLAP, Cord(gtl::xl(maskedRect), gtl::yl(maskedRect)), gtl::xh(maskedRect) - gtl::xl(maskedRect), gtl::yh(maskedRect) - gtl::yl(maskedRect)));
        }
    }

    return manipTiles;
}