cmake_minimum_required(VERSION 3.31)
project(sub_process)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(fetch_github_content GIT_REPOSITORY https://github.com/CesarBerriot/fetch_github_content.git GIT_TAG 1.0.0)
FetchContent_MakeAvailable(fetch_github_content)
include(${fetch_github_content_SOURCE_DIR}/fetch_github_content.cmake)

block()
  fetch_github_content(
    discover_sources CesarBerriot 1.1.0 INCLUDE_DEFAULT
    string_theory CesarBerriot 3.9
  )
endblock()

add_library(sub_process)
discover_sources(sub_process)
target_include_directories(sub_process PUBLIC include)
target_link_libraries(sub_process PRIVATE string_theory)