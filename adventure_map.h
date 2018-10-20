#ifndef ADVENTURE_MAP_H
#define ADVENTURE_MAP_H

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <utility>
#include <exception>
#include <stdexcept>
#include <limits>

// represents an adventure graph structure.
// models a finite state machine envisioned as a graph where nodes (states) are situations (questions) and arcs are responses (choices).
// the payload types are the types of the "data" fields of the Arc and Node types declared internally.
template<typename NodePayload, typename ArcPayload>
struct AdventureMap
{
public: // -- types -- //

    // represents an arc in the adventure graph (a choice)
    struct Arc
    {
        ArcPayload data; // user-defined data payload

        std::size_t dest; // the ending position for this arc
    };

    // represents a node in the adventure graph (a question)
    struct Node
    {
        NodePayload data; // user-defined node payload

        std::vector<Arc> arcs; // the arcs from this node
    };

    typedef typename std::vector<Node>::iterator             iterator;
    typedef typename std::vector<Node>::const_iterator const_iterator;

    typedef typename std::vector<Node>::difference_type difference_type;

private: // -- data -- //

    std::vector<Node> _nodes; // all the nodes in the graph

    std::size_t _state; // the current state

public: // -- ctor / dtor / asgn --

    // constructs an empty adventure map
    AdventureMap() = default;

public: // -- accessors -- //

    // gets/sets the current state
    std::size_t &state()       { return _state; }
    std::size_t  state() const { return _state; }

    // gets the number of nodes
    std::size_t size() const { return _nodes.size(); }

    // returns the node at the specified index. no bounds checking.
          Node &operator[](std::size_t index)       { return _nodes[index]; }
    const Node &operator[](std::size_t index) const { return _nodes[index]; }

    // returns the node at the specified index. includes bounds checking (std::out_of_range).
          Node &at(std::size_t index)       { return _nodes.at(index); }
    const Node &at(std::size_t index) const { return _nodes.at(index); }

public: // -- iterators -- //

    iterator begin() { return _nodes.begin(); }
    iterator end()   { return _nodes.end();   }

    const_iterator begin() const { return _nodes.begin(); }
    const_iterator end()   const { return _nodes.end();   }

    const_iterator cbegin() const { return _nodes.cbegin(); }
    const_iterator cend()   const { return _nodes.cend();   }

public: // -- add / remove -- //

    // adds the specified node to the graph.
    // WARNING: invalidates iterators
    void push_back(const Node &node) { _nodes.push_back(node); }

    // adds the specified node to the graph.
    // WARNING: invalidates iterators
    template<typename ...Args>
    void emplace_back(Args &&...args) { _nodes.emplace_back(std::forward<Args>(args)...); }

    // removes the specified nodes from the graph. any arcs that reference it are deleted as well. no bounds checking.
    // arcs in other nodes are updated to reflect the change. arcs pointing to the removed node are removed as well.
    // WARNING: invalidates iterators
    void erase(const_iterator iter)
    {
        // convert iterator into a position
        std::size_t index = std::size_t(iter - _nodes.cbegin());

        // remove the node (can't use the iterator since this version of C++ uses non-const iterators for erase)
        _nodes.erase(_nodes.begin() + difference_type(index));

        // for each remaining node
        for (Node &node : _nodes)
        {
            // for each arc
            for (auto i = node.arcs.begin(), _end = node.arcs.end(); i != _end; )
            {
                // if the arc pointed to the removed node
                if (i->dest == index)
                {
                    // remove it as well
                    i = node.arcs.erase(i);
                    _end = node.arcs.end();
                }
                else
                {
                    // account for the removed node
                    if (i->dest >= index) --i->dest;
                    ++i;
                }
            }
        }
    }

};

#endif // ADVENTURE_MAP_H
