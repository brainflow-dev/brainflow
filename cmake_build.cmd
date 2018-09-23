if not exist build (
mkdir build
)

cd build
cmake -G "Visual Studio 14 2015 Win64" ..
cmake --build .
cd ..

echo F | xcopy /Y .\compiled\Debug\BoardController.dll .\python-package\brainflow\lib\BoardController.dll

echo F | xcopy /Y .\compiled\Debug\BoardController.dll .\cpp-package\lib\BoardController.dll
echo F | xcopy /Y .\compiled\Debug\BoardController.lib .\cpp-package\lib\BoardController.lib
echo F | xcopy /Y .\src\board_controller\inc\board_controller.h .\cpp-package\inc\board_controller.h

echo F | xcopy /Y .\compiled\Debug\BoardController.dll .\matlab-package\brainflow\lib\BoardController.dll
echo F | xcopy /Y .\compiled\Debug\BoardController.lib .\matlab-package\brainflow\lib\BoardController.lib
echo F | xcopy /Y .\src\board_controller\inc\board_controller.h .\matlab-package\brainflow\inc\board_controller.h
