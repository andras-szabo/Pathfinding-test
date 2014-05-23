#include "pathfinder.h"
#include "nodeID.h"
#include <cmath>
#include <cassert>
#include <iostream>

int cPathFinder::UID = 1;

inline bool cPathFinder::onCList(const cNodeID& id) const
{
    if (!valid(id.x, id.y)) return false;
    return mBoard[id.x][id.y].whichList == -UID;
}

inline bool cPathFinder::onOList(const cNodeID& id) const
{
    return valid(id.x, id.y) && mBoard[id.x][id.y].whichList == UID;
}

// 500: size of the view
cPathFinder::cPathFinder(unsigned int x, unsigned int y):
mTileSize { 500 / x, 500 / y },
mBoardSize { x, y }
{
    std::vector<cField>     col(mBoardSize.y);
    for(auto i = 0; i < mBoardSize.x; ++i)
    {
        mBoard.push_back(col);
    }

    for(auto j = 0; j < mBoardSize.y; ++j)
        for(auto i = 0; i < mBoardSize.x; ++i)
        {
            float left = i * mTileSize.x;
            float top = j * mTileSize.y;
            mGrid.push_back(sf::Vertex(sf::Vector2f(left, top), sf::Color::White));
            mGrid.push_back(sf::Vertex(sf::Vector2f(left + mTileSize.x, top), sf::Color::White));
            mGrid.push_back(sf::Vertex(sf::Vector2f(left + mTileSize.x,
                                                    top + mTileSize.y), sf::Color::White));
            mGrid.push_back(sf::Vertex(sf::Vector2f(left, top + mTileSize.y), sf::Color::White));
        }
        
    q.setPred([](const listElement& a, const listElement& b) { return a < b; });
}

unsigned int cPathFinder::calcHscore(const cNodeID& from,
                                     const cNodeID& to) const
{
    /* Distance - estimating heuristic comes here */
    return (abs(to.x-from.x) * 10 + abs(to.y-from.y) * 10);
}

unsigned int cPathFinder::calcGscore(const cNodeID& from,
                                     const cNodeID& to) const
{
    
    // If not immedately next to the node, we calculate gScore simply as distance,
    // PLUS the original gScore of the parent.
    if ( abs(from.x - to.x) + abs(from.y - to.y) > 2 )
    {
        return mBoard[from.x][from.y].gScore + sqrt(pow(from.x - to.x, 2) + pow(from.y - to.y, 2)) * 10;
    }

    if ( from == to ) return 0;
    if ( from.x != to.x && from.y != to.y )
        return mBoard[from.x][from.y].gScore + 14;

    return mBoard[from.x][from.y].gScore + 10;
}

void cPathFinder::addToOpenList(const cNodeID& target,
                                const cNodeID& parent,
                                const cNodeID& end)
{
    // Adding to the open list includes:
    // - setting whichList
    // - calculating g and f scores
    // - setting parent.
    // - actually adding it to the open list.
    
    /* ... error checking ... */
    auto x = target.x, y = target.y;
    
    if (!valid(x,y)) return;
    
    mBoard[x][y].whichList = UID;
    mBoard[x][y].parent = parent;
    mBoard[x][y].gScore = calcGscore(parent, target);   // from -> to
    mBoard[x][y].hScore = calcHscore(target, end);      // from -> to
    
    q.push(listElement(target, mBoard[x][y].gScore + mBoard[x][y].hScore));
}

void cPathFinder::addToClosedList(const cNodeID& id)
{
    if ( mBoard[id.x][id.y].whichList == UID )
        mBoard[id.x][id.y].whichList = -UID;
    else
        throw std::runtime_error("Trying to add unopened to closed list.");
}

bool cPathFinder::valid(long x, long y) const
{
    return x >= 0 && x < mBoardSize.x && y >= 0 && y < mBoardSize.y;
}

bool cPathFinder::blocked(long int x, long int y) const
{
    if (!valid(x,y)) return true;
    return mBoard[x][y].status == cStatus::blocked;
}

