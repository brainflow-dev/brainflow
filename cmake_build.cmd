if not exist build (
mkdir build
)

cd build
cmake -G "Visual Studio 14 2015 Win64" ..
cmake --build .
cd ..

echo F | xcopy /Y .\compiled\Debug\BoardController.dll .\python\brainflow\lib\BoardController.dll
