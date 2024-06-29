/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2024, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#include "KLUSolveX.h"
#include "KLUSystemX.h"

using KLUSolveX::KLUSystemX;

int KLUSOLVEX_STDCALL SetLogFile(char*, unsigned int) // Unused, kept for potential backwards compatibility
{
    return 0;
}

void KLUSOLVEX_STDCALL SetOptions(void* handle, uint64_t opts)
{
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(handle);
    if (!pSys) 
        return;
    
    int32_t previousFormat = pSys->dataFormat;
    pSys->options = opts & ~0x00F0;
    pSys->dataFormat = opts & 0x00F0;

    if (previousFormat != pSys->dataFormat)
    {
        pSys->Initialize(pSys->m_nBus, 0, 0);
    }
}

void* KLUSOLVEX_STDCALL NewSparseSet(unsigned int nBus)
{
    void* rc = 0;

    //	write_lfp ("NewSparseSet %u\n", nBus);

    KLUSystemX* pSys = new KLUSystemX();
    if (pSys)
    {
        pSys->Initialize(nBus, 0, nBus);
        rc = reinterpret_cast<void*>(pSys);
    }
    return rc;
}

int KLUSOLVEX_STDCALL ZeroSparseSet(void* hSparse)
{
    unsigned long rc = 0;

    //	write_lfp ("ZeroSparseSet\n");

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        pSys->zero();
        pSys->bFactored = false;
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL FactorSparseMatrix(void* hSparse)
{
    int rc = 0;

    //	write_lfp ("FactorSparseMatrix\n");

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
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
int KLUSOLVEX_STDCALL SolveSparseSet(void* hSparse, complex* acxX, complex* acxB)
{
    int rc = 0;

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        if (!pSys->bFactored || (pSys->reuseSymbolic && (pSys->options >= ReuseSymbolicFactorization)))
        {
            pSys->FactorSystem();
        }
        if (pSys->bFactored)
        {
            pSys->SolveSystem(reinterpret_cast<KLUSolveX::complex*>(acxX), reinterpret_cast<KLUSolveX::complex*>(acxB));
            rc = 1;
        }
        else
        {
            rc = 2;
        }
    }
    return rc;
}

int KLUSOLVEX_STDCALL DeleteSparseSet(void* hSparse)
{
    int rc = 0;

    //	write_lfp ("DeleteSparseSet %u\n", hSparse);

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        delete pSys;
        rc = 1;
    }

    return rc;
}

/* i and j are 1-based for these */
int KLUSOLVEX_STDCALL AddMatrixElement(void* hSparse, unsigned int i, unsigned int j, complex* pcxVal)
{
    int rc = 0;

    //	write_lfp ("AddMatrixElement [%u,%u] = %G + j%G\n", i, j, pcxVal->x, pcxVal->y);

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        pSys->AddElement(i, j, *reinterpret_cast<KLUSolveX::complex*>(pcxVal), 1);
        if (i != j)
            pSys->AddElement(j, i, *reinterpret_cast<KLUSolveX::complex*>(pcxVal), 1);

        pSys->bFactored = false;
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL SetMatrixElement(void* hSparse, unsigned int i, unsigned int j, complex* pcxVal)
{
    int rc = 0;

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        pSys->AddElement(i, j, *reinterpret_cast<KLUSolveX::complex*>(pcxVal), 1);
        pSys->bFactored = false;
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetMatrixElement(void* hSparse, unsigned int i, unsigned int j, complex* pcxVal)
{
    int rc = 0;

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        pSys->GetElement(i, j, *reinterpret_cast<KLUSolveX::complex*>(pcxVal));
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL IncrementMatrixElement(void* hSparse, unsigned int i, unsigned int j, double re, double im)
{
    int rc = 0;

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
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

int KLUSOLVEX_STDCALL ZeroiseMatrixElement(void* hSparse, unsigned int i, unsigned int j)
{
    int rc = 0;

    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
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
int KLUSOLVEX_STDCALL GetSize(void* hSparse, unsigned int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetSize();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetNNZ(void* hSparse, unsigned int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetNNZ();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetSparseNNZ(void* hSparse, unsigned int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetSparseNNZ();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetRCond(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetRCond();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetRGrowth(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetRGrowth();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetCondEst(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetCondEst();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetFlops(void* hSparse, double* pResult)
{
    int rc = 0;
    *pResult = 0.0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetFlops();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetSingularCol(void* hSparse, unsigned int* pResult)
{
    int rc = 0;
    *pResult = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        *pResult = pSys->GetSingularCol();
        rc = 1;
    }
    return rc;
}

int KLUSOLVEX_STDCALL AddPrimitiveMatrix(void* hSparse, unsigned int nOrder, unsigned int* pNodes, complex* pcY)
{
    int rc = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        rc = pSys->AddPrimitiveMatrix(nOrder, pNodes, reinterpret_cast<KLUSolveX::complex*>(pcY));
        pSys->bFactored = false;
        pSys->reuseSymbolic = false;
    }
    return rc;
}

int KLUSOLVEX_STDCALL GetCompressedMatrix(void* hSparse, unsigned int nColP, unsigned int nNZ, unsigned int* pColP, unsigned int* pRowIdx, complex* pcY)
{
    int rc = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        if (pSys->GetCompressedMatrix(nColP, nNZ, pColP, pRowIdx, reinterpret_cast<KLUSolveX::complex*>(pcY)))
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

int KLUSOLVEX_STDCALL GetTripletMatrix(void* hSparse, unsigned int nNZ, unsigned int* pRows, unsigned int* pCols, complex* pcY)
{
    int rc = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        if (pSys->GetTripletMatrix(nNZ, pRows, pCols, reinterpret_cast<KLUSolveX::complex*>(pcY)))
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

int KLUSOLVEX_STDCALL FindIslands(void* hSparse, unsigned int nOrder, unsigned int* pNodes)
{
    int rc = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys && nOrder >= pSys->GetSize())
    {
        rc = pSys->FindIslands(pNodes);
    }
    return rc;
}

int KLUSOLVEX_STDCALL SaveAsMarketFiles(void* hSparse, const char* fileNameMatrix, const double *b, const char* fileNameVector)
{
    int rc = 0;
    KLUSystemX* pSys = reinterpret_cast<KLUSystemX*>(hSparse);
    if (pSys)
    {
        rc = pSys->SaveAsMarketFiles(fileNameMatrix, b, fileNameVector);
    }
    return rc;
}
