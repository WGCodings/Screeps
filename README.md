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

Room planning is handled by the `Colony` class (`Peabrain/src/Room/Colony.cpp`).

Planning stores all intended structures in `Memory.rooms[roomName].blueprint` as a flat object of named entries, keyed by `"structType_x_y"`:
```json
{
  "blueprint": {
    "container_12_8":  { "x": 12, "y": 8,  "sType": "container", "cLevel": 1, "status": "planned"},
    "link_13_8":       { "x": 13, "y": 8,  "sType": "link",      "cLevel": 5, "status": "planned"},
    "road_14_9":       { "x": 14, "y": 9,  "sType": "road",      "cLevel": 2, "status": "planned"}
  }
}
```

Each entry has:
- `x`, `y` — tile coordinates in the room (0–49)
- `sType` — structure type constant (e.g. `"container"`, `"link"`, `"road"`)
- `cLevel` — minimum room controller level before this structure should be built
- `status` — `"planned"` initially
- `role` — optional hint for logic
  The following planning functions are implemented:

| Function           | What it places                                                                         |
|--------------------|----------------------------------------------------------------------------------------|
| `planContainers()` | One container adjacent to each energy source, on the tile closest to the controller    |
| `planLinks()`      | One hub link adjacent to the spawn + one spoke link adjacent to each planned container |
| `planRoads()`      | Roads from each source → storage, and controller → storage, using `Room::findPath`     |
| `planStorage()`    | One storage near the room center                                                       |
| `planSpawns()`     | Spawn placement for future expansion rooms                                             |
| `planTowers()`     | Tower placement scaled to RCL                                                          |
| `planExtensions()` | Extension grid placement scaled to RCL                                                 |


### Blueprint visualizer

A Python utility (`tools/visualize_room.py`) renders the blueprint as an interactive 50×50 grid. It uses only Python's built-in `tkinter` — no pip installs required.

**Step 1 — export memory from the in-game console:**
```javascript
JSON.stringify(Memory.rooms)
```
Copy the output and save it as `memory.json`.

**Step 2 — run the visualizer:**
```bash
python visualize_room.py memory.json E28S42
```
Replace `E28S42` with your room name. If the room name is omitted, the first room in the file is used.

The visualizer shows:
- Each structure type in its own colour, with a short label (`C` = container, `L` = link, `St` = storage, `T` = tower, `E` = extension, `R` = rampart, `·` = road)
- A small **cLevel tint** in the bottom-right corner of each tile — darker = unlocks at a lower RCL, brighter = higher RCL requirement
- **Dashed borders** on tiles with `"status": "planned"` (all tiles initially)
- **Hover** over any tile to see full details (type, coordinates, RCL requirement, role, status) in the status bar at the bottom
- A legend panel on the right showing colour→type and RCL tint mappings

### Roadmap

Just some ideas I'm having that I would like to implement at some point:
* Dashboard where I can see statistics about energy minder, energy used on all types, etc. Would to helpful to see if a patch makes the colony more efficient.
* Idle counter to each creep. If it passes x ticks, then reset memory to default value.