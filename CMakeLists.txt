cmake_minimum_required(VERSION "3.16.3")

INCLUDE(FindPkgConfig)

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

find_package(Threads REQUIRED)

find_package(SDL2 REQUIRED)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf)

message(STATUS "SDL2_FOUND = ${SDL2_FOUND}")
message(STATUS "SDL2_INCLUDE_DIR = ${SDL2_INCLUDE_DIR}")
message(STATUS "SDL2_LIBRARIES = ${SDL2_LIBRARIES}")
message(STATUS "SDL2IMAGE_LIBRARIES = ${SDL2IMAGE_LIBRARIES}")
message(STATUS "SDL2TTF_LIBRARIES = ${SDL2TTF_LIBRARIES}")

add_headers(OX_HEADERS algorithms.h bytes.h common.h formatting.h terminal.h types.h
        file.h ptr.h triple.h canvas.h colors.h)
add_sources(OX_SOURCES terminal.cpp bytes.cpp canvas.cpp file.cpp)

add_library(ox ${OX_SOURCES} ${OX_HEADERS})
add_library(ox::ox ALIAS ox)

set_target_properties(ox PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(ox PRIVATE ${SDL2_LIBRARIES})
target_link_libraries(ox PRIVATE ${SDL2IMAGE_LIBRARIES})
target_link_libraries(ox PRIVATE ${SDL2TTF_LIBRARIES})
target_link_libraries(ox PRIVATE Threads::Threads)
target_include_directories("ox" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
