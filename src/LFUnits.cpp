#include "LFUnits.h"
#include <iostream>


Cord::Cord()
    : x(0), y(0) 
    {}

Cord::Cord(len_t x_in, len_t y_in)
    : x(x_in), y(y_in) 
    {}

Cord& Cord::operator = (const Cord &other){
    if(this == &other) return (*this);
    this->x = other.x;
    this->y = other.y;

    return (*this);
}
Cord Cord::operator + (const Cord &addend) const{
    return Cord(x + addend.x, y + addend.y);
}

Cord Cord::operator - (const Cord &subtrahend) const{
    return Cord(x - subtrahend.x, y - subtrahend.y);
}

Cord Cord::operator * (const len_t &scalar) const{
    return Cord(scalar*x, scalar*y);
}

bool Cord::operator == (const Cord &comp) const{
    return ((x == comp.x) && (y == comp.y));
}

bool Cord::operator != (const Cord &comp) const{
    return ((x != comp.x) || (y != comp.y));
}

bool Cord::operator < (const Cord &comp) const{
    return ((x < comp.x) && (y < comp.y));
}

bool Cord::operator <= (const Cord &comp) const{
    return ((x <= comp.x) && (y <= comp.y));
}

bool Cord::operator > (const Cord &comp) const{
    return ((x > comp.x) && (y > comp.y));
}

bool Cord::operator >= (const Cord &comp) const{
    return ((x >= comp.x) && (y >= comp.y));
}

std::ostream &operator << (std::ostream &os, const Cord &c) {
    os << "(" << c.x << ", " << c.y << ")";
    return os;
}