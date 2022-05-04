# How to build

For static build, export `KLU_AS_STATIC`. e.g.:
```
export KLU_AS_STATIC=TRUE
```

## In shell-based enviroments (Linux and MacOs)
### x64 (64-bits)
```shell
#../klusolve
mkdir build
cd build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN3=OFF ..
cmake --build . --config Release
```

### x84 (32-bits)
```shell
#../klusolve
mkdir build
cd build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN3=OFF -DCMAKE_CXX_COMPILER_ARG1=-m32 -DCMAKE_C_COMPILER_ARG1=-m32 ..
cmake --build . --config Release
```