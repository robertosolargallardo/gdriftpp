cmake_minimum_required(VERSION 2.8.9)
project(gdrift_services)
set(CMAKE_BUILD_TYPE Release)
find_package(Boost 1.55.0 REQUIRED)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -O3")
else()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/modules)

add_definitions(-DGDRIFT_ROOT_DIR)
add_definitions(-DRESTBED_ROOT_DIR)

find_package(Gdrift REQUIRED)
if(NOT GDRIFT_FOUND)
   message(FATAL_ERROR "Package libgdrift not found")
endif(NOT GDRIFT_FOUND)

find_package(Restbed REQUIRED)
if(NOT RESTBED_FOUND)
   message(FATAL_ERROR "Package restbed not found")
endif(NOT RESTBED_FOUND)

find_package(BsonCXX REQUIRED)
if(NOT BSONCXX_FOUND)
   message(FATAL_ERROR "Package bsoncxx not found")
endif(NOT BSONCXX_FOUND)

find_package(MongoCXX REQUIRED)
if(NOT MONGOCXX_FOUND)
   message(FATAL_ERROR "Package mongocxx not found")
endif(NOT MONGOCXX_FOUND)

link_directories(/usr/local/lib /usr/local/library)

include_directories(${MONGOCXX_INCLUDE_DIR} ${RESTBED_INCLUDE_DIR} ${BSONCXX_INCLUDE_DIR} ${GDRIFT_INCLUDE_DIR})

set(analyzer_SOURCES "${CMAKE_SOURCE_DIR}/analyzer/src/Analyzer.cc" "${CMAKE_SOURCE_DIR}/analyzer/src/analyzer.cc")
include_directories(analyzer ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/analyzer/include)
add_executable(analyzer ${analyzer_SOURCES})
target_link_libraries(analyzer mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

message("${MONGOCXX_LIBRARIES} ${RESTBED_LIBRARIES} ${BSONCXX_LIBRARIES} ${GDRIFT_LIBRARIES}")
message("${MONGOCXX_INCLUDE_DIR} ${RESTBED_INCLUDE_DIR} ${BSONCXX_INCLUDE_DIR} ${GDRIFT_INCLUDE_DIR}")

#add_definitions(-DRESTBED_PATH)
#if(NOT DEFINED RESTBED_PATH)
#   set(RESTBED_PATH "/usr/local/")
#endif()

#add_definitions(-DGDRIFT_PATH)
#if(NOT DEFINED GDRIFT_PATH)
#   set(GDRIFT_PATH "/usr/local/")
#endif()

#add_definitions(-DBSONCXX_PATH)
#if(NOT DEFINED BSONCXX_PATH)
#   set(BSONCXX_PATH "/usr/local/")
#endif()

#add_definitions(-DMONGOCXX_PATH)
#if(NOT DEFINED MONGOCXX_PATH)
#   set(MONGOCXX_PATH "/usr/local/")
#endif()


#file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cc")

#add_executable(test_sim "${CMAKE_SOURCE_DIR}/test/sim/test.cc")
#add_executable(test_load "${CMAKE_SOURCE_DIR}/test/load/test.cc")
#add_executable(test_parallel "${CMAKE_SOURCE_DIR}/test/parallel/test.cc")
#link_directories(${CMAKE_BINARY_DIR})
#target_link_libraries(test_sim gdrift++)
#target_link_libraries(test_load gdrift++)
#target_link_libraries(test_parallel gdrift++ pthread)

#install(TARGETS gdrift++ DESTINATION /usr/local/lib)
#install(TARGETS test_sim test_load test_parallel DESTINATION /usr/local/bin)
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION /usr/local/include/gdrift++ FILES_MATCHING PATTERN "*.h")

