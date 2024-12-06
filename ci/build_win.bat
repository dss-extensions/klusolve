set KLUSOLVE_TAG=%GITHUB_REF_NAME%

@REM We don't use Perl at all, much less need the weird toolset from Strawberry
ren C:\Strawberry Strawberry.disabled

if "%KLUSOLVE_COMPILER%"=="gcc" (
    set CMAKE_EXTRA=-DCMAKE_BUILD_TYPE=Release
    if "%KLUSOLVE_ARCH%"=="x86" (
        set CMAKE_GENERATOR=MinGW Makefiles
        set CXX=i686-w64-mingw32-g++
        set CC=i686-w64-mingw32-gcc
        set CMAKE_EXTRA=%CMAKE_EXTRA% -DCMAKE_CXX_COMPILER_ARG1=-m32 -DCMAKE_C_COMPILER_ARG1=-m32
    ) else (
        set CMAKE_GENERATOR=MSYS Makefiles
        set CXX=g++
        set CC=gcc
    )
)

if "%KLUSOLVE_COMPILER%"=="msvc" (
    if "%KLUSOLVE_OS_IMAGE%"=="x86" (
        set CMAKE_EXTRA=-A Win32
    )
    if "%KLUSOLVE_OS_IMAGE%"=="windows-2019" (
        set CMAKE_GENERATOR=Visual Studio 16 2019
    ) else (
        set CMAKE_GENERATOR=Visual Studio 17 2022
    )
) 

echo CMAKE_GENERATOR=%CMAKE_GENERATOR%

@REM if "%KLUSOLVE_USE_MINGW%"=="1" mklink /D c:\mingw C:\Qt\Tools\mingw530_32

@REM Prepare the environment
@REM ren c:\cygwin cygwin.disabled
@REM ren c:\cygwin64 cygwin64.disabled
@REM del /s "c:\Program Files\git\usr\bin\bash.exe"
@REM del /s "c:\Program Files\git\usr\bin\sh.exe"

@REM if exist c:\mingw ren c:\mingw mingw.disabled 
@REM if "%KLUSOLVE_USE_MINGW%"=="1" mklink /D c:\mingw C:\Qt\Tools\mingw530_32
@REM if "%KLUSOLVE_USE_MINGW_W64%"=="1" mklink /D c:\mingw C:\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64

REM Run CMake to build it
mkdir build
cd build

set path=%ADD_THIS_TO_PATH%;%path%
echo FULL CMAKE COMMAND LINE: cmake .. -DDSS_EXTENSIONS=ON -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN=OFF %CMAKE_EXTRA%
cmake .. -DDSS_EXTENSIONS=ON -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN=OFF %CMAKE_EXTRA%
cmake --build . --config Release

@REM REM Copy dependency DLLs
@REM cd c:\projects\klusolve
@REM if exist C:\mingw\bin\libwinpthread-1.dll copy C:\mingw\bin\libwinpthread-1.dll c:\projects\klusolve\lib\win_%KLUSOLVE_ARCH%\

REM Just to check in the logs, list the contents
dir /s lib
