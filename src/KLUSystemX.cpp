/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2024, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#include "KLUSystemX.h"
#include <algorithm>
#include <unsupported/Eigen/SparseExtra>

namespace KLUSolveX {

using std::size_t;

KLUSystemX::KLUSystemX()
{
    InitDefaults();
}

KLUSystemX::KLUSystemX(unsigned int nBus, unsigned int nV, unsigned int nI)
{
    InitDefaults();
    Initialize(nBus, nV, nI);
}

KLUSystemX::~KLUSystemX()
{
    Clear();
}

void KLUSystemX::ZeroIndices()
{
    m_nBus = m_nX = 0;
    m_NZpre = m_NZpost = 0;
    m_fltBus = 0;
}

void KLUSystemX::NullPointers()
{
    Numeric = nullptr;
    Symbolic = nullptr;
}

void KLUSystemX::InitDefaults()
{
    options = 0;
    dataFormat = 0;

    m_nBus = 0;
    bFactored = false;
    reuseSymbolic = false;
    ZeroIndices();
    NullPointers();
}

void KLUSystemX::Clear()
{
    spmat = SparseMatrix();
    triplets = std::vector<Eigen::Triplet<complex>>();

    if (Numeric)
        klu_free_numeric(&Numeric, &Common);
    if (Symbolic)
        klu_free_symbolic(&Symbolic, &Common);

    ZeroIndices();
    NullPointers();
}

int KLUSystemX::Initialize(unsigned int nBus, unsigned int nV, unsigned int nI)
{
    Clear();

    klu_defaults(&Common);
    Common.halt_if_singular = 0;

    m_nBus = m_nX = nBus;

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            spmat_f64 = SparseMatrixF64(m_nX, m_nX);
            spmat_f64.reserve(4 * size_t(m_nX));
            break;
        default:
            spmat = SparseMatrix(m_nX, m_nX);
            spmat.reserve(4 * size_t(m_nX));
            break;
    }    
    return 0;
}

int KLUSystemX::FactorSystem()
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

void KLUSystemX::SolveSystem(complex* acxX, complex* acxB)
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            memcpy(&acxX[0], acxB, sizeof(double) * m_nBus);
            Solve(&acxX[0]);
            return;
        default:
            memcpy(&acxX[0], acxB, sizeof(complex) * m_nBus);
            Solve(&acxX[0]);
            return;
    }
}

int KLUSystemX::AddPrimitiveMatrix(unsigned int nOrder, unsigned int* pNodes, complex* pMat)
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

void KLUSystemX::ProcessTriplets()
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
        {
            std::vector<Eigen::Triplet<double> > triplets_f64;
            triplets_f64.reserve(triplets.size());
            for (auto &t: triplets)
            {
                triplets_f64.push_back({t.row(), t.col(), t.value().real()});
            }
            spmat_f64.setFromTriplets(triplets_f64.begin(), triplets_f64.end());
            m_NZpre = spmat_f64.nonZeros();
            break;
        }
        default:
            spmat.setFromTriplets(triplets.begin(), triplets.end());
            m_NZpre = spmat.nonZeros();
            break;
    }
    triplets = std::vector<Eigen::Triplet<complex>>();
}

