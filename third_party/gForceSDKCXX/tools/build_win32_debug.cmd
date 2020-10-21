rmdir /S /Q build32
mkdir build32

cd build32
cmake -G "Visual Studio 15 2017" -DCMAKE_SYSTEM_VERSION=10.0.17763.0 ..
cmake --build . --config Debug -j 4 --parallel 4 -- /p:CharacterSet=Unicode
cd ..
