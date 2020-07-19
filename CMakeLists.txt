cmake_minimum_required(VERSION 3.7)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
project(SDL2Test VERSION 0.9 LANGUAGES CXX)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS_FILES ${CMAKE_SOURCE_DIR}/include/*.h)

configure_file(${CMAKE_SOURCE_DIR}/includeGen/Utils.h.in ${CMAKE_SOURCE_DIR}/include/Utils.h)
find_package(SDL2 REQUIRED)
find_package(GLM REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${PROJECT_NAME} ${GLM_INCLUDE_DIR})
include_directories(
	"${CMAKE_SOURCE_DIR}/include"
)
add_executable(${PROJECT_NAME} ${HEADERS_FILES} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})