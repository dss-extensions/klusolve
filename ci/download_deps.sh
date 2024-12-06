set -e -x

WGET=wget

if [[ "${KLUSOLVE_OS}" == "windows" ]]; then
    WGET=/c/msys64/usr/bin/wget.exe
fi

${WGET} https://github.com/DrTimothyAldenDavis/SuiteSparse/archive/v5.6.0.tar.gz -O suitesparse.tar.gz -q
tar zxf suitesparse.tar.gz
mv SuiteSparse-5.6.0 SuiteSparse
${WGET} https://gitlab.com/libeigen/eigen/-/archive/${EIGEN_VERSION}/eigen-${EIGEN_VERSION}.tar.gz -O eigen3.tar.gz -q
tar zxf eigen3.tar.gz