int KLUSystemX::Factor()
{
    int32_t nrows;
    // first convert the triplets to column-compressed form, and prep the columns
    if (triplets.size())
    {
        ProcessTriplets();
        switch (dataFormat)
        {
            case MatrixFormat_DoublePrecisionReal:
                spmat_f64.makeCompressed(); // should be a no-op here
                nrows = spmat_f64.rows();
                break;
            default:
                spmat.makeCompressed(); // should be a no-op here
                nrows = spmat.rows();
                break;
        }
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
        {
            klu_free_numeric(&Numeric, &Common); // Works for everything
        }
        Numeric = nullptr;
    }

    bool reuseFailed = true;

    if ((reuseSymbolic && (options >= ReuseSymbolicFactorization)) && Symbolic)
    {
        if (Numeric && (options >= ReuseNumericFactorization))
        {
            // If refactorization has failed, run the full numeric factorization
            switch (dataFormat)
            {
                case MatrixFormat_DoublePrecisionReal:
                    reuseFailed = klu_refactor(spmat_f64.outerIndexPtr(), spmat_f64.innerIndexPtr(), reinterpret_cast<double*>(spmat_f64.valuePtr()), Symbolic, Numeric, &Common) != 1;
                    break;
                default:
                    reuseFailed = klu_z_refactor(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common) != 1;
                    break;
            }
        }
        else
        {
            // Not allowed to reuse the numeric factorization, run the full version
            if (Numeric)
            {
                klu_free_numeric(&Numeric, &Common);
            }
            switch (dataFormat)
            {
                case MatrixFormat_DoublePrecisionReal:
                    Numeric = klu_factor(spmat_f64.outerIndexPtr(), spmat_f64.innerIndexPtr(), reinterpret_cast<double*>(spmat_f64.valuePtr()), Symbolic, &Common);
                    break;
                default:
                    Numeric = klu_z_factor(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, &Common);
                    break;
            }

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
        switch (dataFormat)
        {
            case MatrixFormat_DoublePrecisionReal:
                Symbolic = klu_analyze(spmat_f64.rows(), spmat_f64.outerIndexPtr(), spmat_f64.innerIndexPtr(), &Common);
                Numeric = klu_factor(spmat_f64.outerIndexPtr(), spmat_f64.innerIndexPtr(), reinterpret_cast<double*>(spmat_f64.valuePtr()), Symbolic, &Common);
                break;
            default:
                Symbolic = klu_analyze(spmat.rows(), spmat.outerIndexPtr(), spmat.innerIndexPtr(), &Common);
                Numeric = klu_z_factor(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, &Common);
                break;
        }
    }

    m_fltBus = Common.singular_col;
    if (Common.singular_col < nrows)
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

void KLUSystemX::Solve(complex* acxVbus)
{
    if (m_nX < 1)
        return; // nothing to do

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            klu_solve(Symbolic, Numeric, spmat_f64.rows(), 1, reinterpret_cast<double*>(acxVbus), &Common);
            break;
        default:
            klu_z_solve(Symbolic, Numeric, spmat.rows(), 1, reinterpret_cast<double*>(acxVbus), &Common);
            break;
    }
}

double KLUSystemX::GetRCond()
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            klu_rcond(Symbolic, Numeric, &Common);
            break;
        default:
            klu_z_rcond(Symbolic, Numeric, &Common);
            break;
    }
    return Common.rcond;
}

double KLUSystemX::GetRGrowth()
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            if (spmat_f64.rows() == 0)
                return 0.0;
            if (klu_rgrowth(spmat_f64.outerIndexPtr(), spmat_f64.innerIndexPtr(), reinterpret_cast<double*>(spmat_f64.valuePtr()), Symbolic, Numeric, &Common) == 1)
                return Common.rgrowth;
            break;
        default:
            if (spmat.rows() == 0)
                return 0.0;
            if (klu_z_rgrowth(spmat.outerIndexPtr(), spmat.innerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common) == 1)
                return Common.rgrowth;
            break;
    }
    return -1;
}

double KLUSystemX::GetCondEst()
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            if (spmat_f64.rows() == 0)
                return 0.0;
            klu_condest(spmat_f64.outerIndexPtr(), reinterpret_cast<double*>(spmat_f64.valuePtr()), Symbolic, Numeric, &Common);
            break;
        default:
            if (spmat.rows() == 0)
                return 0.0;
            klu_z_condest(spmat.outerIndexPtr(), reinterpret_cast<double*>(spmat.valuePtr()), Symbolic, Numeric, &Common);
            break;
    }
    return Common.condest;
}

double KLUSystemX::GetFlops()
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            klu_flops(Symbolic, Numeric, &Common);
            break;
        default:
            klu_z_flops(Symbolic, Numeric, &Common);
            break;
    }    
    return Common.flops;
}

int KLUSystemX::FindDisconnectedSubnetwork()
{
    Factor();

    return m_fltBus;
}