std::vector<cNodeID> cPathFinder::adjacent(const cNodeID& id,
                                           bool cornerCuttingAllowed) const
{
    std::vector<cNodeID> ret;
    for ( auto i = -1; i < 2; ++i)
        for ( auto j = -1; j < 2; ++j )
            if ( i != 0 || j != 0 )
            {
                if ( valid(id.x + i, id.y + j) &&
                    mBoard[id.x + i][id.y+j].status != cStatus::blocked )
                {
                    if ( cornerCuttingAllowed )
                    {
                        ret.push_back(cNodeID { id.x + i, id.y + j });
                    }
                    else // no corner cutting
                    {
                        bool add = true;

                        // Top left corner
                        if ( i == -1 && j == -1 &&
                            (mBoard[id.x-1][id.y].status == cStatus::blocked ||
                             mBoard[id.x][id.y-1].status == cStatus::blocked) )
                                add = false;
                        
                        // Top right corner
                        if ( i == 1 && j == -1 &&
                            (mBoard[id.x+1][id.y].status == cStatus::blocked ||
                            mBoard[id.x][id.y-1].status == cStatus::blocked))
                                add = false;
                        
                        // Bottom left corner
                        if ( i == -1 && j == 1 &&
                            (mBoard[id.x-1][id.y].status == cStatus::blocked ||
                             mBoard[id.x][id.y+1].status == cStatus::blocked))
                                add = false;
                        
                        // Bottom right corner
                        if ( i == 1 && j == 1 &&
                            (mBoard[id.x+1][id.y].status == cStatus::blocked ||
                             mBoard[id.x][id.y+1].status == cStatus::blocked))
                                add = false;
                        
                        if ( add )
                            ret.push_back(cNodeID { id.x+i, id.y+j });
                    }
                }
            }
    return ret;
}

