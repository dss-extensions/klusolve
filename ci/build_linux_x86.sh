set -e -x

cd /io

export SUITESPARSE_SRC=`readlink -f ./SuiteSparse`
export EIGEN3_SRC=`readlink -f ./eigen-eigen-323c052e1731`

# Build KLUSolve
cd /io
rm -rf klusolve/build
ls -lR klusolve
mkdir /io/klusolve/build
ln -s SuiteSparse klusolve/build/
ln -s eigen-eigen-323c052e1731 klusolve/build/
cd /io/klusolve/build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN3=OFF -DCMAKE_CXX_COMPILER_ARG1=-m32 -DCMAKE_C_COMPILER_ARG1=-m32 ..
cmake --build . --config Release
