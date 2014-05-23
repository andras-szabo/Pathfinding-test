#ifndef __small_astartest__nodeID__
#define __small_astartest__nodeID__

#include <SFML/Graphics.hpp>

struct cNodeID {
    cNodeID();
    cNodeID(int, int);
    cNodeID(sf::Vector2i);
    
    cNodeID& operator=(const cNodeID& rhs)
    {
        if ( &rhs != this )
        {
            x = rhs.x;
            y = rhs.y;
            valid = rhs.valid;
        }
        return *this;
    }
    
    int         x;
    int         y;
    bool        valid;
};

bool operator<(const cNodeID&, const cNodeID&);
bool operator>(const cNodeID&, const cNodeID&);
bool operator==(const cNodeID&, const cNodeID&);
bool operator!=(const cNodeID&, const cNodeID&);

cNodeID operator-(const cNodeID& a, const cNodeID& b);
cNodeID operator+(const cNodeID& a, const cNodeID& b);

#endif /* defined(__small_astartest__nodeID__) */