std::vector<cNodeID> cPathFinder::successors(const cNodeID& target,
                                             const cNodeID& start,
                                             const cNodeID& goal,
                                             bool cornerCutting)
{

    // So what's going to happen here? We consider the node's successors - not-necessarily-adjacent
    // neighbours. A simple way to decide which neighbours are interesting is to pretend that
    // we're always arriving at the node coming from the left (when moving in a straight line),
    // or from the left and from bottom (when moving diagonally). So we create a small matrix
    // representing the node and its immediate neighbours, and then rotate that matrix so that
    // "coming from left" becomes true. It's a bit of fuss with the matrix, but it greatly
    // simplifies conditional evaluation - we only have to check 2 or 3 neighbours instead of 8.
    
    auto parent = mBoard[target.x][target.y].parent;

    if (parent == target) return adjacent(target, cornerCutting);
    
    std::vector<cNodeID> succ;
    
    // First, let's set up the 3x3 matrix that represents the
    // node we're currently examining, and its 8 neighbours
    // (0, 1, 2,
    //  3, 4, 5,
    //  6, 7, 8),
    // currently examined node in the centre, under index 4.
    
    mMatrix[4].x = target.x;
    mMatrix[4].y = target.y;
    
    mMatrix[0].x = target.x - 1;    mMatrix[0].y = target.y - 1;
    mMatrix[1].x = target.x;        mMatrix[1].y = target.y - 1;
    mMatrix[2].x = target.x + 1;    mMatrix[2].y = target.y - 1;
    
    mMatrix[3].x = target.x - 1;    mMatrix[3].y = target.y;
    mMatrix[5].x = target.x + 1;    mMatrix[5].y = target.y;
    
    mMatrix[6].x = target.x - 1;    mMatrix[6].y = target.y + 1;
    mMatrix[7].x = target.x;        mMatrix[7].y = target.y + 1;
    mMatrix[8].x = target.x + 1;    mMatrix[8].y = target.y + 1;
    
    for (auto& i : mMatrix)
    {
        if (i.x < 0 || i.x >= mBoardSize.x ||
            i.y < 0 || i.y >= mBoardSize.y ||
            mBoard[i.x][i.y].status == cStatus::blocked) i.ok = false;
        else
            i.ok = true;
    }
    
    // Now, which direction are we coming from, and is it straight or diagonal?
    
    int dx = target.x - parent.x;
    int dy = target.y - parent.y;
    
    if ( dx == 0 || dy == 0 )   // we're coming from straight direction!
    {
        if ( dx > 0 )
        {
            // We're coming from the left, so nothing to do here.
        }
        if ( dx < 0 )   // we're coming from the right, so we need to
                        // rotate the matrix by 180 degrees
        {
            twoints tmp[9];
            for (auto i = 0; i < 9; ++i)
                tmp[i] = mMatrix[8 - i];
    
            for (auto i = 0; i < 9; ++i)
                mMatrix[i] = tmp[i];
        }

        if ( dy > 0 )   // We're coming from up, so we need to rotate the
                        // matrix 90 egrees to the left (counterclockwise)
        {
            twoints tmp[9];
            for (auto i = 0; i < 3; ++i)
            {
                tmp[i * 3] = mMatrix[2 - i];        // 0, 3, 6 => 2, 1, 0
                tmp[1 + i * 3] = mMatrix[5 - i];    // 1, 4, 7 => 5, 4, 3
                tmp[2 + i * 3] = mMatrix[8 - i];    // 2, 5, 8 => 8, 7, 6
            }

            for (auto i = 0; i < 9; ++i)
                mMatrix[i] = tmp[i];
        }
        
        if ( dy < 0 )   // We're coming from down, so we need to rotate 90
                        // degrees to the right (clockwise)
        {
            twoints tmp[9];
            for (auto i = 0; i < 3; ++i)
            {
                tmp[i * 3] = mMatrix[6 + i];    // 0, 3, 6 =>6, 7, 8
                tmp[1 + i * 3] = mMatrix[3 + i];
                tmp[2 + i * 3] = mMatrix[i];
            }
            
            for (auto i = 0; i < 9; ++i)
                mMatrix[i] = tmp[i];
        }
        
        cNodeID tmp;

        // Node: 2.
        // It's to be added if: itself is OK, and 1 is blocked and 5 is walkable.
        // . x 2
        // . N .
        // . . .
        if ( mMatrix[2].ok && !mMatrix[1].ok && (cornerCutting || mMatrix[5].ok ))
        {
            tmp.x = mMatrix[2].x;
            tmp.y = mMatrix[2].y;
            succ.push_back(tmp);
        }
            
        // Node 8: mirror image of 2
        if ( mMatrix[8].ok && !mMatrix[7].ok && (cornerCutting || mMatrix[5].ok ))
        {
            tmp.x = mMatrix[8].x;
            tmp.y = mMatrix[8].y;
            succ.push_back(tmp);
        }
            
        // Node: 5. This is the only natural neighbour if we're moving straight;
        // we always add it, without checking g scores -> so, again, this cannot
        // account for variable terrain costs.
        if ( mMatrix[5].ok )
        {
            tmp.x = mMatrix[5].x;
            tmp.y = mMatrix[5].y;
            succ.push_back(tmp);
        }
    
    } // End of condition: we're coming from a straight direction
    else            // We're coming here diagonally.
    {
        if ( dx > 0 && dy < 0)
        {
            // we're coming from bottom left, do nothing
        }
        if ( dx > 0 && dy > 0 ) // rotate left (counterclockwise)
        {
            twoints tmp[9];
            for (auto i = 0; i < 3; ++i)
            {
                tmp[i * 3] = mMatrix[2 - i];        // 0, 3, 6 => 2, 1, 0
                tmp[1 + i * 3] = mMatrix[5 - i];    // 1, 4, 7 => 5, 4, 3
                tmp[2 + i * 3] = mMatrix[8 - i];    // 2, 5, 8 => 8, 7, 6
            }
            
            for (auto i = 0; i < 9; ++i)
                mMatrix[i] = tmp[i];
        }
        if ( dx < 0 && dy < 0 ) // rotate right (clockwise)
        {
            twoints tmp[9];
            for (auto i = 0; i < 3; ++i)
            {
                tmp[i * 3] = mMatrix[6 + i];    // 0, 3, 6 =>6, 7, 8
                tmp[1 + i * 3] = mMatrix[3 + i];
                tmp[2 + i * 3] = mMatrix[i];
            }
            
            for (auto i = 0; i < 9; ++i)
                mMatrix[i] = tmp[i];
        }
        if ( dx < 0 && dy > 0)  // we're coming from top right, we need to rotate 180 degrees
        {
            twoints tmp[9];
            for (auto i = 0; i < 9; ++i)
                tmp[i] = mMatrix[8 - i];
            
            for (auto i = 0; i < 9; ++i)
                mMatrix[i] = tmp[i];
        }

        cNodeID tmp;
        
        // First deal with the natural neighbours: 1, 2, and 5.

        if ( mMatrix[1].ok )
        {
            tmp.x = mMatrix[1].x;
            tmp.y = mMatrix[1].y;
            succ.push_back(tmp);
        }
        
        
        if ( mMatrix[2].ok )
        {
            tmp.x = mMatrix[2].x;
            tmp.y = mMatrix[2].y;
            succ.push_back(tmp);
        }
        
        if ( mMatrix[5].ok )
        {
            tmp.x = mMatrix[5].x;
            tmp.y = mMatrix[5].y;
            succ.push_back(tmp);
        }
        
        // Now let's look at the forced neighbours, possibly 0 and 8.
        // 0 needs added if itself is ok, and 3 is blocked
        
        if ( mMatrix[0].ok && !mMatrix[3].ok && (cornerCutting || mMatrix[1].ok ))
        {
                tmp.x = mMatrix[0].x;
                tmp.y = mMatrix[0].y;
                succ.push_back(tmp);
        }
        
        if ( mMatrix[8].ok && !mMatrix[7].ok && (cornerCutting || mMatrix[5].ok ))
        {
                tmp.x = mMatrix[8].x;
                tmp.y = mMatrix[8].y;
                succ.push_back(tmp);
        }

    }   // End of condition: we're coming here from a diagonal direction.
    
    // And now the jumping part...
    
    nodevec ret;
    for (auto& i : succ)
    {
        int dx = i.x - target.x, dy = i.y - target.y;
        assert(abs(dx) <= 1);
        assert(abs(dy) <= 1);
        auto n = jump(target, dx, dy, start, goal, cornerCutting);
        if (n.valid) ret.push_back(n);
    }
    
    return ret;
}


