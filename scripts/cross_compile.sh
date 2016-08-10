cd ..
rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../scripts/mingw_toolchain.cmake -DDHEWM3LIBS=../scripts/dhewm3-libs/i686-w64-mingw32 ../neo
