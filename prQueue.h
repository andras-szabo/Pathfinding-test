#ifndef __priority_queue__prQueue__
#define __priority_queue__prQueue__

#include <vector>
#include <functional>
#include <iostream>

// Priority queue template, implemented as a wrapper around
// a vector that is used as a binary heap.
// If it's not supplied a predicate, it will use "bigger than"
// as a default, i.e. it's going to sort its elements
// into decreasing order. This, obviously, requires, that
// items be comparable this way.
// But custom predicates are OK too; note that when supplying
// them, they should always have 2 parameters, both being
// const references to the type in question (const T&).

// Key interface:
// cPQ(), cPQ(vector<T>), or cPQ(vector<T>, predicate) : constructs new queue
// push(const T&)
// pop()
// top() - returns const & to the item that's on top of the heap
// getSize() - returns the number of ACTUAL elements on the heap. The physical size
// of the heap is going to be getSize()+1, because the indexing of the heap
// starts at 1, not at 0.

template <typename T>
class cPQ {
public:
    cPQ();                                  // default constructor
                                            // it requires that T has default initialization.
    
    // Next one: constructs heap from vector using predicate. The default predicate is
    // supplied here using a lambda.
    
    cPQ(const std::vector<T>&, bool (const T&, const T&) = [](const T& a, const T& b) { return a > b;} );
    cPQ(const cPQ&);
    ~cPQ() { };
    
    bool        empty() const { return last == 0; }
    void        push(const T&);
    T           pop_and_get();      // Note that this returns by value! => copies object.
    void        pop();              // Only pops the top of the heap, but doesn't return it
    const T&    top() const;        // Returns reference to the top item
    size_t      getSize() const { return last;  }   // How many valid elements are there on the heap?
    bool        contains(const T& find_this)
                {
                    if ( last == 0 ) return false;
                    if ( search(find_this, 1) != 0 ) return true;
                    return false;
                }
    
    bool        replace(const T&, const T&);        // Replaces element a with element b...?
    
    void        setPred(bool f(const T&, const T&))
    {
        // Oho. So we actually changed the predicate while we had stuff in the
        // array. So we need to reconstruct the whole thing!
        pred = f;
        construct();
    }
    
    void        display()
                {
                    for (auto i = 1; i <= last; ++i)
                        std::cout << a[i] << " ";
                    std::cout << "\n";
                }
    
private:
    void        construct();
    void        exchg(size_t n, size_t m)
    {
        if ( n == m ) return;
        T tmp { a[n] };
        a[n] = a[m];
        a[m] = tmp;
    }
    
    size_t      search(const T&, size_t);
    // Recursive search for element x.
    
    size_t      fine(size_t);
    // checks if a node is "fine," i.e. predicate is true
    // in relation to it and its 2 children. If not true,
    // this returns the ID of the child with which it's
    // to be swapped. It may return itself, suggesting that
    // it is actually "fine." So when we sink or swim items
    // in the heap, we need to do that until we reach the point
    // where fine(item index) == item index.
    
private:
    std::vector<T>                          a;      // the "array"
    size_t                                  last;   // last VALID index; it's 0 if the heap is empty.
    std::function<bool(const T&, const T&)> pred;   // teh predicate.
};

#include "prQueue.inl"

#endif /* defined(__priority_queue__prQueue__) */