bool    cPathFinder::has_forced_neighbour(const cNodeID &id,
                                          int dx,
                                          int dy) const
{
    auto nx = id.x, ny = id.y;
    
    if ( dy == 0 )
    {
        if ( dx == 1 )  // straight, from left to right
        {
            return ((!blocked(nx+1, ny-1) &&    // 2 walkable
                    blocked(nx, ny-1))          // 1 blocked
                    ||
                    (!blocked(nx+1, ny+1) &&    // 8 walkable
                     blocked(nx, ny+1)));       // 7 blocked
        }
        
        if (dx == -1) // straight, from right to left
        {
            return ((!blocked(nx-1, ny-1) &&    // 0 walkable
                     blocked(nx, ny-1))         // 1 blocked
                    ||
                    (!blocked(nx-1, ny+1) &&    // 6 walkable
                     blocked(nx, ny+1)));       // 7 blocked
        }
    }
    
    if ( dx == 0 )
    {
        if ( dy == 1 )  // straight, from top to bottom
        {
            return ((!blocked(nx+1, ny+1) &&    // 8 walkable
                     blocked(nx+1, ny))         // 5 blocked
                    ||
                    (!blocked(nx-1, ny+1) &&    // 6 walkable
                     blocked(nx-1, ny)));          // 3 blocked
        }
        
        if ( dy == -1) // straight, from bottom to top
        {
            return ((!blocked(nx-1, ny-1) &&    // 0 walkable
                     blocked(nx-1, ny))       // 3 blocked
                    ||
                    (!blocked(nx+1, ny-1) &&    // 2 walkable
                     blocked(nx+1, ny)));       // 5 blocked
        }
    }
    
    if ( dx > 0 && dy < 0 )
    {
        return ((!blocked(nx-1, ny-1) &&       // 0 walkable
                 blocked(nx-1, ny))            // 3 blocked
                ||
                (!blocked(nx+1, ny+1) &&        // 8 walkable
                 blocked(nx, ny+1)));            // 7 blocked
    }
    
    if ( dx > 0 && dy > 0 )
    {
        return ((!blocked(nx+1, ny-1) &&       // 2 walkable
                 blocked(nx, ny-1))          // 1 blocked
                ||
                (!blocked(nx-1, ny+1) &&        // 6 walkable
                 blocked(nx-1, ny)));          // 3 blocked
    }
    
    if ( dx < 0 && dy > 0 )
    {
        return ((!blocked(nx-1, ny-1) &&       // 0 walkable
                 blocked(nx, ny-1))           // 1 blocked
                ||
                (!blocked(nx+1, ny+1) &&        // 8 walkable
                 blocked(nx+1, ny)));            // 5 blocked
    }
    
    if ( dx < 0 && dy < 0 )
    {
        return ((!blocked(nx+1, ny-1) &&       // 2 walkable
                 blocked(nx+1, ny))           // 5 blocked
                ||
                (!blocked(nx-1, ny+1) &&        // 6 walkable
                 blocked(nx, ny+1)));            // 7 blocked
    }
    
    
    // We really shouldn't be returning here, but to appease
    // the compiler:
    
    return false;
}

