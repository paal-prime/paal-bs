#ifndef _GRAPH_H
#define _GRAPH_H

namespace Graph
{
    struct directed { enum { is_directed = true }; };
    struct undirected { enum { is_directed = false }; };
    struct unweighted {};
}

#endif /* _GRAPH_H */
