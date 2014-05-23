#ifndef __small_astartest__pathfinder__
#define __small_astartest__pathfinder__

#include "nodeID.h"
#include "board.h"
#include "prQueue.h"
#include "listElement.h"
#include <SFML/Graphics.hpp>

typedef std::vector<cNodeID> nodevec;

struct twoints {
    int x, y;
    bool ok;
};

class cPathFinder {
public:
    cPathFinder(unsigned int, unsigned int);
    
    void        render(sf::RenderWindow&);

    void        toggle(const sf::Vector2i&);
    void        toggle(unsigned long int, unsigned long int);
    
    void        startMarking(const sf::Vector2i&);
    void        keepMarking(const sf::Vector2i&);
    
    void        toggleMarkedOnes();
    void        setView(const sf::View&);

    nodevec     findPath(const cNodeID& start,
                         const cNodeID& end,
                         bool corCutAllowed = false,
                         bool smooth = true);
    
    bool        walk(const sf::Vector2i& from,
                     const sf::Vector2i& to,
                     bool cornercutting,
                     bool smoothing);

public:
    bool        mJPS { false };

private:
    void            addToOpenList(const cNodeID& target,
                                  const cNodeID& parent,
                                  const cNodeID& end);
    void            addToClosedList(const cNodeID&);
    unsigned int    calcGscore(const cNodeID&, const cNodeID&) const;
    unsigned int    calcHscore(const cNodeID&, const cNodeID&) const;
    inline bool     onCList(const cNodeID&) const;
    inline bool     onOList(const cNodeID&) const;
    nodevec         adjacent(const cNodeID&,
                                     bool cornerCuttingAllowed = true) const;
    inline bool     valid(long int x, long int y) const;
    bool            blocked(long int x, long int y) const;

    void            updateOpenList(const cNodeID& target,
                                   const cNodeID& new_parent);
    nodevec         smoothPath(const nodevec&) const;
    nodevec         walkable(const cNodeID&,
                             const cNodeID&) const;
    
    nodevec         successors(const cNodeID& target,
                               const cNodeID& start,
                               const cNodeID& goal,
                               bool cornerCutting);
    
    nodevec         neighbours(const cNodeID&,
                               const cNodeID&,
                               const cNodeID&,
                               bool cornerCutting,
                               bool jps);
    
    cNodeID         jump(const cNodeID& current,
                         int dx,
                         int dy,
                         const cNodeID& start,
                         const cNodeID& goal,
                         bool corcutallowed) const;
    
    bool            has_forced_neighbour(const cNodeID& id,
                                         int dx,
                                         int dy) const;
    
private:
    static int                          UID;
    
    twoints                             mMatrix[9];

    std::vector<std::vector<cField>>    mBoard;
    sf::Vector2u                        mTileSize;
    sf::Vector2u                        mBoardSize;
    sf::Vector2u                        mMarkerStart;
    sf::Vector2u                        mMarkerNow;
    sf::Vector2f                        mVs;    // view size
    sf::Vector2f                        mVc;    // view center;
    
    cPQ<listElement>                    q;      // priority queue for
                                                // quick pathfinding.
    std::vector<sf::Vertex>             mGrid;
};

#endif /* defined(__small_astartest__pathfinder__) */