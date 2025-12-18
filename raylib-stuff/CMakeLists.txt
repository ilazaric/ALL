cmake_minimum_required(VERSION 3.20)
project(mygame LANGUAGES CXX)

include_directories(/home/ilazaric/repos/ALL/build/include)
include_directories(/home/ilazaric/repos/ALL/build/default_include)
# include_directories(/usr/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

set (CMAKE_CXX_STANDARD 23)
set(CMAKE_EXECUTABLE_SUFFIX ".html")   # ← add this

file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
    DEPENDS ${ASSETS}                   # re‑run if any source asset changes
    COMMENT "Copying game assets → build/assets"
)  

add_subdirectory(/home/ilazaric/repos/raylib build-raylib)

function(add_exe NAME)
  add_executable(${NAME} ${NAME}.cpp)
  target_link_libraries(${NAME} PRIVATE raylib)
  set_target_properties(${NAME} PROPERTIES LINK_OPTIONS "-sALLOW_MEMORY_GROWTH;-sMAX_WEBGL_VERSION=2;-sMIN_WEBGL_VERSION=2;-sFULL_ES3=1;-sASYNCIFY;-sASSERTIONS")
  set_property(TARGET ${NAME} APPEND PROPERTY LINK_OPTIONS "--preload-file" "assets")
  add_dependencies(${NAME} copy_assets)
endfunction()

add_exe(mygame)
add_exe(myearth)
add_exe(gameoflife)
add_exe(rts)
add_exe(blatruc)
add_exe(factorio)
