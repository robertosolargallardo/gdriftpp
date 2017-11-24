cmake_minimum_required(VERSION 2.8.9)
project(gdrift_services)
set(CMAKE_BUILD_TYPE Release)
find_package(Boost 1.54.0 REQUIRED)
find_package(Armadillo 4.200.0 REQUIRED)

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

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/include/*.cc")
link_directories(${GDRIFT_LIBRARY} ${MONGOCXX_LIBRARY} ${BSONCXX_LIBRARY} ${RESTBED_LIBRARY})
include_directories(${CMAKE_SOURCE_DIR}/include ${MONGOCXX_INCLUDE} ${RESTBED_INCLUDE} ${BSONCXX_INCLUDE} ${GDRIFT_INCLUDE})

file(GLOB analyzer_SOURCES "${CMAKE_SOURCE_DIR}/analyzer/src/*.cc" ${SOURCES})
include_directories(gdrift_service_analyzer ${CMAKE_SOURCE_DIR}/analyzer/include)
add_executable(gdrift_service_analyzer ${analyzer_SOURCES})
target_link_libraries(gdrift_service_analyzer mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl armadillo)

file(GLOB controller_SOURCES "${CMAKE_SOURCE_DIR}/controller/src/*.cc" ${SOURCES})
include_directories(gdrift_service_controller ${CMAKE_SOURCE_DIR}/controller/include)
add_executable(gdrift_service_controller ${controller_SOURCES})
target_link_libraries(gdrift_service_controller mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

file(GLOB scheduler_SOURCES "${CMAKE_SOURCE_DIR}/scheduler/src/*.cc" ${SOURCES})
include_directories(gdrift_service_scheduler ${CMAKE_SOURCE_DIR}/scheduler/include)
add_executable(gdrift_service_scheduler ${scheduler_SOURCES})
target_link_libraries(gdrift_service_scheduler mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

file(GLOB results_SOURCES "${CMAKE_SOURCE_DIR}/results/src/*.cc" ${SOURCES})
include_directories(gdrift_service_results ${CMAKE_SOURCE_DIR}/results/include)
add_executable(gdrift_service_results ${results_SOURCES})
target_link_libraries(gdrift_service_results mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl armadillo)

add_executable(extract_data  "${CMAKE_SOURCE_DIR}/test/statistics/extract_data.cc")
target_link_libraries(extract_data mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

add_executable(prepare_data  "${CMAKE_SOURCE_DIR}/test/statistics/prepare_data.cc")
target_link_libraries(prepare_data mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

add_executable(prepare_data_simple  "${CMAKE_SOURCE_DIR}/test/statistics/prepare_data_simple.cc")
target_link_libraries(prepare_data_simple mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

add_executable(extract_times  "${CMAKE_SOURCE_DIR}/test/statistics/extract_times.cc")
target_link_libraries(extract_times mongocxx bsoncxx restbed gdrift++ ssl crypto pthread boost_thread boost_system curl)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "default install path" FORCE )
endif()

install(TARGETS gdrift_service_scheduler gdrift_service_controller gdrift_service_results gdrift_service_analyzer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