cNodeID cPathFinder::jump(const cNodeID &current,
                          int dx,
                          int dy,
                          const cNodeID& start,
                          const cNodeID& goal,
                          bool corcutallowed) const
{
    // Here, both dx and dy are between -1 and 1.
    
    cNodeID n { current };
    n.x += dx;
    n.y += dy;

    // If n is an obstacle or outside the grid then
    // return an invalid node.
    
    if ( n.x < 0 || n.x >= mBoardSize.x ||
         n.y < 0 || n.y >= mBoardSize.y ||
        mBoard[n.x][n.y].status == cStatus::blocked )
    {
        n.valid = false;
        return n;
    }

    // If n is the goal then we just return n.
    if ( n == goal ) return n;
    
    // Return if there's a slipping through corners.
    if ( !corcutallowed && dx != 0 && dy != 0 )
    {
        if (blocked(n.x+dx, n.y) && blocked(n.x, n.y+dy))
        {
            n.valid = false;
            return n;
        }
    }
    
    // If n has at least one forced neighbour, then
    // we have to return n
    if ( has_forced_neighbour(n, dx, dy) ) return n;

    // If no forced neighbours, and we're going diagonally,
    // first send out vertical and horizontal scan lines
    if ( dx != 0 && dy != 0 )
    {
        if ( jump(n, 0, dy, start, goal, corcutallowed).valid ) return n;
        if ( jump(n, dx, 0, start, goal, corcutallowed).valid ) return n;
    }
    
    // When push comes to shove, OK, fair enough, shoot
    // those diagonal jumps too
    
    return jump(n, dx, dy, start, goal, corcutallowed);
}


void cPathFinder::updateOpenList(const cNodeID& target,
                                 const cNodeID& new_parent)
{
    auto x = target.x, y = target.y;
    listElement old { target, mBoard[x][y].gScore + mBoard[x][y].hScore };
    
    mBoard[x][y].parent = new_parent;
    mBoard[x][y].gScore = calcGscore(new_parent, target);
    
    listElement tmp { target, mBoard[x][y].gScore + mBoard[x][y].hScore };
    q.replace(old, tmp);
}

std::vector<cNodeID> cPathFinder::walkable(const cNodeID& start,
                                           const cNodeID& end) const
{
    // Attempts to draw a straight line from start to end;
    // if this can be done, returns each of the line's points;
    // if not, returns an empty vector.
    
    static std::vector<cNodeID> empty_one;
    cNodeID step, current = start;
    std::vector<cNodeID> ret;
    
    // First divide line into vertical and horizontal components
    int dx = end.x - start.x, dy = end.y - start.y;
    step.x = ( dx > 0 ) - ( dx < 0 );
    step.y = ( dy > 0 ) - ( dy < 0 );
    
    // We distinguish between 2 basic cases: horizontal and vertical-like
    // lines. That's important because getting this wrong might result
    // in a wrong number of points drawn ( e.g. a line from 9:10 to 11:20,
    // only 3 points are drawn if we try to draw this as horizontal.
    
    int adx = abs(dx), ady = abs(dy);

    if ( adx > ady ) //horizontal line
    {
        // tmp keeps track of when we need to increment / decrement the
        // other variable, e.g. when drawing a horizontal line, when do
        // we make a vertical step. for an evenly distributed line,
        // this should start from half of the other component.
        
        int tmp = adx / 2;
        while ( current != end )
        {
            if ( blocked(current.x, current.y) )
                return empty_one;   // can't draw line.
            else ret.push_back(current);
            current.x += step.x;
            tmp += ady;
            if ( tmp >= adx )
            {
                tmp -= adx; current.y += step.y;
            }
        }
        
        // Add last point:
        ret.push_back(end);
        return ret;
    } else // vertical line
    {
        int tmp = ady / 2;
        while ( current != end )
        {
            if ( blocked(current.x, current.y))
                return empty_one;
            else ret.push_back(current);
            
            current.y += step.y;
            tmp += adx;
            if ( tmp > ady )
            {
                tmp -= ady;
                current.x += step.x;
            }
        }
        ret.push_back(end);
        return ret;
    }
}

