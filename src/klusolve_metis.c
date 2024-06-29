#ifdef _MSC_VER
#   if _MSC_VER <= 1500
#       error "This compiler version is not supported anymore."
#   endif
#endif

#ifdef __cplusplus
#include <cstdint.h>
#include <cstdlib> 
#else // __cplusplus
#include <stdint.h>
#include <stdlib.h> 
#include <stdio.h>
#endif
#include <metis.h>


#if defined(_WIN32) && !defined(_WIN64)
#define KLUSOLVEX_STDCALL __stdcall
#else
#define KLUSOLVEX_STDCALL
#endif

#ifdef __cplusplus
extern "C" 
#endif
int32_t KLUSOLVEX_STDCALL klusolve_metis(
    int32_t *sorted_edge_pairs, // ([v1 v2] [v1 v3]) ...
    int32_t *edge_weights,
    int32_t num_edges, // number of edges
    int32_t num_vertices, 
    int32_t num_partitions, 
    
    int32_t *zones, // size = num_vertices, allocated by caller
    int32_t *edge_cut // simple pointer
)
{
    int32_t error = 0;
    idx_t i;
    idx_t vfrom, vto, ew;
    idx_t current_vertex, edge_idx;
    idx_t nparts = num_partitions, num_vertices2 = num_vertices, ncon = 1;
    idx_t edgecut;
    idx_t *xadj=NULL, *adjacency=NULL, *adj_weights=NULL, *vertex_partitions=NULL;
    num_edges *= 2;
    // idx_t options[METIS_NOPTIONS];

    /////
    // Loop through the matrix to count the edges
    /////
    // current_vertex = 0;
    // for (i = 0; i < num_edges; ++i)
    // {
    //     while (sorted_edge_pairs[2 * i] == current_vertex)
    //     {
    //         edge_vertex = sorted_edge_pairs[2 * i + 1];
    //         if (sorted_edge_pairs[2 * i] == edge_vertex) // skip diagonal
    //         {
    //             ++i;
    //             continue;
    //         }

    //         if (edge_vertex < 0 || edge_vertex >= num_vertices2)
    //         {
    //             error = 1;
    //             goto ON_ERROR;
    //         }

    //         ++edge_idx;
    //     }
    //     ++current_vertex;
    // }

    // nedges = edge_idx;
    xadj = calloc(num_vertices + 1, sizeof(idx_t));
    adjacency = calloc(num_edges, sizeof(idx_t));
    adj_weights = calloc(num_edges, sizeof(idx_t));
    vertex_partitions = calloc(num_vertices, sizeof(idx_t));

    /////
    // Loop through the matrix to copy the adjacency data
    /////

    // Start with vertex 0
    current_vertex = 0;
    // Mark its adjacency list as starting from 0
    xadj[current_vertex] = 0;

    edge_idx = 0;
    for (i = 0; i < num_edges; ++i)
    {
        vfrom = sorted_edge_pairs[2 * i];
        vto = sorted_edge_pairs[2 * i + 1];

        if (vfrom != current_vertex)
        {
            // Increment vertex and mark its adjacency position
            ++current_vertex;
            xadj[current_vertex] = edge_idx;
        }

        if (vto == current_vertex) // skip diagonal / self edges
        {
            continue;
        }

        if (vto < 0 || vto >= num_vertices2)
        {
            error = 1;
            goto ON_ERROR;
        }

        adjacency[edge_idx] = vto;
        adj_weights[edge_idx] = edge_weights[i];
        ++edge_idx;
    }
    xadj[num_vertices] = edge_idx;

    // if (num_partitions <= 8)
    // {
        error = METIS_PartGraphKway(
            &num_vertices2, 
            &ncon,
            xadj, 
            adjacency, 
            NULL, // vwgt
            NULL, // vsize
            adj_weights,
            &nparts, 
            NULL,
            NULL,
            NULL,
            &edgecut, 
            vertex_partitions
        );
    // }
    // else
    // {
    //     error = METIS_PartGraphRecursive(
    //         &num_vertices2, 
    //         &ncon,
    //         xadj, 
    //         adjacency, 
    //         NULL, // vwgt
    //         NULL, // vsize
    //         adj_weights,
    //         &nparts, 
    //         NULL,
    //         NULL,
    //         NULL,
    //         &edgecut, 
    //         vertex_partitions
    //     );
    // }
    
    if (error == METIS_OK)
    {
        // Got valid output, copy 
        for (i = 0; i < num_vertices; ++i)
        {
            zones[i] = vertex_partitions[i];
        }
        *edge_cut = edgecut;
        error = 0;
    }
    else
    {
        error = 2;
    }

ON_ERROR:
    free(xadj);
    free(adjacency);
    free(adj_weights);
    free(vertex_partitions);
    return error;
}
