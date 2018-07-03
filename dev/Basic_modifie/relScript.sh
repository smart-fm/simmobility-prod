rm -rf Release
mkdir Release
cd Release
cmake ../ -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_SHORT:BOOL=OFF -DBUILD_MEDIUM:BOOL=ON -DBUILD_LONG:BOOL=OFF
make -j30
cd ..


