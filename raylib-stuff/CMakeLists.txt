cmake_minimum_required(VERSION 3.20)
project(mygame LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 23)
set(CMAKE_EXECUTABLE_SUFFIX ".html")   # ← add this

# file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")
add_custom_target(copy_assets ALL
    # 1) be sure the dest dir exists
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/assets

    # 2) copy the changed files only
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${ASSETS} ${CMAKE_CURRENT_BINARY_DIR}/assets

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

# add_executable(mygame game.cpp)
# target_link_libraries(mygame PRIVATE raylib)
# set_target_properties(mygame PROPERTIES LINK_OPTIONS "-sALLOW_MEMORY_GROWTH;-sMAX_WEBGL_VERSION=2;-sASYNCIFY")
# set_property(TARGET mygame APPEND PROPERTY LINK_OPTIONS "--preload-file" "assets")

# add_executable(myearth earth.cpp)
# target_link_libraries(myearth PRIVATE raylib)
# set_target_properties(myearth PROPERTIES LINK_OPTIONS "-sALLOW_MEMORY_GROWTH;-sMAX_WEBGL_VERSION=2;-sASYNCIFY")
# set_property(TARGET myearth APPEND PROPERTY LINK_OPTIONS "--preload-file" "assets")
