#include "nodeID.h"

cNodeID::cNodeID():
x { 0 }, y { 0 }, valid { true } { }

cNodeID::cNodeID(int a, int b):
x { a }, y { b }, valid { true } { }

cNodeID::cNodeID(sf::Vector2i v):
x { v.x }, y { v.y }, valid { true } { }

bool operator<(const cNodeID& a, const cNodeID& b)
{
    return a.x < b.x;
}

bool operator>(const cNodeID& a, const cNodeID& b)
{
    return a.x > b.x;
}

bool operator==(const cNodeID& a, const cNodeID& b)
{
    return a.x == b.x && a.y == b.y;
}

bool operator!=(const cNodeID& a, const cNodeID& b)
{
    return a.x != b.x || a.y != b.y;
}

cNodeID operator-(const cNodeID& a, const cNodeID& b)
{
    return cNodeID { static_cast<int>(a.x - b.x), static_cast<int>(a.y - b.y) };
}

cNodeID operator+(const cNodeID& a, const cNodeID& b)
{
    return cNodeID { static_cast<int>(a.x + b.x), static_cast<int>(a.y + b.y) };
}
