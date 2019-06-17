rmdir /S /Q build64
mkdir build64

cd build64
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_SYSTEM_VERSION=8.1 -DCMAKE_INSTALL_PREFIX=..\\installed64 ..
cmake --build . --target install --config Release
cd ..
