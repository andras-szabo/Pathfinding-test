#ifndef __small_astartest__board__
#define __small_astartest__board__

#include <vector>
#include "enums.h"
#include "nodeID.h"

struct cField {
    cField():
        status { cStatus::walkable },
        marked { false } {}
    
    cStatus         status;
    bool            marked;
    int             whichList;
    cNodeID         parent;
    unsigned int    gScore;
    unsigned int    hScore;
};

#endif /* defined(__small_astartest__board__) */
