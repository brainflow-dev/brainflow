rmdir /S /Q build64
mkdir build64

cd build64
cmake -DUSE_OPENMP=ON -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=8.1 -DCMAKE_INSTALL_PREFIX=..\\installed64 ..
cmake --build . --target install --config Release -j 4 --parallel 4
cd ..
