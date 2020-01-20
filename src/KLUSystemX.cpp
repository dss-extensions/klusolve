/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2020, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#include "KLUSystemX.h"
#include <algorithm>

KLUSystem::KLUSystem()
{
    InitDefaults();
}

KLUSystem::KLUSystem(int nBus, int nV, int nI)
{
    InitDefaults();
    Initialize(nBus, nV, nI);
}

KLUSystem::~KLUSystem()
{
    Clear();
}

void KLUSystem::ZeroIndices()
{
    m_nBus = m_nX = 0;
    m_NZpre = m_NZpost = 0;
    m_fltBus = 0;
}

void KLUSystem::NullPointers()
{
    Numeric = nullptr;
    Symbolic = nullptr;
}

void KLUSystem::InitDefaults()
{
    options = 0;

    m_nBus = 0;
    bFactored = false;
    reuseSymbolic = false;
    ZeroIndices();
    NullPointers();
}

void KLUSystem::Clear()
{
    spmat = SparseMatrix();
    triplets = std::vector<Eigen::Triplet<complex>>();

    if (Numeric)
        klu_z_free_numeric(&Numeric, &Common);
    if (Symbolic)
        klu_free_symbolic(&Symbolic, &Common);

    ZeroIndices();
    NullPointers();
}

int KLUSystem::Initialize(int nBus, int nV, int nI)
{
    Clear();

    klu_defaults(&Common);
    Common.halt_if_singular = 0;

    m_nBus = m_nX = nBus;

    spmat = SparseMatrix(m_nX, m_nX);
    spmat.reserve(4 * size_t(m_nX));
    return 0;
}

int KLUSystem::FactorSystem()
{
    bFactored = false;

    int rc = Factor();

    if (rc == 1)
    {
        bFactored = true;
        reuseSymbolic = false;
        return 0;
    }
    return 1;
}

void KLUSystem::SolveSystem(complex* acxX, complex* acxB)
{
    memcpy(&acxX[0], acxB, sizeof(complex) * m_nBus);
    Solve(&acxX[0]);
}

int KLUSystem::AddPrimitiveMatrix(int nOrder, int* pNodes, complex* pMat)
{
    int i, j, idRow, idCol, idVal;
    double re, im;

    // check the node numbers
    for (i = 0; i < nOrder; i++)
    {
        if (pNodes[i] > m_nBus)
            return 0;
    }

    // add the matrix
    for (i = 0; i < nOrder; i++)
    {
        if (pNodes[i] < 1)
            continue; // skip ground

        idVal = i;
        idRow = pNodes[i] - 1; // convert to zero-based
        for (j = 0; j < nOrder; j++)
        {
            if (pNodes[j])
            {
                idCol = pNodes[j] - 1;
                re = pMat[idVal].real();
                im = pMat[idVal].imag();
                if (re != 0.0 || im != 0.0)
                {
                    // stuff this value into the correct partition
                    triplets.push_back({ idRow, idCol, pMat[idVal] });
                    //spmat.insert(idRow, idCol) = pMat[idVal];
                }
            }
            // always step through values, even if we don't use them
            idVal += nOrder;
        }
    }

    return 1;
}

void KLUSystem::ProcessTriplets()
{
    spmat.setFromTriplets(triplets.begin(), triplets.end());
    m_NZpre = spmat.nonZeros();
    triplets = std::vector<Eigen::Triplet<complex>>();
}

