# How to build

This project uses a CMake script to either build the dependencies in-place, downloading them if required, or using the dependencies available on your system.

The official build uses a shared library (`.dll` on Windows, `.so` on Linux). The static library support is a new adition and still not supported by DSS C-API (work in progress).

Definitions:

- `USE_SYSTEM_SUITESPARSE`: If "OFF" cmake will download the SuiteSparse. If "ON" cmake will use your system's installation if available. Default: "ON".
- `USE_SYSTEM_EIGEN3`: If "OFF" cmake will download the Eigen3. If "ON" cmake will use your system's installation if available. Default: "ON".
- `KLUSOLVE_LIB_TYPE`: If "SHARED" will compile the KLUSolveX as a shared library. If "STATIC" cmake will compile as static library. Default: "SHARED".

## Examples for shell-based enviroments (Linux and MacOs)
### x64 (64-bits)
```shell
#../klusolve
mkdir build
cd build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN3=OFF -DKLUSOLVE_LIB_TYPE=SHARED .. 
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
