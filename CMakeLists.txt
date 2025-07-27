cmake_minimum_required(VERSION 3.21)
project(THREADPOOL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")


add_library(ThreadPool INTERFACE)
target_include_directories(ThreadPool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)


option(TP_BUILD_TESTS "Build unit test" ${PROJECT_IS_TOP_LEVEL})
if (TP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif (TP_BUILD_TESTS)


add_executable(app main.cpp)
target_link_libraries(app PRIVATE ThreadPool)