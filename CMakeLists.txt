cmake_minimum_required(VERSION 3.21)
project(umba CXX)

# Руководство по CMake для разработчиков C++ библиотек - https://habr.com/ru/articles/683204/
# https://github.com/pananton/cpp-lib-template/blob/main/CMakeLists.txt

# Сохранение древовидной структуры в Visual Studio с CMake или создание папок в проекте - https://habr.com/ru/articles/754272/

# string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)
# use PROJECT_IS_TOP_LEVEL instead (requires CMake v3.21)

# Modules - https://cmake.org/cmake/help/book/mastering-cmake/chapter/Modules.html
# Finding Packages - https://cmake.org/cmake/help/book/mastering-cmake/chapter/Finding%20Packages.html
# How to structure your project - https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html

# CMake 3 Tools - https://github.com/CLIUtils/cmake
# Modern CMake (заменяет CMake 3 Tools) - https://github.com/CLIUtils/modern_cmake
# The Ultimate Guide to Modern CMake - https://rix0r.nl/blog/2015/08/13/cmake-guide/

# Введение в CMake - https://habr.com/ru/articles/155467/
# Руководство по CMake для разработчиков C++ библиотек - https://habr.com/ru/articles/683204/
#   cpp-lib-template - https://github.com/pananton/cpp-lib-template

# cmake-init - The missing CMake project initializer - https://github.com/friendlyanon/cmake-init


if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


### End of common part


set(MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}")

file(GLOB_RECURSE sources "${MODULE_ROOT}/*.cpp")
list(FILTER sources EXCLUDE REGEX ".*CMakeCXXCompilerId\\.cpp$")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${sources})

file(GLOB_RECURSE headers "${MODULE_ROOT}/*.h")
list(FILTER headers EXCLUDE REGEX ".*zz_.*\\.h$")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${headers})


add_library(${PROJECT_NAME} ${sources} ${headers})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# target_include_directories(${PROJECT_NAME} PRIVATE ${MODULE_ROOT}/..)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE "ws2_32" "iphlpapi" "Shell32")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
