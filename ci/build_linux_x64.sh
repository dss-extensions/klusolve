set -e -x

cd /io

export SUITESPARSE_SRC=`readlink -f ./SuiteSparse`
export EIGEN3_SRC=`readlink -f ./eigen-${EIGEN_VERSION}`

# Build KLUSolve
cd /io
rm -rf klusolve/build
ls -lR klusolve
mkdir /io/klusolve/build
ln -s SuiteSparse klusolve/build/
ln -s eigen-${EIGEN_VERSION} klusolve/build/
cd /io/klusolve/build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN3=OFF ..
cmake --build . --config Release
