# Screeps
Code for the MMO-programming game Screeps. This project will be my first contact with c++, scary stuff. 
I hope to learn the language during this project and progressively improve.
The JavaScript wrapper that I will be using is from the screepsxx project, which I will and try to expand as well.

# Running the project
All commands are run starting from the root directory.

## Emsdk setup
Screeps only supports ES5 js syntax which can only be guaranteed by using emsdk 2.0.0.

```
cd <emsdk_dir>
./emsdk install 2.0.0
./emsdk activate 2.0.0
```

## Cmake setup
Create the makefiles using cmake. For more info look at the readme in screepsxx.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=<emsdk_dir>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
```

# Build
To build the project run the following. 

```
cd build
cmake --build .
```