rmdir /S /Q build64
mkdir build64

cd build64
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_SYSTEM_VERSION=10.0.17763.0 ..
cmake --build . --config Debug -j 4 --parallel 4 -- /p:CharacterSet=Unicode
cd ..
