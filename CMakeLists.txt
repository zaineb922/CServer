set( CMAKE_CXX_COMPILER "/usr/bin/g++" )
set( CMAKE_C_COMPILER "/usr/bin/gcc" )
cmake_minimum_required(VERSION 3.10.0)
project (Project)
add_executable(Project server.c)

# Link with the llhttp_static target
target_link_libraries(Project /usr/lib/llhttp-main/build/libllhttp.so)

