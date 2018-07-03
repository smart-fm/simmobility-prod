rm -rf Debug
mkdir Debug
cd Debug
cmake ../ -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_SHORT:BOOL=OFF -DBUILD_MEDIUM:BOOL=ON -DBUILD_LONG:BOOL=OFF
make -j21
cd ..


