cmake_minimum_required(VERSION "3.16.3")

# if (NOT DEFINED OX_LIB_MASTER_PROJECT)
#   set(OX_LIB_MASTER_PROJECT OFF)
#   if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
#     set(OX_LIB_MASTER_PROJECT ON)
#     message(STATUS "CMake version: ${CMAKE_VERSION}")
#   endif ()
# endif ()
#
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(DCMAKE_CXX_FLAGS "-Werror -Wall -Wextra")
#
# find_program(
#     CLANG_TIDY_EXE
#     NAMES "clang-tidy"
# )
#
# if(NOT CLANG_TIDY_EXE)
#     message(STATUS "clang-tidy not found.")
# else()
#     message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
#     set(
#         CMAKE_CXX_CLANG_TIDY
#         "${CLANG_TIDY_EXE}"
#         -checks=cppcoreguidelines-*,readability-*,performance-*,clang-analyzer-*,bugprone-*,misc-*,modernize-*,-modernize-use-trailing-return-type,-cppcoreguidelines-avoid-c-arrays,-cppcoreguidelines-avoid-magic-numbers,-cppcoreguidelines-c-copy-assignment-signature,-cppcoreguidelines-explicit-virtual-functions,-cppcoreguidelines-non-private-member-variables-in-classes;
#         -header-filter=src/.*.hpp
#         --warnings-as-errors=cppcoreguidelines-*,readability-*,performance-*,clang-analyzer-*,bugprone-*,misc-*,modernize-*,-modernize-use-trailing-return-type,-cppcoreguidelines-avoid-c-arrays,-cppcoreguidelines-avoid-magic-numbers,-cppcoreguidelines-c-copy-assignment-signature,-cppcoreguidelines-explicit-virtual-functions,-cppcoreguidelines-non-private-member-variables-in-classes;
#         -p .;
#     )
# endif()

function(add_headers VAR)
  set(headers ${${VAR}})
  foreach (header ${ARGN})
    set(headers ${headers} include/ox/${header})
  endforeach()
  set(${VAR} ${headers} PARENT_SCOPE)
endfunction()

function(add_sources VAR)
  set(sources ${${VAR}})
  foreach (source ${ARGN})
    set(sources ${sources} src/${source})
  endforeach()
  set(${VAR} ${sources} PARENT_SCOPE)
endfunction()

add_headers(OX_HEADERS algorithms.h bytes.h common.h formating.h terminal.h types.h)
add_sources(OX_SOURCES terminal.cpp bytes.cpp)

add_library(ox ${OX_SOURCES} ${OX_HEADERS})
add_library(ox::ox ALIAS ox)

set_target_properties(ox PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories("ox" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
