/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2020, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#include "KLUSolveX.h"
#include "KLUSystemX.h"

#define SYMMETRIC_MATRIX

int SetLogFile(char*, int) // Unused, kept for potential backwards compatibility
{
    return 0;
}

void SetOptions(void* handle, uint64_t opts)
{
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(handle);
    if (!pSys) 
        return;
    
    pSys->options = opts;
}

void* NewSparseSet(int nBus)
{
    void* rc = 0;

    //	write_lfp ("NewSparseSet %u\n", nBus);

    KLUSystem* pSys = new KLUSystem();
    if (pSys)
    {
        pSys->Initialize(nBus, 0, nBus);
        rc = reinterpret_cast<void*>(pSys);
    }
    return rc;
}

int ZeroSparseSet(void* hSparse)
{
    unsigned long rc = 0;

    //	write_lfp ("ZeroSparseSet\n");

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        pSys->zero();
        pSys->bFactored = false;
        rc = 1;
    }
    return rc;
}

int FactorSparseMatrix(void* hSparse)
{
    int rc = 0;

    //	write_lfp ("FactorSparseMatrix\n");

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        if (pSys->FactorSystem() == 0)
        { // success
            rc = 1;
        }
        else
        { // singular
            rc = 2;
        }
    }
    return rc;
}

/* 
  input: current injections in zero-based _acxB
  output: node voltages in zero-based _acxX
  no provision for voltage sources
*/
int SolveSparseSet(void* hSparse, double* acxX, double* acxB)
{
    int rc = 0;

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        if (!pSys->bFactored || (pSys->reuseSymbolic && (pSys->options >= ReuseSymbolicFactorization)))
        {
            pSys->FactorSystem();
        }
        if (pSys->bFactored)
        {
            pSys->SolveSystem(reinterpret_cast<complex*>(acxX), reinterpret_cast<complex*>(acxB));
            rc = 1;
        }
        else
        {
            rc = 2;
        }
    }
    return rc;
}

int DeleteSparseSet(void* hSparse)
{
    int rc = 0;

    //	write_lfp ("DeleteSparseSet %u\n", hSparse);

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        delete pSys;
        rc = 1;
    }

    return rc;
}

/* i and j are 1-based for these */
int AddMatrixElement(void* hSparse, int i, int j, double* pcxVal)
{
    int rc = 0;

    //	write_lfp ("AddMatrixElement [%u,%u] = %G + j%G\n", i, j, pcxVal->x, pcxVal->y);

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        pSys->AddElement(i, j, *reinterpret_cast<complex*>(pcxVal), 1);
#ifdef SYMMETRIC_MATRIX
        if (i != j)
            pSys->AddElement(j, i, *reinterpret_cast<complex*>(pcxVal), 1);
#endif
        pSys->bFactored = false;
        rc = 1;
    }
    return rc;
}

int GetMatrixElement(void* hSparse, int i, int j, double* pcxVal)
{
    int rc = 0;

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        pSys->GetElement(i, j, *reinterpret_cast<complex*>(pcxVal));
        rc = 1;
    }
    return rc;
}

int IncrementMatrixElement(void* hSparse, int i, int j, double re, double im)
{
    int rc = 0;

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        rc = pSys->IncrementElement(i, j, re, im);
        if (rc)
        {
            pSys->bFactored = false;
            pSys->reuseSymbolic = true;
        }
        else
        {
            pSys->reuseSymbolic = false;
        }
    }
    return rc;
}

int ZeroiseMatrixElement(void* hSparse, int i, int j)
{
    int rc = 0;

    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        rc = pSys->ZeroiseElement(i, j);
        if (rc)
        {
            pSys->bFactored = false;
            pSys->reuseSymbolic = true;
        }
        else
        {
            pSys->reuseSymbolic = false;
        }
    }
    return rc;
}

// new functions
int GetSize(void* hSparse, int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetSize();
        rc = 1;
    }
    return rc;
}

int GetNNZ(void* hSparse, int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetNNZ();
        rc = 1;
    }
    return rc;
}

int GetSparseNNZ(void* hSparse, int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetSparseNNZ();
        rc = 1;
    }
    return rc;
}

int GetRCond(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetRCond();
        rc = 1;
    }
    return rc;
}

int GetRGrowth(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetRGrowth();
        rc = 1;
    }
    return rc;
}

int GetCondEst(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetCondEst();
        rc = 1;
    }
    return rc;
}

int GetFlops(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetFlops();
        rc = 1;
    }
    return rc;
}

int GetSingularCol(void* hSparse, int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetSingularCol();
        rc = 1;
    }
    return rc;
}

int AddPrimitiveMatrix(void* hSparse, int nOrder, int* pNodes, double* pcY)
{
    int rc = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        rc = pSys->AddPrimitiveMatrix(nOrder, pNodes, reinterpret_cast<complex*>(pcY));
        pSys->bFactored = false;
        pSys->reuseSymbolic = false;
    }
    return rc;
}

int GetCompressedMatrix(void* hSparse, int nColP, int nNZ, int* pColP, int* pRowIdx, double* pcY)
{
    int rc = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        if (pSys->GetCompressedMatrix(nColP, nNZ, pColP, pRowIdx, reinterpret_cast<complex*>(pcY)))
        {
            rc = 1;
        }
        else
        { // probably a size mismatch
            rc = 2;
        }
    }
    return rc;
}

int GetTripletMatrix(void* hSparse, int nNZ, int* pRows, int* pCols, double* pcY)
{
    int rc = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys)
    {
        if (pSys->GetTripletMatrix(nNZ, pRows, pCols, reinterpret_cast<complex*>(pcY)))
        {
            rc = 1;
        }
        else
        { // probably a size mismatch
            rc = 2;
        }
    }
    return rc;
}

int FindIslands(void* hSparse, int nOrder, int* pNodes)
{
    int rc = 0;
    KLUSystem* pSys = reinterpret_cast<KLUSystem*>(hSparse);
    if (pSys && nOrder >= pSys->GetSize())
    {
        rc = pSys->FindIslands(pNodes);
    }
    return rc;
}
