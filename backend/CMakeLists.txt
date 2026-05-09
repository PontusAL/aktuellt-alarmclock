cmake_minimum_required(VERSION 3.16)

project(svt_aktuellt_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG master
)

FetchContent_MakeAvailable(Crow)

add_executable(svt_aktuellt_backend
    src/main.cpp
)

target_link_libraries(svt_aktuellt_backend
    PRIVATE Crow::Crow
)