cmake_minimum_required(VERSION 3.16)

project(Peabrain CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Generate compile_commands.json so CLion/clangd can index correctly
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(COMPILE_FLAGS --cache ${CMAKE_BINARY_DIR}/cache)
set(LINK_FLAGS -sASSERTIONS=0 -sMALLOC=emmalloc -sWASM_ASYNC_COMPILATION=0)
add_compile_options(${COMPILE_FLAGS})
add_link_options(${COMPILE_FLAGS} ${LINK_FLAGS})

add_subdirectory(screepsxx)

# If you change TARGET_NAME, please, make corresponding changes in main.js.
set(TARGET_NAME Peabrain)

include_directories(${CMAKE_SOURCE_DIR}/Peabrain/include)

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/Peabrain/src/*.cpp ${CMAKE_SOURCE_DIR}/Peabrain/src/*/*.cpp)

add_executable(${TARGET_NAME} ${SRC_FILES})
target_link_libraries(${TARGET_NAME} screepsxx)
target_link_options(${TARGET_NAME} PUBLIC -sMODULARIZE=1 --no-entry --bind -sEXPORT_ES6=0)

# Collect all artifacts in 'dist' directory
# WASM-module and corresponding JS-module must have different base names in order to use them in Screeps, so we add suffixes.
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${TARGET_NAME}.wasm ${CMAKE_SOURCE_DIR}/dist/${TARGET_NAME}_module.wasm)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${TARGET_NAME}.js ${CMAKE_SOURCE_DIR}/dist/${TARGET_NAME}_loader.js)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/js/main.js ${CMAKE_SOURCE_DIR}/dist/main.js)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/js/wasm_loader.js ${CMAKE_SOURCE_DIR}/dist/wasm_loader.js)

# Following post-build step will automatically upload artifacts to
# official Screeps server.
# set SCREEPS_TOKEN environment variable to your Screeps API token
# (https://docs.screeps.com/auth-tokens.html).
find_package(Python COMPONENTS Interpreter REQUIRED)

set(SCREEPS_BRANCH "default" CACHE STRING "Screeps branch to deploy to")
set(SCREEPS_SCRIPTS_DIR "C:/Users/warre/AppData/Local/Screeps/scripts/127_0_0_1___21025/${SCREEPS_BRANCH}")

# here you could chose to deploy to a local server or to the official screeps world.
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dist ${SCREEPS_SCRIPTS_DIR})
#add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${Python_EXECUTABLE} ${screepsxx_SOURCE_DIR}/tools/upload.py ${CMAKE_SOURCE_DIR}/dist $ENV{SCREEPS_TOKEN} ${SCREEPS_BRANCH})