/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2024, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#ifndef DSS_EXTENSIONS_KLUSYSTEMX_H
#define DSS_EXTENSIONS_KLUSYSTEMX_H

#include "KLUSolveX.h"
#include <Eigen/SparseCore>
#include "klu.h"

namespace KLUSolveX {

typedef std::complex<double> complex;

struct matrix_complex
{
    complex* acx;
    unsigned int nRow, nCol;
    complex get_acx(unsigned int i, unsigned int j)
    {
        return acx[i * nCol + j];
    }
};

/* Kron reduction not supported, this version just solves

|Y22| * |V| = |I|

KLU manages complex values as interleaved real/imag in double arrays
KLU arrays are zero-based
*/

class KLUSystemX
{
public:
    typedef Eigen::SparseMatrix<complex> SparseMatrix;
    typedef Eigen::SparseMatrix<double> SparseMatrixF64;
    typedef Eigen::SparseMatrix<float> SparseMatrixF32;
    typedef Eigen::SparseMatrix<std::complex<float>> SparseMatrixC64;

    // admittance matrix blocks in compressed-column storage, like Matlab
    SparseMatrix spmat;
    SparseMatrixF64 spmat_f64;
    SparseMatrixF32 spmat_f32;
    SparseMatrixC64 spmat_c64;

    std::vector<Eigen::Triplet<complex> > triplets;
    std::vector<complex> acx;

    klu_symbolic* Symbolic;
    klu_numeric* Numeric;
    klu_common Common;

    uint32_t m_nBus; // number of nodes
    uint32_t m_nX; // number of unknown voltages, hardwired to m_nBus
    uint32_t m_NZpre; // number of non-zero entries before factoring
    uint32_t m_NZpost; // number of non-zero entries after factoring
    uint32_t m_fltBus; // row number of a bus causing singularity

    void InitDefaults();
    void Clear();
    void ZeroIndices();
    void NullPointers();
    void ProcessTriplets();
 
    KLUSystemX();
    KLUSystemX(unsigned int nBus, unsigned int nV = 0, unsigned int nI = 0);
    ~KLUSystemX();

    uint64_t options; // KLUSolveX options, currently limited to values in enum ReuseFlags
    uint32_t dataFormat;
    
    bool bFactored; //  system has been factored
    bool reuseSymbolic; // current state, actual reuse depends on options

    int FactorSystem();
    void SolveSystem(complex* acxX, complex* acxB);
    
    // this resets and reinitializes the sparse matrix, nI = nBus
    int Initialize(unsigned int nBus, unsigned int nV = 0, unsigned int nI = 0);

    uint32_t GetSize() { return m_nBus; }

    // metrics
    uint32_t GetSparseNNZ() 
	{
		return m_NZpost; 
	}
    uint32_t GetNNZ() 
	{ 
		return m_NZpre;
	}
    uint32_t GetSingularCol()
    {
        return m_fltBus;
    }

    double GetRCond();
    double GetRGrowth();
    double GetCondEst();
    double GetFlops();

    // bSum is ignored
    // void AddMatrix(int* aidBus, matrix_complex* pcxm, int bSum);

    // returns 1 for success, -1 for a singular matrix
    // returns 0 for another KLU error, most likely the matrix is too large for int32
    int Factor();

    // input: acxVbus[0] is ground voltage
    //        acxVbus[1..nBus] are current injections
    // output: acxVbus[1..nBus] are solved voltages
    void Solve(complex* acxVbus);

    // returns the number of connected components (cliques) in the whole system graph
    //  (i.e., considers Y11, Y12, and Y21 in addition to Y22)
    // store the island number (1-based) for each node in idClique
    int FindIslands(unsigned int* idClique);

    // returns the row > 0 if a zero appears on the diagonal
    // calls Factor if necessary
    // note: the EMTP terminology is "floating subnetwork"
    int FindDisconnectedSubnetwork();

    // The following were added for ESolv32:
    // maintains allocations, zeros matrix values
    void zero();
    
    void AddElement(unsigned int iRow, unsigned int iCol, complex& cpxVal, int);
    // return the sum of elements at 1-based [iRow, iCol]
    void GetElement(unsigned int iRow, unsigned int iCol, complex& cpxVal);
    // for OpenDSS, return 1 for success
    int AddPrimitiveMatrix(unsigned int nOrder, unsigned int* pNodes, complex* pMat);

    // return in compressed triplet form, return 1 for success, 0 for a size mismatch
    int GetCompressedMatrix(unsigned int nColP, unsigned int nNZ, unsigned int* pColP, unsigned int* pRowIdx, complex* pMat);
    int GetTripletMatrix(unsigned int nNZ, unsigned int* pRows, unsigned int* pCols, complex* pMat);
    
    int IncrementElement(unsigned int iRow, unsigned int iCol, double re, double im);
    int ZeroiseElement(unsigned int iRow, unsigned int iCol);
    int SaveAsMarketFiles(const char* fileNameMatrix, const double *b, const char* fileNameVector);
};

} // namespace KLUSolveX

#endif // #ifndef DSS_EXTENSIONS_KLUSYSTEMX_H
