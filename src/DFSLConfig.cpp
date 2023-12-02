#include "DFSLConfig.h"

Config::Config(): 
    maxCostCutoff(1000000.0),

    OBAreaWeight(750.0),
    OBUtilWeight(1000.0),
    OBAspWeight(100.0),
    OBUtilPosRein(-500.0),

    BWUtilWeight(1500.0),
    BWUtilPosRein(-500.0),
    BWAspWeight(500.0),

    BBAreaWeight(150.0),
    BBFromUtilWeight(700.0),
    BBFromUtilPosRein(-500.0),    
    BBToUtilWeight(1000.0),
    BBToUtilPosRein(-100.0),
    BBAspWeight(50.0),
    BBFlatCost(90.0),

    WWFlatCost(1000000.0) 
    { ; }