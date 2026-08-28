cmake_minimum_required(VERSION 3.20)

project(PhotoshopAPI.C LANGUAGES C CXX)

add_library(PhotoshopAPI.C SHARED
    src/PhotoshopAPI.C.cpp
    src/PhotoshopAPI.C.V2.cpp)

target_compile_features(PhotoshopAPI.C PRIVATE cxx_std_20)
target_compile_definitions(PhotoshopAPI.C PRIVATE PHOTOSHOPAPI_C_EXPORTS)
target_include_directories(PhotoshopAPI.C
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(PhotoshopAPI.C PRIVATE PhotoshopAPI)

if(MSVC)
    target_compile_options(PhotoshopAPI.C PRIVATE /W4 /utf-8)
else()
    target_compile_options(PhotoshopAPI.C PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(PhotoshopAPI.C.AbiSmoke
    tests/PhotoshopAPI.C.AbiSmoke.c)
target_link_libraries(PhotoshopAPI.C.AbiSmoke PRIVATE PhotoshopAPI.C)
target_include_directories(PhotoshopAPI.C.AbiSmoke PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include")

add_executable(PhotoshopAPI.C.FixtureGenerator
    tests/PhotoshopAPI.C.FixtureGenerator.cpp)
target_compile_features(PhotoshopAPI.C.FixtureGenerator PRIVATE cxx_std_20)
target_link_libraries(PhotoshopAPI.C.FixtureGenerator PRIVATE PhotoshopAPI.C)
target_include_directories(PhotoshopAPI.C.FixtureGenerator PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include")

if(MSVC)
    target_compile_options(PhotoshopAPI.C.FixtureGenerator PRIVATE /W4 /WX /utf-8)
else()
    target_compile_options(PhotoshopAPI.C.FixtureGenerator PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