// stack-based DFS from Sedgewick
static void mark_dfs(std::vector<int> &stack, unsigned int j, unsigned int cnt, int* Ap, int* Ai, int* clique)
{
    int i, k;
    stack.push_back(j);
    while (!stack.empty())
    {
        j = stack.back();
        stack.pop_back();
        clique[j] = cnt;
        for (k = Ap[j]; k < Ap[j + 1]; k++)
        {
            i = Ai[k];
            if (clique[i] == 0)
            {
                stack.push_back(i);
                clique[i] = (unsigned int) -1; // to only push once
            }
        }
    }
}

// The KLU factorization might have some information about cliques in Y22 only,
//   but we want to consider the whole system, so this function
//   performs a new DFS on the compressed non-zero pattern
// This function could behave differently than before,
//   since the compression process removes numerical zero elements
int KLUSystemX::FindIslands(unsigned int* idClique)
{
    Factor();

    int* clique = new int[m_nBus];
    int* Ap;
    int* Ai;
    int j;

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            Ap = spmat_f64.outerIndexPtr();
            Ai = spmat_f64.innerIndexPtr();
            break;
        default:
            Ap = spmat.outerIndexPtr();
            Ai = spmat.innerIndexPtr();
            break;
    }

    // DFS down the columns
    int cnt = 0;
    for (j = 0; j < m_nBus; j++)
        clique[j] = cnt; // use to mark the nodes with clique #
    
    std::vector<int> stack;
    stack.reserve(m_nBus + 1);
    for (j = 0; j < m_nBus; j++)
    {
        if (clique[j] == 0)
        { // have not visited this column yet
            ++cnt;
            mark_dfs(stack, j, cnt, Ap, Ai, clique);
        }
    }

    for (j = 0; j < m_nBus; j++)
        idClique[j] = clique[j];

    delete[] clique;

    return cnt;
}

void KLUSystemX::zero()
{
    Initialize(m_nBus, 0, m_nBus);
}

void KLUSystemX::AddElement(unsigned int iRow, unsigned int iCol, complex& cpxVal, int)
{
    if (iRow > m_nBus || iCol > m_nBus)
        return;
    if (iRow <= 0 || iCol <= 0)
        return;
    if (cpxVal.real() == 0.0 && cpxVal.imag() == 0.0)
        return;

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            if (spmat_f64.nonZeros())
            {
                spmat_f64.coeffRef(iRow - 1, iCol - 1) += cpxVal.real();
                return;
            }
            break;
        default:
            if (spmat.nonZeros())
            {
                spmat.coeffRef(iRow - 1, iCol - 1) += cpxVal;
                return;
            }
            break;
    }
    triplets.push_back({ static_cast<int>(iRow) - 1, static_cast<int>(iCol) - 1, cpxVal });
}

void KLUSystemX::GetElement(unsigned int iRow, unsigned int iCol, complex& cpxVal)
{
    cpxVal = complex(0.0);

    if (iRow > m_nBus || iCol > m_nBus)
        return;
    if (iRow == 0 || iCol == 0)
        return;

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
            cpxVal = spmat_f64.coeff(iRow - 1, iCol - 1);
            return;
        default:
            cpxVal = spmat.coeff(iRow - 1, iCol - 1);
            return;
    }
}

int KLUSystemX::IncrementElement(unsigned int iRow, unsigned int iCol, double re, double im)
{
    if ((options < ReuseCompressedMatrix) || (iRow > m_nBus || iCol > m_nBus) || (iRow == 0 || iCol == 0))
        return 0;

    --iRow;
    --iCol;

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
        {
            double* colData;
            int rowIdxInCol;

            double* Ax = spmat_f64.valuePtr();
            int* Ap = spmat_f64.outerIndexPtr();
            int* Ai = spmat_f64.innerIndexPtr();
            const int numColElements = Ap[iCol + 1] - Ap[iCol];
            const int* it_begin = (Ai) + Ap[iCol];
            const int* it_end = (Ai) + Ap[iCol + 1];

            const int* it = std::lower_bound(it_begin, it_end, iRow);
            if (it == it_end || (*it != iRow))
                return 0; // no row

            rowIdxInCol = (it - it_begin);
            colData = Ax + Ap[iCol];
            colData[rowIdxInCol] += re;
        }
        default:
        {
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
        }
    }
    return 1;
}

