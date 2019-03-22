rmdir /S /Q build
mkdir build

cd build
cmake -G "Visual Studio 14 2015 Win64" ..
cmake --build .
.\Debug\GetData.exe %1