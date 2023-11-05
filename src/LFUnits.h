#ifndef __LFUNITS_H__
#define __LFUNITS_H__

#include <iostream>
#include "boost/polygon/polygon.hpp"

typedef int len_t;
typedef int area_t;

namespace gtl = boost::polygon;
typedef gtl::rectangle_data<len_t> Rectangle;
typedef gtl::polygon_90_set_data<len_t> Polygon90Set;
typedef gtl::polygon_90_with_holes_data<len_t> Polygon90WithHoles;
typedef gtl::point_data<len_t> Point;

class Cord{
public:
    len_t x;
    len_t y;
    Cord();
    Cord(len_t x_in, len_t y_in);

    Cord& operator = (const Cord &other);

    Cord operator + (const Cord &addend) const;
    Cord operator - (const Cord &subtrahend) const;
    Cord operator * (const len_t &scalar) const;
    
    bool operator == (const Cord &comp) const;
    bool operator != (const Cord &comp) const;
    bool operator < (const Cord &comp) const;
    bool operator <= (const Cord &comp) const;
    bool operator > (const Cord &comp) const;
    bool operator >= (const Cord &comp) const;

    // friend std::ostream &operator << (std::ostream &os, const Cord &c);
    // friend std::ostream &operator << (std::ostream &os, const Cord &c) {
    //     os << "(" << c.x << ", " << c.y << ")";
    //     return os;
    // }

};

std::ostream &operator << (std::ostream &os, const Cord &c);

#endif // __LFUNITS_H__