int KLUSystemX::ZeroiseElement(unsigned int iRow, unsigned int iCol)
{
    if ((options < ReuseCompressedMatrix) || (iRow > m_nBus || iCol > m_nBus) || (iRow == 0 || iCol == 0))
        return 0;

    --iRow;
    --iCol;

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:
        {
            double* colData;
            int rowIdxInCol;

            double* Ax = spmat_f64.valuePtr();
            int* Ap = spmat_f64.outerIndexPtr();
            int* Ai = spmat_f64.innerIndexPtr();
            const int numColElements = Ap[iCol + 1] - Ap[iCol];
            const int* it_begin = (Ai) + Ap[iCol];
            const int* it_end = (Ai) + Ap[iCol + 1];

            const int* it = std::lower_bound(it_begin, it_end, iRow);
            if (it == it_end || (*it != iRow))
                return 0; // no row

            rowIdxInCol = (it - it_begin);
            colData = Ax + Ap[iCol];
            colData[rowIdxInCol] = 0;
            break;
        }
        default:
        {
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
            break;
        }
    }    
    return 1;
}

int KLUSystemX::GetCompressedMatrix(unsigned int nColP, unsigned int nNZ, unsigned int* pColP, unsigned int* pRowIdx, complex* pMat)
{
    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:  
        {
            if (triplets.size())
            {
                ProcessTriplets();
            }

            if (!spmat_f64.isCompressed())
                spmat_f64.makeCompressed();

            if (nNZ < spmat_f64.nonZeros() || nColP <= m_nBus || !spmat_f64.nonZeros())
                return 0;

            double* Ax = spmat_f64.valuePtr();
            int* Ap = spmat_f64.outerIndexPtr();
            int* Ai = spmat_f64.innerIndexPtr();

            memcpy(pMat, Ax, spmat_f64.nonZeros() * sizeof(complex));
            memcpy(pColP, Ap, (spmat_f64.cols() + 1) * sizeof(int));
            memcpy(pRowIdx, Ai, spmat_f64.nonZeros() * sizeof(int));

            return spmat_f64.nonZeros();
        }
        default:
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
    }
}

int KLUSystemX::GetTripletMatrix(unsigned int nNZ, unsigned int* pRows, unsigned int* pCols, complex* pMat)
{
    if (triplets.size())
        ProcessTriplets();

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:  
        {
            if (!spmat_f64.isCompressed())
                spmat_f64.makeCompressed();

            if (nNZ < spmat_f64.nonZeros() || !spmat_f64.nonZeros())
                return 0;

            for (int k = 0; k < spmat_f64.outerSize(); ++k)
            {
                for (SparseMatrixF64::InnerIterator it(spmat_f64, k); it; ++it)
                {
                    *(pMat++) = it.value();
                    *(pRows++) = it.row();
                    *(pCols++) = it.col();
                }
            }
            return spmat_f64.nonZeros();
        }
        default:
        {
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
    }
}

int KLUSystemX::SaveAsMarketFiles(const char* fileNameMatrix, const double *b, const char* fileNameVector)
{
    bool res = 0;
    if (triplets.size())
        ProcessTriplets();

    switch (dataFormat)
    {
        case MatrixFormat_DoublePrecisionReal:  
            res = Eigen::saveMarket(spmat_f64, fileNameMatrix);
            if (!res)
            {
                return 0;
            }
            if (b)
            {
                Eigen::VectorXd Bcopy = Eigen::Map<const Eigen::VectorXd>(b, spmat_f64.rows());
                res = Eigen::saveMarketVector(Bcopy, fileNameVector);
            }
            break;
        default:
        {
            res = Eigen::saveMarket(spmat, fileNameMatrix);
            if (!res)
            {
                return 0;
            }
            if (b)
            {
                res = Eigen::saveMarketVector(Eigen::Map<const Eigen::VectorXcd>(reinterpret_cast<const complex*>(b), spmat.rows()), fileNameVector);
            }
            break;
        }
    }
    if (!res)
    {
        return 0;
    }
    return 1;
}

} // namespace KLUSolveX