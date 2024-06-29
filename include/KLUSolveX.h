/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2024, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#ifndef DSS_EXTENSIONS_KLUSOLVEX_H
#define DSS_EXTENSIONS_KLUSOLVEX_H

#ifdef _MSC_VER
#if _MSC_VER <= 1500
#error "This compiler version is not supported anymore."
#endif
#endif

#if defined(_WIN32) && !defined(_WIN64)
#define KLUSOLVEX_STDCALL __stdcall
#else
#define KLUSOLVEX_STDCALL
#endif

#ifdef __cplusplus
#include <cstdint>
#else // __cplusplus
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef _COMPLEX_DEFINED
#define _COMPLEX_DEFINED
    typedef struct _complex {
        double x, y;
    } complex;
#endif

    enum ReuseFlags { 
        // The values themselves are subject to change in future versions,
        // use this enum for easier upgrades
        ReuseNothing = 0,
        ReuseCompressedMatrix = 1, // Reuse only the prepared CSC matrix
        ReuseSymbolicFactorization = 2, // Reuse the symbolic factorization, implies ReuseCompressedMatrix
        ReuseNumericFactorization = 3 // Reuse the numeric factorization, implies ReuseSymbolicFactorization
    };

    enum MatrixFormatFlags {
        MatrixFormat_DoublePrecisionReal = 32 // The matrix elements are float64, no imaginary part
        // MatrixFormat_SinglePrecisionComplex = 33, // The matrix elements are complex float32, real and imaginary parts
        // MatrixFormat_SinglePrecisionReal = 34 // The matrix elements are float32, no imaginary part
    };

    // Set KLUSolveX options. Currently restricted to ReuseFlags values.
    // Other bits reserved for future use.
    void KLUSOLVEX_STDCALL SetOptions(void* handle, uint64_t opts);

    // return handle of new sparse set, 0 if error
    // be sure to DeleteSparseSet using the returned handle
    void* KLUSOLVEX_STDCALL NewSparseSet(unsigned int nBus);

    // return 1 if successful, 0 if not
    int KLUSOLVEX_STDCALL ZeroSparseSet(void* handle);

    // return 1 if successful, 2 if singular, 0 if other error
    int KLUSOLVEX_STDCALL FactorSparseMatrix(void* handle);

    /* 
	input: current injections in zero-based _acxB
	output: node voltages in zero-based _acxX
	no provision for voltage sources
	*/
    // return 1 if successful, 2 if singular, 0 if other error
    int KLUSOLVEX_STDCALL SolveSparseSet(void* handle, complex* acxX, complex* acxB);
    
    // return 1 if successful, 0 if not
    int KLUSOLVEX_STDCALL DeleteSparseSet(void* handle);

    /* i and j are 1-based for these */
    // return 1 if successful, 0 if not
    int KLUSOLVEX_STDCALL AddMatrixElement(void* handle, unsigned int i, unsigned int j, complex* pcxVal);
    int KLUSOLVEX_STDCALL GetMatrixElement(void* handle, unsigned int i, unsigned int j, complex* pcxVal);
    int KLUSOLVEX_STDCALL SetMatrixElement(void* handle, unsigned int i, unsigned int j, complex* pcxVal);

    // new functions
    int KLUSOLVEX_STDCALL SetLogFile(char* path, unsigned int iAction); // no-op in current version
    int KLUSOLVEX_STDCALL GetSize(void* handle, unsigned int* pResult);
    int KLUSOLVEX_STDCALL GetNNZ(void* handle, unsigned int* pResult);
    int KLUSOLVEX_STDCALL GetSparseNNZ(void* handle, unsigned int* pResult);
    int KLUSOLVEX_STDCALL GetRCond(void* handle, double* pResult);
    int KLUSOLVEX_STDCALL GetRGrowth(void* handle, double* pResult);
    int KLUSOLVEX_STDCALL GetCondEst(void* handle, double* pResult);
    int KLUSOLVEX_STDCALL GetFlops(void* handle, double* pResult);
    int KLUSOLVEX_STDCALL GetSingularCol(void* handle, unsigned int* pResult);

    int KLUSOLVEX_STDCALL AddPrimitiveMatrix(void* handle, unsigned int nOrder, unsigned int* pNodes, complex* pcY);
    int KLUSOLVEX_STDCALL GetCompressedMatrix(void* handle, unsigned int nColP, unsigned int nNZ, unsigned int* pColP, unsigned int* pRowIdx, complex* pcY);
    int KLUSOLVEX_STDCALL GetTripletMatrix(void* handle, unsigned int nNZ, unsigned int* pRows, unsigned int* pCols, complex* pcY);
    int KLUSOLVEX_STDCALL FindIslands(void* handle, unsigned int nOrder, unsigned int* pNodes);

    int KLUSOLVEX_STDCALL IncrementMatrixElement(void* handle, unsigned int i, unsigned int j, double re, double im);
    int KLUSOLVEX_STDCALL ZeroiseMatrixElement(void* handle, unsigned int i, unsigned int j);
    int KLUSOLVEX_STDCALL SaveAsMarketFiles(void* handle, const char* fileNameMatrix, const double *b, const char* fileNameVector);

    void KLUSOLVEX_STDCALL mvmult(int32_t N, complex* b, complex* A, complex* x);

    int32_t KLUSOLVEX_STDCALL klusolve_metis(
        int32_t *sorted_edge_pairs, // ([v1 v2] [v1 v3]) ...
        int32_t *edge_weights,
        int32_t num_edges,
        int32_t num_vertices, 
        int32_t num_partitions, 
        
        int32_t *zones, // size = num_vertices, allocated by caller
        int32_t *edge_cut // simple pointer
    );

#ifdef __cplusplus
}
#endif

#endif // #ifndef DSS_EXTENSIONS_KLUSOLVEX_H
