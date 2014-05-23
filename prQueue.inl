template <typename T>
cPQ<T>::cPQ()
{
    T tmp;
    a.push_back(tmp);
    last = 0;
    pred = [](const T& a, const T& b) { return a > b;} ;
}

template<typename T>
void cPQ<T>::construct()
{
    if ( last < 2 ) return;
    for (auto i = last / 2; i >= 1; --i)
    {
        auto item = i;
        auto place = fine(i);
        while ( item != place )
        {
            exchg(item, place);
            item = place;
            place = fine(item);
        }
    }
}

// Let's construct heap from vector!
template <typename T>
cPQ<T>::cPQ(const std::vector<T>& v, bool f(const T&, const T&))
{
    // Firstest thing to do: make sure to start counting from 1
    T tmp;
    a.push_back(tmp);
    last = 0;
    
    // Then set predicate.
    pred = f;
    
    // Then copy the vector to make it our own.
    for (auto& i : v)
    {
        a.push_back(i);
        ++last;
    }
    
    // Then let's construct the heap order.
    construct();
}

// Copy constructor; shall be tested thoroughly,
// seems to work for now though.
template <typename T>
cPQ<T>::cPQ(const cPQ& c)
{
    a = c.a;
    last = c.last;
    pred = c.pred;
}

// The all-important fine thing. If an element is "fine", then we return
// its own index; otherwise the index of that item with which it should
// be exchanged.
template <typename T>
size_t cPQ<T>::fine(size_t n)
{
    auto nn = n*2;
    if ( nn > last ) return n;      // Item has no kids, must be fine.
    if ( nn+1 > last )              // Item has 1 child.
    {
        if ( pred(a[n],a[nn]) == true ) return n;
        else return nn;
    }
    if ( pred(a[n],a[nn]) && pred(a[n],a[nn+1]) ) return n;  // Item has 2 kids, but in good place
    
    // Otherwise we exchange it with the child that's bigger (a[nn] is considered bigger
    // if the predicate is true in its relationship with a[nn+1].
    return pred(a[nn],a[nn+1]) ? nn : nn+1;
}

// When adding new items: push them back to the end of the array,
// and let them swim up to the position they belong to.
template<typename T>
void cPQ<T>::push(const T& item)
{
    a.push_back(item);
    ++last;
    auto i = last;
    while ( i > 1 && pred(a[i],a[i/2]) )
    {
        exchg(i, i/2);
        i /= 2;
    }
}

template<typename T>
size_t cPQ<T>::search(const T& find_this, size_t start_here)
{
    if ( a[start_here] == find_this ) return start_here;
    
    // Can we go left? And if we can, does it make sense?
    // It's pointless to go on if the predicate is not true
    // in the relationship of find_this and left child:
    // i.e. in basic PQ, if find_this is not larger than left
    // child, it makes sense to go on, because it might be
    // that the child in question will be the one we're looking for.
    // If on the other hand find_this is already larger than the
    // child, we're obviously not going to find it under it.
    if ( start_here * 2 <= last )
        if ( !pred(find_this, a[start_here * 2]) )
        {
            auto found = search(find_this, start_here * 2);
            if (found != 0) return found;
        }
    
    // Can we go right?
    if ( (start_here * 2) + 1 <= last )
        if ( !pred(find_this, a[(start_here * 2) + 1]) )
        {
            auto found = search(find_this, start_here * 2 + 1);
            if (found != 0) return found;
        }
    
    // Otherwise, return 0.
    return 0;
}

// Replacing an item: find item corresponding to a;
// if exists, replace it with b and return true,
// if doesn't exist, don't replace, return false
template<typename T>
bool cPQ<T>::replace(const T& rep_this, const T& with_this )
{
    if ( last == 0 ) return false;  // empty heap
    auto id = search(rep_this, 1);
    if ( id == 0 ) return false;    // not there, can't replace
    
    // Otherwise:
    a[id] = with_this;
    
    // And now we have to see if this is fine - does it have to sink or swim?
    
    // Maybe it has to swim?
    if ( id >= 2 )  // it has parent
        if ( !pred(a[id/2], a[id]) )    // it has to swim
        {
            while ( id >= 2 && !pred(a[id/2], a[id]) )
            {
                exchg(id/2, id);
                id /= 2;
            }
            return true;                // it swam to the right place.
        }
    
    // Maybe it has to sink?
    if ( id * 2 <= last )   // it has kids at any rate
    {
        size_t item = id;
        size_t place = fine(item);
        while ( item != place )
        {
            exchg(item, place);
            item = place;
            place = fine(item);
        }
    }
    
    return true;
}

template<typename T>
T cPQ<T>::pop_and_get()
{
    if ( last < 1 ) throw std::runtime_error("Trying to pop from empty heap.");
    T ret { a[1] };
    pop();
    return ret;
}

template<typename T>
void cPQ<T>::pop()
{
    // First we take the top element and put it on the bottom, then remove it.
    if ( last < 1 ) throw std::runtime_error("Trying to pop from empty heap.");
    exchg(1, last--);
    a.pop_back();
    
    // If we still have data, and more than 1 items, then we sink the new top
    // item to its place.
    if ( last > 1 )
    {
        size_t item = 1;
        size_t place = fine(item);
        while ( item != place )
        {
            exchg(item, place);
            item = place;
            place = fine(item);
        }
    }
}

template <typename T>
const T& cPQ<T>::top() const
{
    if ( last < 1 ) throw std::runtime_error("Trying to read from empty heap.");
    return a[1];
}
