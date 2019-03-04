set KLUSOLVE_TAG=%APPVEYOR_REPO_COMMIT%

REM Prepare the environment
ren c:\cygwin cygwin.disabled
ren c:\cygwin64 cygwin64.disabled
ren "C:\Program Files\Git" Git.disabled

if exist c:\mingw ren c:\mingw mingw.disabled 
if "%KLUSOLVE_USE_MINGW%"=="1" mklink /D c:\mingw C:\Qt\Tools\mingw530_32
if "%KLUSOLVE_USE_MINGW_W64%"=="1" mklink /D c:\mingw C:\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64

REM Run CMake to build it
mkdir c:\projects\klusolve\build
cd c:\projects\klusolve\build

set path=%ADD_THIS_TO_PATH%;%path%
cmake .. -DUSE_SYSTEM_SUITESPARSE=OFF -DUSE_SYSTEM_EIGEN3=OFF -G"%CMAKE_GENERATOR%" %CMAKE_EXTRA%
cmake --build . --config Release

REM Copy dependency DLLs
cd c:\projects\klusolve
if exist C:\mingw\bin\libwinpthread-1.dll copy C:\mingw\bin\libwinpthread-1.dll c:\projects\klusolve\lib\win_%KLUSOLVE_ARCH%\

REM Just to check in the logs, list the contents
dir /s lib

REM Prepare the release package
IF DEFINED APPVEYOR_REPO_TAG_NAME (
    set KLUSOLVE_TAG=%APPVEYOR_REPO_TAG_NAME%
)
mkdir release
mkdir release\klusolve
xcopy /E lib release\klusolve\lib\
xcopy /E include release\klusolve\include\
copy LICENSE release\klusolve\
copy README.md release\klusolve\
cd release
7z a "klusolve_%KLUSOLVE_TAG%_win_%KLUSOLVE_ARCH%-%KLUSOLVE_COMPILER%.zip" klusolve
cd ..
rd /s /q release\klusolve

appveyor PushArtifact "c:\projects\klusolve\release\klusolve_%KLUSOLVE_TAG%_win_%KLUSOLVE_ARCH%-%KLUSOLVE_COMPILER%.zip"
