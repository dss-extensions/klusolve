# if [ -n "$TRAVIS_TAG" ]; then
    # set KLUSOLVE_TAG=$TRAVIS_COMMIT
# else
    # set KLUSOLVE_TAG=$TRAVIS_TAG
# fi

mkdir -p release/klusolvex
cp -R lib release/klusolvex/
cp -R include release/klusolvex/
cp LICENSE release/klusolvex/
cp README.md release/klusolvex/
cd release
tar zcf "klusolvex_${TRAVIS_TAG}_${KLUSOLVE_OS}_${KLUSOLVE_ARCH}.tar.gz" klusolvex
cd ..
rm -rf release/klusolvex
