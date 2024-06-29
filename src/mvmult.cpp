/* ------------------------------------------------------------------------- */
/* DSS-Extensions KLUSolve (KLUSolveX)                                       */
/* Copyright (c) 2019-2024, Paulo Meira                                      */
/* Licensed under the GNU Lesser General Public License (LGPL) v 2.1         */
/* ------------------------------------------------------------------------- */

#ifdef _MSC_VER
#if _MSC_VER <= 1500
#error "This compiler version is not supported anymore."
#endif
#endif
#include <cstdint>

#include <Eigen/Eigen>
#include <complex>

extern "C" void mvmult(int32_t N, double* b_, double* A_, double* x_)
{
    typedef std::complex<double> Complex;
    using namespace Eigen;
    typedef Matrix<Complex, 6, 1> Vector6cd;
    typedef Matrix<Complex, 6, 6> Matrix6cd;
    typedef Matrix<Complex, 8, 1> Vector8cd;
    typedef Matrix<Complex, 8, 8> Matrix8cd;

    Complex* A = (Complex*)A_;
    Complex* x = (Complex*)x_;
    Complex* b = (Complex*)b_;

    switch (N)
    {
    case 1:
        b[0] = A[0] * x[0];
        break;
    case 2:
        Map<Vector2cd>(b).noalias() = Map<Matrix2cd>(A) * Map<Vector2cd>(x);
        break;
    case 3:
        Map<Vector3cd>(b).noalias() = Map<Matrix3cd>(A) * Map<Vector3cd>(x);
        break;
    case 4:
        Map<Vector4cd>(b).noalias() = Map<Matrix4cd>(A) * Map<Vector4cd>(x);
        break;
    case 6:
        Map<Vector6cd>(b).noalias() = Map<Matrix6cd>(A) * Map<Vector6cd>(x);
        break;
    case 8:
        Map<Vector8cd>(b).noalias() = Map<Matrix8cd>(A) * Map<Vector8cd>(x);
        break;
    default:
        Map<VectorXcd>(b, N).noalias() = Map<MatrixXcd>(A, N, N) * Map<VectorXcd>(x, N);
        break;
    }
}
