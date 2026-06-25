# Screeps - Peabrain
Code for the MMO-programming game Screeps. This project will be my first contact with c++, scary stuff.
I hope to learn the language during this project and progressively improve.
The JavaScript wrapper that I will be using is from the screepsxx project, which I will and try to expand as well.


## screepsxx

**screepsxx** is a C++ library for [Screeps](https://screeps.com) API.
It provides wrappers for the majority of classes in [Screeps API](https://docs.screeps.com/api).




# Getting the project running

A record of how this project (`screepsxx`-based C++ bot for Screeps) was set up on Windows, and why each step exists. Useful for rebuilding on a new machine, onboarding, or future debugging.

## Overview

The bot is written in C++, compiled to WebAssembly via Emscripten, and loaded into Screeps through a small JavaScript shim. Screeps itself only ever runs JavaScript — the `.wasm` module is loaded as a binary module and called into once per game tick.

```
Screeps server (each tick)
  -> runs main.js
     -> loads compiled app_module.wasm via wasm_loader.js
        -> calls exported C++ loop()
```

## 1. Toolchain installation

| Tool                                      | Purpose                        | Install location                                          |
|-------------------------------------------|--------------------------------|-----------------------------------------------------------|
| [Emscripten SDK](https://emscripten.org/) | Compiles C++ to WebAssembly    | `C:\emsdk`                                                |
| [Python 3](https://python.org)            | Required by Emscripten tooling | `C:\Users\<User>\AppData\Local\Programs\Python\Python313` |
| CMake 3.16+                               | Build configuration            | bundled with CLion / used standalone                      |
| Ninja                                     | Build system used by CMake     | bundled with CLion                                        |


### Emscripten setup

```powershell
cd C:\
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
.\emsdk install 2.0.0
.\emsdk activate 2.0.0
```

## 2. Project structure

The `example/` folder from the [screepsxx](https://github.com/UltraCoderRU/screepsxx) repo was used as the project root directly (its contents become the top-level project files, not a subfolder), with the library itself added as a git submodule:

```powershell
cd C:\Users\warre\CLionProjects\Screeps
git submodule add https://github.com/<your-fork>/screepsxx.git
```

Final structure:
```
Screeps/
├── CMakeLists.txt
├── Peabrain               <- actual code of the Peabrain
├── js/
│   ├── main.js            <- Screeps entry point, loads the wasm module
│   └── wasm_loader.js     <- generic Emscripten module loader glue
├── screepsxx/             <- git submodule (the library)
├── dist/                  <- build output staging folder (generated)
└── build/                 <- out-of-source CMake build folder (generated)
```

## 3. Build configuration


```powershell
cd C:\Users\<User>\CLionProjects\Screeps
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/<emsdk_location>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
```

Day-to-day rebuilds only need:
```powershell
cmake --build .
```
(Re-run the full configure line only if `build/` is deleted, or `CMakeLists.txt` changes in ways CMake can't auto-detect, e.g. new cache variables.)


## 4. Deployment targets

Two deployment paths were set up, both triggered automatically as CMake post-build steps.

### A. Local private server (Steam version)

The Steam client's private server syncs code directly from a folder on disk — no HTTP API involved:
```
C:\Users\<User>\AppData\Local\Screeps\scripts\127_0_0_1___21025\<branch>\
```

Post-build step copies the `dist/` folder straight there.
```cmake
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dist "C:/Users/warre/AppData/Local/Screeps/scripts/127_0_0_1___21025/default/")

```


### B. Official screeps.com server

You can post to different branches than the default branch by changing the ` "default" ` to another (pre-created) branch name.
Uses `screepsxx/tools/upload.py`:
```cmake
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${Python_EXECUTABLE} ${screepsxx_SOURCE_DIR}/tools/upload.py ${CMAKE_SOURCE_DIR}/dist $ENV{SCREEPS_TOKEN} "default")
```

Requires:
- An auth token from Screeps account settings, stored in a permanent environment variable:
  ```powershell
  [System.Environment]::SetEnvironmentVariable('SCREEPS_TOKEN', 'your-token', 'User')
  ```
- The target branch must already exist in the Screeps account (create via the in-game Code IDE) before pushing to it.

## 5. Features

### Automatic room planning 