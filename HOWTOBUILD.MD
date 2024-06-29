# How to build

This project uses a CMake script to either build the dependencies in-place, downloading them if required, or using the dependencies available on your system.

The official build uses a shared library (`.dll` on Windows, `.so` on Linux). The static library support is a new addition and still not supported by DSS C-API (work in progress, expected to be used in Jan 2025).

Definitions:

- `USE_SYSTEM_SUITESPARSE`: If "OFF" cmake will download the SuiteSparse. If "ON" cmake will use your system's installation if available. Default: "ON".
- `USE_SYSTEM_EIGEN`: If "OFF" cmake will download the Eigen3. If "ON" cmake will use your system's installation if available. Default: "ON".
- `BUILD_SHARED_LIBS`: When "ON", building a shared library (.dll/.so/.dylib). Set to "OFF" to produce a static library instead. Previously, `KLUSOLVE_LIB_TYPE` was used and it's been replaced to provide easier integration with other tools and libraries. Default: "ON".
- `DSS_EXTENSIONS`: If "ON", tweaks the artifact output folders for the release process. Default: "OFF".

## Examples for shell-based environments (Linux and macOS)
### x64 (64-bits)
```shell
#../klusolve
mkdir build
cd build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN=OFF .. 
cmake --build . --config Release
```

### x84 (32-bits)
```shell
#../klusolve
mkdir build
cd build
cmake -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN=OFF -DCMAKE_CXX_COMPILER_ARG1=-m32 -DCMAKE_C_COMPILER_ARG1=-m32 ..
cmake --build . --config Release
```