int KLUSystem::Factor()
{
    // first convert the triplets to column-compressed form, and prep the columns
    if (triplets.size())
    {
        ProcessTriplets();
        spmat.makeCompressed(); // should be a no-op here
    }
    else if ((options != ReuseCompressedMatrix) && !(reuseSymbolic && (options >= ReuseSymbolicFactorization)))
    {
        // otherwise, compression and factoring has already been done
        if (m_fltBus)
            return -1; // was found singular before
        return 1; // was found okay before
    }

    // then factor Y22
    if (!(reuseSymbolic && (options >= ReuseSymbolicFactorization)))
    {
        if (Symbolic)
            klu_free_symbolic(&Symbolic, &Common);
        Symbolic = nullptr;
    }
    if (!(reuseSymbolic && (options >= ReuseSymbolicFactorization)) || !(Numeric && (options >= ReuseNumericFactorization)))
    {
        if (Numeric)
            klu_z_free_numeric(&Numeric, &Common);
        Numeric = nullptr;
    }

    bool reuseFailed = true;

    if ((reuseSymbolic && (options >= ReuseSymbolicFactorization)) && Symbolic)
    {
        if (Numeric && (options >= ReuseNumericFactorization))
        {
            // If refactorization has failed, run the full numeric factorization
            reuseFailed = klu_z_refactor(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common) != 1;
        }
        else
        {
            // Not allowed to reuse the numeric factorization, run the full version
            if (Numeric)
                klu_z_free_numeric(&Numeric, &Common);

            Numeric = klu_z_factor(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, &Common);

            if (Common.status == KLU_OK)
                reuseFailed = false;
        }
        // TODO: handle this? (from the manual)
        //   "Since this can lead to numeric instability, the use of klu
        //    rcond, klu rgrowth, or klu condest is recommended to check the
        //    accuracy of the resulting factorization."
        //
        // if (!reuseFailed )
        // {
        // klu_z_condest(spmat.outerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common);
        // if (Common.condest ...)
        // {
        // failed = true;
        // }
        // }
    }

    if (reuseFailed)
    {
        Symbolic = klu_analyze(spmat.rows(), spmat.outerIndexPtr(), spmat.innerIndexPtr(), &Common);
        Numeric = klu_z_factor(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, &Common);
    }

    m_fltBus = Common.singular_col;
    if (Common.singular_col < spmat.rows())
    {
        ++m_fltBus; // for 1-based NexHarm row numbers
        // m_fltBus += 0; // skip over the voltage source buses
    }
    else
    {
        m_fltBus = 0; // no singular submatrix was found
    }

    if (Common.status == KLU_OK)
    {
        // compute size of the factorization
        m_NZpost += (Numeric->lnz + Numeric->unz - Numeric->n + ((Numeric->Offp) ? (Numeric->Offp[Numeric->n]) : 0));
        return 1;
    }
    else if (Common.status == KLU_SINGULAR)
    {
        return -1;
    }
    else
    {
        // KLU_OUT_OF_MEMORY, KLU_INVALID, or KLU_TOO_LARGE
        if (!m_fltBus)
        {
            m_fltBus = 1; // this is the flag for unsuccessful factorization
        }
        return 0;
    }

    return 1;
}

void KLUSystem::Solve(complex* acxVbus)
{
    if (m_nX < 1)
        return; // nothing to do

    klu_z_solve(Symbolic, Numeric, spmat.rows(), 1, reinterpret_cast<double*>(acxVbus), &Common);
}

double KLUSystem::GetRCond()
{
    klu_z_rcond(Symbolic, Numeric, &Common);
    return Common.rcond;
}

double KLUSystem::GetRGrowth()
{
    if (spmat.rows() == 0)
        return 0.0;
    if (klu_z_rgrowth(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common) == 1)
        return Common.rgrowth;

    return -1;
}

double KLUSystem::GetCondEst()
{
    if (spmat.rows() == 0)
        return 0.0;
    klu_z_condest(spmat.outerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common);
    return Common.condest;
}

double KLUSystem::GetFlops()
{
    klu_z_flops(Symbolic, Numeric, &Common);
    return Common.flops;
}

int KLUSystem::FindDisconnectedSubnetwork()
{
    Factor();

    return m_fltBus;
}

// stack-based DFS from Sedgewick
static int* stack = nullptr;
static int stk_p = 0;
static void push(int v) { stack[stk_p++] = v; }
static int pop() { return stack[--stk_p]; }
static void stackfree()
{
    if (!stack)
        return;
    delete[] stack;
    stack = nullptr;
}
static int stackempty() { return !stk_p; }
static void stackinit(size_t size)
{
    stackfree();
    stack = new int[size + 1];
    stk_p = 0;
}

static void mark_dfs(int j, int cnt, int* Ap, int* Ai, int* clique)
{
    int i, k;

    push(j);
    while (!stackempty())
    {
        j = pop();
        clique[j] = cnt;
        for (k = Ap[j]; k < Ap[j + 1]; k++)
        {
            i = Ai[k];
            if (clique[i] == 0)
            {
                push(i);
                clique[i] = -1; // to only push once
            }
        }
    }
}

// The KLU factorization might have some information about cliques in Y22 only,
//   but we want to consider the whole system, so this function
//   performs a new DFS on the compressed non-zero pattern
// This function could behave differently than before,
//   since the compression process removes numerical zero elements
int KLUSystem::FindIslands(int* idClique)
{
    Factor();

    int* clique = new int[m_nBus];
    int* Ap = spmat.outerIndexPtr();
    int* Ai = spmat.innerIndexPtr();
    int j;

    // DFS down the columns
    int cnt = 0;
    for (j = 0; j < m_nBus; j++)
        clique[j] = cnt; // use to mark the nodes with clique #
    stackinit(m_nBus);
    for (j = 0; j < m_nBus; j++)
    {
        if (clique[j] == 0)
        { // have not visited this column yet
            ++cnt;
            mark_dfs(j, cnt, Ap, Ai, clique);
        }
    }

    for (j = 0; j < m_nBus; j++)
        idClique[j] = clique[j];

    delete[] clique;

    return cnt;
}

