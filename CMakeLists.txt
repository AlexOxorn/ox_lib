cmake_minimum_required(VERSION "3.16.3")

INCLUDE(FindPkgConfig)
project(oxlib)
set(CMAKE_CXX_STANDARD 20)

if (NOT DEFINED OXLIB_MASTER_PROJECT)
  set(OXLIB_MASTER_PROJECT OFF)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(OXLIB_MASTER_PROJECT ON)
    message(STATUS "CMake version: ${CMAKE_VERSION}")
  endif ()
endif ()

if(OXLIB_MASTER_PROJECT EQUAL "ON")
  set(OXLIB_CANVAS TRUE)
endif()

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

find_package(Threads)

if (DEFINED OXLIB_CANVAS)
  find_package(SDL2)
  PKG_SEARCH_MODULE(SDL2IMAGE SDL2_image)
  PKG_SEARCH_MODULE(SDL2TTF SDL2_ttf)

  message(STATUS "SDL2_FOUND = ${SDL2_FOUND}")
  message(STATUS "SDL2_INCLUDE_DIR = ${SDL2_INCLUDE_DIR}")
  message(STATUS "SDL2_LIBRARIES = ${SDL2_LIBRARIES}")
  message(STATUS "SDL2IMAGE_LIBRARIES = ${SDL2IMAGE_LIBRARIES}")
  message(STATUS "SDL2TTF_LIBRARIES = ${SDL2TTF_LIBRARIES}")

  if(SDL2_FOUND)
    add_headers(OX_HEADERS canvas.h)
  endif()
endif()



add_headers(
        OX_HEADERS
        algorithms.h
        bytes.h
        formatting.h
        terminal.h
        types.h
        io.h
        colors.h
        array.h
        math.h
        grid.h
        ranges.h
        graph.h
        std_abbreviation.h
        tree.h
        matrix.h
        hash.h
        threading.h
        parser.h
        infinite_grid.h
        polygon.h
        combinators.h
        future/vector_format.h
        future/generator.h
)
add_sources(OX_SOURCES terminal.cpp bytes.cpp canvas.cpp file.cpp debug.cpp)

add_library(ox ${OX_SOURCES} ${OX_HEADERS})
add_library(ox::ox ALIAS ox)

set_target_properties(ox PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(ox PRIVATE ${SDL2_LIBRARIES})
target_link_libraries(ox PRIVATE ${SDL2IMAGE_LIBRARIES})
target_link_libraries(ox PRIVATE ${SDL2TTF_LIBRARIES})
target_link_libraries(ox PRIVATE Threads::Threads)
target_include_directories("ox" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
