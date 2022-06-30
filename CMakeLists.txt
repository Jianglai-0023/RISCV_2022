project(RISC_V)

set(CMAKE_CXX_FLAGS O2)
set(CMAKE_CXX_STANDARD 14)

add_executable(code src/main.cpp)
