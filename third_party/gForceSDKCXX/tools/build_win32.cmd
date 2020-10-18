rmdir /S /Q build32
mkdir build32

cd build32
cmake -G "Visual Studio 15 2017" -DCMAKE_SYSTEM_VERSION=8.1 ..
cmake --build . --config Release -- /p:CharacterSet=Unicode
cd ..
