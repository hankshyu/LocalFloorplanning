#include <assert.h>
#include "tensor.h"




double tns::calVectorAngle(double from_x, double from_y, double to_x, double to_y){
    
    assert(!((from_x == to_x) && (from_y == to_y)));
    double vect_x = (to_x - from_x);
    double vect_y = (to_y - from_y);

    return std::atan2(vect_x, vect_y);
}