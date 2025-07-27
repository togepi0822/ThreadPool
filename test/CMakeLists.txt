cmake_minimum_required(VERSION 3.12)
project(ThreadPool_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")

set(ENV{http_proxy} "http://127.0.0.1:10808")
set(ENV{https_proxy} "http://127.0.0.1:10808")
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest.git
        GIT_TAG         52eb8108c5bdec04579160ae17225d66034bd723
        GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE test_src_list
        "src/*.cpp"
)

add_executable(ThreadPool_test ${test_src_list})
target_link_libraries(ThreadPool_test
        PRIVATE
        ThreadPool
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ThreadPool_test)
target_compile_options(ThreadPool_test PRIVATE -Wall -Wextra -Werror)