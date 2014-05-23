#include "listElement.h"

bool operator<(const listElement& a, const listElement& b)
{
    return a.fScore < b.fScore;
}

bool operator>(const listElement& a, const listElement& b)
{
    return a.fScore > b.fScore;
}

bool operator<=(const listElement& a, const listElement& b)
{
    return a.fScore <= b.fScore;
}

bool operator>=(const listElement& a, const listElement& b)
{
    return a.fScore >= b.fScore;
}

bool operator==(const listElement& a, const listElement& b)
{
    return (a.fScore == b.fScore && a.id == b.id);
}

bool operator!=(const listElement& a, const listElement& b)
{
    return (a.fScore != b.fScore || a.id != b.id);
}