void KLUSystem::zero()
{
    Initialize(m_nBus, 0, m_nBus);
}

void KLUSystem::AddElement(int iRow, int iCol, complex& cpxVal, int)
{
    if (iRow > m_nBus || iCol > m_nBus)
        return;
    if (iRow <= 0 || iCol <= 0)
        return;
    if (cpxVal.real() == 0.0 && cpxVal.imag() == 0.0)
        return;

    if (spmat.nonZeros())
    {
        spmat.coeffRef(iRow - 1, iCol - 1) += cpxVal;
    }
    else
    {
        triplets.push_back({ iRow - 1, iCol - 1, cpxVal });
    }
}

void KLUSystem::GetElement(int iRow, int iCol, complex& cpxVal)
{
    cpxVal = complex(0.0);

    if (iRow > m_nBus || iCol > m_nBus)
        return;
    if (iRow == 0 || iCol == 0)
        return;

    cpxVal = spmat.coeff(iRow - 1, iCol - 1);
}

int KLUSystem::IncrementElement(int iRow, int iCol, double re, double im)
{
    if ((options < ReuseCompressedMatrix) || (iRow > m_nBus || iCol > m_nBus) || (iRow == 0 || iCol == 0))
        return 0;

    --iRow;
    --iCol;

    complex* colData;
    int rowIdxInCol;

    complex* Ax = spmat.valuePtr();
    int* Ap = spmat.outerIndexPtr();
    int* Ai = spmat.innerIndexPtr();
    const int numColElements = Ap[iCol + 1] - Ap[iCol];
    const int* it_begin = (Ai) + Ap[iCol];
    const int* it_end = (Ai) + Ap[iCol + 1];

    const int* it = std::lower_bound(it_begin, it_end, iRow);
    if (it == it_end || (*it != iRow))
        return 0; // no row

    rowIdxInCol = (it - it_begin);
    colData = Ax + Ap[iCol];
    colData[rowIdxInCol] += complex(re, im);

    return 1;
}

int KLUSystem::ZeroiseElement(int iRow, int iCol)
{
    if ((options < ReuseCompressedMatrix) || (iRow > m_nBus || iCol > m_nBus) || (iRow == 0 || iCol == 0))
        return 0;

    --iRow;
    --iCol;

    complex* colData;
    int rowIdxInCol;

    complex* Ax = spmat.valuePtr();
    int* Ap = spmat.outerIndexPtr();
    int* Ai = spmat.innerIndexPtr();
    const int numColElements = Ap[iCol + 1] - Ap[iCol];
    const int* it_begin = (Ai) + Ap[iCol];
    const int* it_end = (Ai) + Ap[iCol + 1];

    const int* it = std::lower_bound(it_begin, it_end, iRow);
    if (it == it_end || (*it != iRow))
        return 0; // no row

    rowIdxInCol = (it - it_begin);
    colData = Ax + Ap[iCol];
    colData[rowIdxInCol] = 0;
    
    return 1;
}

int KLUSystem::GetCompressedMatrix(int nColP, int nNZ, int* pColP, int* pRowIdx, complex* pMat)
{
    if (triplets.size())
    {
        ProcessTriplets();
    }

    if (!spmat.isCompressed())
        spmat.makeCompressed();

    if (nNZ < spmat.nonZeros() || nColP <= m_nBus || !spmat.nonZeros())
        return 0;

    complex* Ax = spmat.valuePtr();
    int* Ap = spmat.outerIndexPtr();
    int* Ai = spmat.innerIndexPtr();

    memcpy(pMat, Ax, spmat.nonZeros() * sizeof(complex));
    memcpy(pColP, Ap, (spmat.cols() + 1) * sizeof(int));
    memcpy(pRowIdx, Ai, spmat.nonZeros() * sizeof(int));

    return spmat.nonZeros();
}

int KLUSystem::GetTripletMatrix(int nNZ, int* pRows, int* pCols, complex* pMat)
{
    if (triplets.size())
        ProcessTriplets();

    if (!spmat.isCompressed())
        spmat.makeCompressed();

    if (nNZ < spmat.nonZeros() || !spmat.nonZeros())
        return 0;

    for (int k = 0; k < spmat.outerSize(); ++k)
    {
        for (SparseMatrix::InnerIterator it(spmat, k); it; ++it)
        {
            *(pMat++) = it.value();
            *(pRows++) = it.row();
            *(pCols++) = it.col();
        }
    }
    return spmat.nonZeros();
}