std::vector<cNodeID> cPathFinder::smoothPath(const std::vector<cNodeID>& path) const
{
    
    // Smooths out a path by trying to eliminate waypoints - a waypoint is where the path
    // changes directions
    
    auto size = path.size();
    if ( size <= 2 ) return path;
    
    if ( mJPS == true )
    {
        std::vector<cNodeID> smoothPath;
        auto it = path.begin();
        auto end = path.end();

        // Next one: immediately near the node we're checking
        auto next = it; ++next;
        
        // Target: the one where we'd like to jump
        auto target = next; ++target;

        // We can add the first point w/o problem.
        smoothPath.push_back(*it);
        ++it;

        bool jmp { false };
        nodevec lastJump;
       
        while ( next != end )
        {
            jmp = false;
            
            // Jump as far as we can
            bool cnt { true };
            while ( target != end && cnt == true)
            {
                nodevec vec = walkable(*it, *target);
                if ( vec.size() != 0 ) // we CAN jump over
                {
                    lastJump.clear();
                    std::copy(vec.begin(), vec.end(), std::back_inserter(lastJump));
                    ++target;
                    jmp = true;
                }
                else
                {
                    --target;
                    cnt = false;
                }
            }
            
            if ( cnt == true ) --target;
            
            if ( jmp )
            {
                for ( auto&& a : lastJump )
                    smoothPath.push_back(std::move(a));
                it = target;
                next = it; ++next;
                target = next; ++target;
            }
            else        // no jump here
            {
                auto vec = walkable(*it, *next);
                for ( auto&& a : vec )
                    smoothPath.push_back(std::move(a));
                it = next;
                ++next;
                ++target;
            }
        }
        
        return smoothPath;

    }
    
    std::vector<size_t> waypointIDs;
    waypointIDs.push_back(0);
    
    cNodeID incoming;
    cNodeID outgoing;
    
    // Fill up a temporary vector of waypoints. A waypoint is a node
    // whose outgoing direction is not the same as its incoming one.
    // Like, "turning points."
    
    for ( auto i = 1; i < size-1; ++i )
    {
        incoming = path[i] - path[i-1];
        outgoing = path[i+1] - path[i];
        if ( outgoing != incoming ) waypointIDs.push_back(i);
    }
    waypointIDs.push_back(size-1);
    
    // Now let's check if there are points to eliminate.
    std::vector<cNodeID> repl;  // replacement
    std::map<cNodeID, std::vector<cNodeID>> redrawn;
    
    for ( auto it = waypointIDs.begin(); it != waypointIDs.end(); ++it )
        if ( *it != 0 && *it != size-1 )
        {
            auto prev = it - 1;
            auto next = it + 1;
            repl = walkable(path[*prev], path[*next]);
            if ( !repl.empty() )
            {
                redrawn.insert(std::pair<cNodeID, std::vector<cNodeID>> { cNodeID( *prev, *next), repl });
                it = waypointIDs.erase(it);
            }
        }
    
    // Now we have the waypoints and their connecting lines, yay!
    std::vector<cNodeID> smoothPath;
    for ( auto it = waypointIDs.begin() + 1; it != waypointIDs.end(); ++it )
    {
        cNodeID actual { static_cast<int>(*(it-1)), static_cast<int>(*it) };
        if ( redrawn.find(actual) != redrawn.end() ) // if this was redrawn
        {
            for ( auto& j : redrawn[actual] )
                smoothPath.push_back(j);
        } else                                  // we just use the original if it wasn't redrawn
            for ( auto k = actual.x; k <= actual.y; ++k )
                smoothPath.push_back(path[k]);
    }
    
    return smoothPath;
}

nodevec cPathFinder::neighbours(const cNodeID& current,
                                const cNodeID& start,
                                const cNodeID& end,
                                bool corcuta,
                                bool jps)
{

    // This is the rub. Jump point search differs from basic A* in the way it finds the
    // neighbours of any given node. For JPS, a neighbour need not be immediately adjacent
    // to the node we're considering.
    
    return jps ? successors(current, start, end, corcuta) : adjacent(current, corcuta);
}

