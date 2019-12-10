#ifdef _MSC_VER
#   if _MSC_VER <= 1500
#       include <stdint_compat.h>
#   else
#       include <stdint.h>
#   endif
#else
#    include <stdint.h>
#endif

#include <stdlib.h> 
#include <metis.h>

#ifdef __cplusplus
extern "C" 
#endif
int32_t klusolve_metis(
    int32_t *data, 
    int32_t data_count, 
    int32_t num_vertices, 
    int32_t num_partitions, 
    
    int32_t *zones, // size = num_vertices, allocated by caller
    int32_t *edge_cut // simple pointer
)
{
    int32_t error = 0;
    idx_t i;
    idx_t nedges, current_vertex, edge_idx, edge_vertex;
    idx_t nparts = num_partitions, nvtxs = num_vertices, ncon = 1;
    idx_t edgecut;
    idx_t *xadj=NULL, *adjncy=NULL, *vertex_partitions=NULL;
    // idx_t options[METIS_NOPTIONS];

    /////
    // Loop through the matrix to count the edges
    /////
    current_vertex = 0;
    i = 0;
    while (i < data_count)
    {
        while (data[2 * i] == current_vertex)
        {
            edge_vertex = data[2 * i + 1];
            if (data[2 * i] == edge_vertex) // skip diagonal
            {
                ++i;
                continue;
            }

            if (edge_vertex < 0 || edge_vertex >= nvtxs)
            {
                error = 1;
                goto ON_ERROR;
            }

            ++edge_idx;
        }
        ++current_vertex;
    }

    nedges = edge_idx;
    xadj = calloc(nvtxs + 1, sizeof(idx_t));
    adjncy = calloc(nedges, sizeof(idx_t));
    vertex_partitions = calloc(nvtxs, sizeof(idx_t));

    /////
    // Loop through the matrix to copy the adjacency data
    /////

    // Start with vertex 0
    current_vertex = 0;
    // Mark its adjacency list as starting from 0
    xadj[current_vertex] = 0;

    edge_idx = 0;
    i = 0;
    while (i < data_count)
    {
        while (data[2 * i] == current_vertex)
        {
            edge_vertex = data[2 * i + 1];
            if (data[2 * i] == edge_vertex) // skip diagonal
            {
                ++i;
                continue;
            }

            if (edge_vertex < 0 || edge_vertex >= nvtxs)
            {
                error = 1;
                goto ON_ERROR;
            }

            adjncy[edge_idx] = edge_vertex;
            ++edge_idx;
        }

        // Increment vertex and mark its adjacency position
        ++current_vertex;
        xadj[current_vertex] = edge_idx;
    }

    if (num_partitions <= 8)
    {
        error = METIS_PartGraphKway(
            &nvtxs, 
            &ncon,
            xadj, 
            adjncy, 
            NULL, 
            NULL, 
            NULL,
            &nparts, 
            NULL,
            NULL,
            NULL,
            &edgecut, 
            vertex_partitions
        );
    }
    else
    {
        error = METIS_PartGraphRecursive(
            &nvtxs, 
            &ncon,
            xadj, 
            adjncy, 
            NULL, 
            NULL, 
            NULL,
            &nparts, 
            NULL,
            NULL,
            NULL,
            &edgecut, 
            vertex_partitions
        );
    }
    
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
    free(adjncy);
    free(vertex_partitions);
    return error;
}
