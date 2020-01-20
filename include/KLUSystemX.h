/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2020, Paulo Meira                                      */
/* Based on KLUSolve, Copyright (c) 2008, EnerNex Corporation                */
/* All rights reserved.                                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#ifndef dssx_klusystem_included
#define dssx_klusystem_included

#include <Eigen/SparseCore>

#include "klu.h"
#include "KLUSolveX.h"

typedef std::complex<double> complex;

struct matrix_complex
{
    complex* acx;
    int nRow, nCol;
    complex get_acx(int i, int j)
    {
        return acx[i * nCol + j];
    }
};

/* Kron reduction not supported, this version just solves

|Y22| * |V| = |I|

KLU manages complex values as interleaved real/imag in double arrays
KLU arrays are zero-based
*/

class KLUSystem
{
public:
    typedef Eigen::SparseMatrix<complex> SparseMatrix;

private:
    // admittance matrix blocks in compressed-column storage, like Matlab
    SparseMatrix spmat;
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
 

public:
    KLUSystem();
    KLUSystem(int nBus, int nV = 0, int nI = 0);
    ~KLUSystem();

    uint64_t options; // KLUSolveX options, currently limited to values in enum ReuseFlags
    
    bool bFactored; //  system has been factored
    bool reuseSymbolic; // current state, actual reuse depends on options

    int FactorSystem();
    void SolveSystem(complex* acxX, complex* acxB);
    
    // this resets and reinitializes the sparse matrix, nI = nBus
    int Initialize(int nBus, int nV = 0, int nI = 0);

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
    int FindIslands(int* idClique);

    // returns the row > 0 if a zero appears on the diagonal
    // calls Factor if necessary
    // note: the EMTP terminology is "floating subnetwork"
    int FindDisconnectedSubnetwork();

    // The following were added for ESolv32:
    // maintains allocations, zeros matrix values
    void zero();
    
    void AddElement(int iRow, int iCol, complex& cpxVal, int);
    // return the sum of elements at 1-based [iRow, iCol]
    void GetElement(int iRow, int iCol, complex& cpxVal);
    // for OpenDSS, return 1 for success
    int AddPrimitiveMatrix(int nOrder, int* pNodes, complex* pMat);

    // return in compressed triplet form, return 1 for success, 0 for a size mismatch
    int GetCompressedMatrix(int nColP, int nNZ, int* pColP, int* pRowIdx, complex* pMat);
    int GetTripletMatrix(int nNZ, int* pRows, int* pCols, complex* pMat);
    
    int IncrementElement(int iRow, int iCol, double re, double im);
    int ZeroiseElement(int iRow, int iCol);
};

#endif // dssx_klusystem_included