std::vector<cNodeID> cPathFinder::findPath(const cNodeID& start,
                                          const cNodeID& end,
                                          bool corCutAllowed,
                                          bool smooth)
{
    std::vector<cNodeID>    path;
    std::vector<cNodeID>    found;
    cNodeID                 currentNode { start };
    
    addToOpenList(currentNode, currentNode, end);
    
    // "q:" the priority queue of nodes on the open list
    
    while ( !onCList(end) && !q.empty() )
    {
        currentNode = q.pop_and_get().id;
        addToClosedList(currentNode);
        found.push_back(currentNode);
        
        for (auto& i : neighbours(currentNode, start, end, corCutAllowed, mJPS))
            if ( !onCList(i) )
            {
                if ( !onOList(i) )
                {
                    addToOpenList(i, currentNode, end);
                }
                else
                {
                    if ( calcGscore(currentNode, i) < mBoard[i.x][i.y].gScore )
                    {
                         updateOpenList(i, currentNode);
                    }
                }
            }
    }
    
    if ( onCList(end) ) // path found!
    {
        cNodeID tmp = end;
        while ( mBoard[tmp.x][tmp.y].parent != tmp )
        {
            path.insert(path.begin(), tmp);
            tmp = mBoard[tmp.x][tmp.y].parent;
        }
        path.insert(path.begin(), tmp);
    }
    
    while ( !q.empty() )    // Flush the open list; very important to do
        q.pop();            // after each pathfinding!
    
    ++UID;                  // also very important: next pathfinding:
                            // new unique ID.

    return smooth == false ? path : smoothPath(path);
}

void cPathFinder::setView(const sf::View& v)
{
    mVs = v.getSize();
    mVc = v.getCenter();
}

void cPathFinder::startMarking(const sf::Vector2i& pos)
{
    mMarkerStart.x = (mVc.x - (mVs.x / 2 ) + pos.x) / mTileSize.x;
    mMarkerStart.y = (mVc.y - (mVs.y / 2 ) + pos.y) / mTileSize.y;
    if ( mMarkerStart.x >= mBoardSize.x ) mMarkerStart.x = mBoardSize.x-1;
    if ( mMarkerStart.y >= mBoardSize.y ) mMarkerStart.y = mBoardSize.y-1;
}

void cPathFinder::keepMarking(const sf::Vector2i& pos)
{
    mMarkerNow.x = (mVc.x - (mVs.x / 2 ) + pos.x) / mTileSize.x;
    mMarkerNow.y = (mVc.y - (mVs.y / 2 ) + pos.y) / mTileSize.y;
    
    if ( mMarkerNow.x >= mBoardSize.x ) mMarkerNow.x = mBoardSize.x-1;
    if ( mMarkerNow.y >= mBoardSize.y ) mMarkerNow.y = mBoardSize.y-1;
    
    short int       stepx { 0 };
    short int       stepy { 0 };
    
    if (mMarkerNow.x > mMarkerStart.x) stepx = 1;
    if (mMarkerNow.x < mMarkerStart.x) stepx = -1;

    if (mMarkerNow.y > mMarkerStart.y ) stepy = 1;
    if (mMarkerNow.y < mMarkerStart.y ) stepy = -1;

    if (mMarkerNow.x == mMarkerStart.x && mMarkerNow.y == mMarkerStart.y)
    {
        mBoard[mMarkerNow.x][mMarkerNow.y].marked = true;
        return;
    }
    
    auto i = mMarkerStart.x;

    do
    {
        auto j = mMarkerStart.y;
        do
        {
            mBoard[i][j].marked = true;
            j += stepy;
        } while (j != mMarkerNow.y + stepy);
        i += stepx;
    } while ( i != mMarkerNow.x + stepx );
    
}

void cPathFinder::toggleMarkedOnes()
{
    short int       stepx { 0 };
    short int       stepy { 0 };
    
    if (mMarkerNow.x > mMarkerStart.x) stepx = 1;
    if (mMarkerNow.x < mMarkerStart.x) stepx = -1;
    
    if (mMarkerNow.y > mMarkerStart.y ) stepy = 1;
    if (mMarkerNow.y < mMarkerStart.y ) stepy = -1;
    
    auto i = mMarkerStart.x;
    
    do
    {
        auto j = mMarkerStart.y;
        do
        {
            if (mBoard[i][j].status == cStatus::blocked)
                mBoard[i][j].status = cStatus::walkable;
            else mBoard[i][j].status = cStatus::blocked;
            j += stepy;
        } while (j != mMarkerNow.y + stepy);
        i += stepx;
    } while ( i != mMarkerNow.x + stepx );
}

