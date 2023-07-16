#ifndef __LFUNITS_H__
#define __LFUNITS_H__


typedef int len_t;
typedef int area_t;

class Cord{
public:
    len_t x;
    len_t y;
    Cord()
        : x(0), y(0) {}

    Cord(len_t x_in, len_t y_in)
        : x(x_in), y(y_in) {}

    Cord operator+(const Cord &addend) const{
        return Cord(x + addend.x, y + addend.y);
    }

    Cord operator-(const Cord &subtrahend) const{
        return Cord(x - subtrahend.x, y - subtrahend.y);
    }

    Cord operator*(const len_t &scalar) const{
        return Cord(scalar*x, scalar*y);
    }

    bool operator==(const Cord &comp) const{
        return ((x == comp.x) && (y == comp.y));
    }

};

#endif // __LFUNITS_H__