#include <assert.h>
#include "tensor.h"




double tns::calVectorAngle(double from_x, double from_y, double to_x, double to_y){
    
    assert(!((from_x == to_x) && (from_y == to_y)));
    double vect_x = (to_x - from_x);
    double vect_y = (to_y - from_y);

    return std::atan2(vect_x, vect_y);
}

double tns::calIncludeAngle(double angleA, double angleB){
    assert((angleA >= -M_PI) && (angleA <= M_PI));
    assert((angleB >= -M_PI) && (angleB <= M_PI));
    // assert(((angleA >= -M_PI) && (angleA <= M_PI))||(angleA == 127));
    // assert(((angleB >= -M_PI) && (angleB <= M_PI))||(angleB == 127));
    if(angleA == 127 || angleB == 127) return -1;
    double answer = (angleA >= angleB)? (angleA - angleB) : (angleB - angleA);
    if(answer > M_PI){
        answer -= M_PI;
    }

    return answer;
}