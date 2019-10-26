/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019, Paulo Meira                                           */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#ifndef dssx_klusolve_included
#define dssx_klusolve_included

#include "klu.h"

#ifdef _MSC_VER
#if _MSC_VER <= 1500
#include <stdint_compat.h>
#else
#ifdef __cplusplus
#include <cstdint>
#else // __cplusplus
#include <stdint.h>
#endif
#endif
#else
#ifdef __cplusplus
#include <cstdint>
#else // __cplusplus
#include <stdint.h>
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // return handle of new sparse set, 0 if error
    // be sure to DeleteSparseSet using the returned handle
    void* NewSparseSet(int nBus);

    // return 1 if successful, 0 if not
    int ZeroSparseSet(void* handle);

    // return 1 if successful, 2 if singular, 0 if other error
    int FactorSparseMatrix(void* handle);

    /* 
	input: current injections in zero-based _acxB
	output: node voltages in zero-based _acxX
	no provision for voltage sources
	*/
    // return 1 if successful, 2 if singular, 0 if other error
    int SolveSparseSet(void* handle, double* acxX, double* acxB);
    
    // return 1 if successful, 0 if not
    int DeleteSparseSet(void* handle);

    /* i and j are 1-based for these */
    // return 1 if successful, 0 if not
    int AddMatrixElement(void* handle, int i, int j, double* pcxVal);
    int GetMatrixElement(void* handle, int i, int j, double* pcxVal);

    // new functions
    int SetLogFile(char* path, int iAction); // no-op in current version
    int GetSize(void* handle, int* pResult);
    int GetNNZ(void* handle, int* pResult);
    int GetSparseNNZ(void* handle, int* pResult);
    int GetRCond(void* handle, double* pResult);
    int GetRGrowth(void* handle, double* pResult);
    int GetCondEst(void* handle, double* pResult);
    int GetFlops(void* handle, double* pResult);
    int GetSingularCol(void* handle, int* pResult);

    int AddPrimitiveMatrix(void* handle, int nOrder, int* pNodes, double* pcY);
    int GetCompressedMatrix(void* handle, int nColP, int nNZ, int* pColP, int* pRowIdx, double* pcY);
    int GetTripletMatrix(void* handle, int nNZ, int* pRows, int* pCols, double* pcY);
    int FindIslands(void* handle, int nOrder, int* pNodes);

    int IncrementMatrixElement(void* handle, int i, int j, double re, double im);
    int ZeroiseMatrixElement(void* handle, int i, int j);

    void mvmult(int32_t N, double* b, double* A, double* x);

#ifdef __cplusplus
}
#endif

#endif // dssx_klusolve_included
