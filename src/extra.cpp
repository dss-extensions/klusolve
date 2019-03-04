#ifdef _MSC_VER
#   if _MSC_VER <= 1500
#       include <stdint_compat.h>
#   else
#       include <cstdint>
#   endif
#else
#    include <cstdint>
#endif

#include <complex>
#include <Eigen/Eigen>

extern "C" void mvmult(int32_t N, void *b_, void *A_, void *x_)
{
    using std::complex;
    typedef complex<double> Complex;
    using namespace Eigen;
    
    Complex *A = (Complex *)A_;
    Complex *x = (Complex *)x_;
    Complex *b = (Complex *)b_;
    
    switch(N)
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
    default:
        Map<VectorXcd>(b, N).noalias() = Map<MatrixXcd>(A, N, N) * Map<VectorXcd>(x, N);
        break;
    }
}
