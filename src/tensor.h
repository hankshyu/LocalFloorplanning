#define _USE_MATH_DEFINES // for C++
#include <cmath>


namespace tns{
    // returns the angle between the vector and the y-axis  <--PI -- {Y-axis} -- +PI -->
    double calVectorAngle(double from_x, double from_y, double to_x, double to_y);
    //return ABS angle between two angles, -1 if fail
    double calIncludeAngle(double angleA, double angleB);

}