bool cPathFinder::walk(const sf::Vector2i& from,
                       const sf::Vector2i& to,
                       bool cornercutting,
                       bool smoothing)
{
    if ( to.x > mBoardSize.x * mTileSize.x ||
         to.y > mBoardSize.y * mTileSize.y ) return false;
    
    sf::Vector2i    sTile, eTile;
    
    sTile.x = (mVc.x - (mVs.x / 2 ) + from.x ) / mTileSize.x;
    sTile.y = (mVc.y - (mVs.y / 2 ) + from.y ) / mTileSize.y;
    
    eTile.x = (mVc.x - (mVs.x / 2 ) + to.x ) / mTileSize.x;
    eTile.y = (mVc.y - (mVs.y / 2 ) + to.y ) / mTileSize.y;
    
    if ( !valid(eTile.x, eTile.y) ) return false;
    
    if ( mBoard[sTile.x][sTile.y].status == cStatus::blocked ||
         mBoard[eTile.x][eTile.y].status == cStatus::blocked) return false;
    
    std::vector<cNodeID> path = findPath(cNodeID { sTile.x, sTile.y },
                                         cNodeID { eTile.x, eTile.y },
                                         cornercutting,
                                         smoothing);
    
    if ( path.empty() ) return false;
    
    for(auto&& i : path)
        mBoard[i.x][i.y].status = cStatus::walked;

    return true;
}

// Attention. What we have here are screen coordinates!
void cPathFinder::toggle(const sf::Vector2i& pos)
{
    sf::Vector2i    tile;
    tile.x = (mVc.x - (mVs.x / 2) + pos.x) / mTileSize.x;
    tile.y = (mVc.y - (mVs.y / 2) + pos.y) / mTileSize.y;
    
    if ( tile.x < 0 || tile.x >= mBoardSize.x ) return;
    if ( tile.y < 0 || tile.y >= mBoardSize.y ) return;
    
    if ( mBoard[tile.x][tile.y].status == cStatus::blocked )
        mBoard[tile.x][tile.y].status = cStatus::walkable;
    else
        mBoard[tile.x][tile.y].status = cStatus::blocked;
}

void cPathFinder::toggle(unsigned long int x,
                         unsigned long int y)
{
    sf::Vector2i    tile;
    tile.x = (mVc.x - (mVs.x / 2) + x) / mTileSize.x;
    tile.y = (mVc.y - (mVs.y / 2) + y) / mTileSize.y;
    
    if ( tile.x < 0 || tile.x >= mBoardSize.x ) return;
    if ( tile.y < 0 || tile.y >= mBoardSize.y ) return;
    
    if ( mBoard[tile.x][tile.y].status == cStatus::blocked )
        mBoard[tile.x][tile.y].status = cStatus::walkable;
    else
        mBoard[tile.x][tile.y].status = cStatus::blocked;
}

void cPathFinder::render(sf::RenderWindow& w)
{
    
    sf::Vector2i startTile;
    startTile.x = (mVc.x - (mVs.x / 2)) / mTileSize.x;
    startTile.y = (mVc.y - (mVs.y / 2)) / mTileSize.y;
    
    sf::Vector2u tileCount;
    tileCount.x = (mVs.x / mTileSize.x) + startTile.x + 2;
    tileCount.y = (mVs.y / mTileSize.y) + startTile.y + 2;
    
    if (tileCount.x > mBoardSize.x ) tileCount.x = mBoardSize.x;
    if (tileCount.y > mBoardSize.y ) tileCount.y = mBoardSize.y;
    
    sf::Color       tmpCol;
    sf::Color       outCol;
    
    auto row = tileCount.x - startTile.x;       // this many tiles in a single row
    
    for(auto i = startTile.y; i < tileCount.y; ++i)
        for(auto j = startTile.x; j < tileCount.x; ++j)
        {
            
            switch (mBoard[j][i].status) {
                case cStatus::walkable:
                {
                    tmpCol = sf::Color::White;
                    break;
                }
                case cStatus::blocked:
                {
                    tmpCol = sf::Color::Blue;
                    break;
                }
                case cStatus::walked:
                {
                    tmpCol = sf::Color::Red;
                    mBoard[j][i].status = cStatus::walkable;
                    break;
                }
            }
            
            // "Marked" boolean flags are automatically reset
            // in each rendering turn.
            
            if (mBoard[j][i].marked)
            {
                tmpCol.a = 120;
                outCol = sf::Color::Cyan;
                mBoard[j][i].marked = false;
            } else outCol = sf::Color::Black;
            
            auto id = (4 * row) * i + ( 4 * j );
            
            mGrid[id].color = tmpCol;
            mGrid[id+1].color = tmpCol;
            mGrid[id+2].color = tmpCol;
            mGrid[id+3].color = tmpCol;
        }

    w.draw(&mGrid[0], mGrid.size(), sf::Quads);     // vertexarray, yay!
}