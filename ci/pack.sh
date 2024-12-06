if [[ "${GITHUB_REF}" == "refs/tags/"* ]]; then
    export KLUSOLVE_TAG="${GITHUB_REF/refs\/tags\//}"
else
    export KLUSOLVE_TAG="dev"
fi

mkdir -p release/klusolvex
cp -R lib release/klusolvex/
cp -R include release/klusolvex/
cp LICENSE release/klusolvex/
cp README.md release/klusolvex/
cd release
if [[ "${KLUSOLVE_COMPILER}" == "gcc" ]]; then
    gcc --version > klusolvex/gcc_version.txt
    if [[ "${KLUSOLVE_OS}-${KLUSOLVE_ARCH}" == "windows-x64" ]]; then
        cp `which libwinpthread-1.dll` klusolvex/lib/win_${KLUSOLVE_ARCH}/
    fi
fi
# if [[ "${KLUSOLVE_COMPILER}" == "msvc" ]]; then
#     cl | head > klusolvex/cl_version.txt
# fi
if [[ "${KLUSOLVE_OS}" == "windows" ]]; then
    7z a "klusolvex_${KLUSOLVE_TAG}_${KLUSOLVE_OS}_${KLUSOLVE_ARCH}_${KLUSOLVE_COMPILER}_${KLUSOLVE_OS_IMAGE}.zip" klusolvex
else
    tar zcf "klusolvex_${KLUSOLVE_TAG}_${KLUSOLVE_OS}_${KLUSOLVE_ARCH}.tar.gz" klusolvex
fi
cd ..
rm -rf release/klusolvex
