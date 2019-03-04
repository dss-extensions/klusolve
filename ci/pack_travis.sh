# if [ -n "$TRAVIS_TAG" ]; then
    # set KLUSOLVE_TAG=$TRAVIS_COMMIT
# else
    # set KLUSOLVE_TAG=$TRAVIS_TAG
# fi

mkdir -p release/klusolve/lib
cp -R lib/ release/klusolve/lib/
cp -R include release/klusolve/
cp LICENSE release/klusolve/
cp README.md release/klusolve/
cd release
tar zcf "klusolve_${TRAVIS_TAG}_${KLUSOLVE_OS}_${KLUSOLVE_ARCH}.tar.gz" klusolve
cd ..
rm -rf release/klusolve
