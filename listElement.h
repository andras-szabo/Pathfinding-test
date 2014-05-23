#ifndef __small_astartest__listElement__
#define __small_astartest__listElement__


#include "nodeID.h"

struct listElement {
    listElement() { }
    
    listElement(cNodeID i, unsigned int fs):
    id { i },
    fScore {fs }
    {
        
    }
    
    cNodeID         id;
    unsigned int    fScore;
};

bool operator<(const listElement& a, const listElement& b);
bool operator>(const listElement& a, const listElement& b);
bool operator<=(const listElement& a, const listElement& b);
bool operator>=(const listElement& a, const listElement& b);
bool operator==(const listElement& a, const listElement& b);
bool operator!=(const listElement& a, const listElement& b);
#endif /* defined(__small_astartest__listElement__) */
