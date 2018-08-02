cmake_minimum_required (VERSION 3.10)
project (brainflow)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_VERBOSE_MAKEFILE ON)

include_directories (
    inc
    src/Utils/
    src/BoardController/inc
)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/brainflow/lib)
set (LIBRARY_SRC
	"src/Utils/TimeStamp.cpp"
	"src/Utils/DataBuffer.cpp"
	"src/BoardController/Board.cpp"
	"src/BoardController/Cython.cpp"
	"src/BoardController/BoardController.cpp"
	)

add_library (
    BoardController SHARED
    ${LIBRARY_SRC}
)

if (${EMULATOR})
	target_compile_definitions (BoardController PUBLIC EMULATOR_MODE)
endif (${EMULATOR})