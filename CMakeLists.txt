cmake_minimum_required(VERSION 4.4)
project(PurplexityBG)

set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(UNICODE _UNICODE)

add_executable(PurplexityBG WIN32 main.cpp)

file(GLOB_RECURSE QUARK_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/QuarkPhysics/*.cpp"
)
add_library(quarkphysics STATIC ${QUARK_SOURCES})
target_include_directories(quarkphysics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/QuarkPhysics")
target_include_directories(PurplexityBG PRIVATE "C:/Users/docsc/OneDrive/Niko/C++/libraries/SFML-3.1.0/include")
set(SFML_DIR "C:/Users/docsc/OneDrive/Niko/C++/libraries/SFML-3.1.0/lib/cmake/SFML")
find_package(SFML 3 REQUIRED COMPONENTS Graphics)
target_link_libraries(PurplexityBG PRIVATE SFML::Graphics dwmapi quarkphysics)