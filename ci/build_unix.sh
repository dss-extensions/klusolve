set -e -x

cd ${KLUSOLVE_WORK_DIR}

export SUITESPARSE_SRC=`readlink -f ./SuiteSparse`
export EIGEN3_SRC=`readlink -f ./eigen-${EIGEN_VERSION}`

# Build KLUSolve
cd ${KLUSOLVE_WORK_DIR}
rm -rf klusolve/build
ls -lR klusolve
mkdir ${KLUSOLVE_WORK_DIR}/klusolve/build
ln -s SuiteSparse klusolve/build/
ln -s eigen-${EIGEN_VERSION} klusolve/build/
cd ${KLUSOLVE_WORK_DIR}/klusolve/build
cmake -DCMAKE_BUILD_TYPE=Release -DDSS_EXTENSIONS=ON -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN=OFF ${KLUSOLVE_EXTRA_CMAKE_FLAGS} ..
cmake --build .
