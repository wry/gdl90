project(gdl90-examples VERSION 0.0.1)

if (NOT DEFINED EMSCRIPTEN)
    add_subdirectory(gdl90-cli)
else()
    add_subdirectory(gdl90-wasm)
endif()