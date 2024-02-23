project(gdl90-wasm VERSION 0.0.1)

set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
set(CMAKE_VERBOSE_MAKEFILE "ON")

add_executable(gdl90-wasm)

target_link_libraries(gdl90-wasm
  PRIVATE
    gdl90
)
target_sources(gdl90-wasm
  PRIVATE
    src/gdl90-wasm.cpp
)
target_include_directories(gdl90-wasm
  PUBLIC
    src
)
set_target_properties(gdl90-wasm
  PROPERTIES
    COMPILE_FLAGS
      "-Os"
    LINK_FLAGS
      "\
      -Os \
      -s WASM=1 \
      -s INVOKE_RUN=0 \
      -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
      -s STANDALONE_WASM \
      --allow-undefined \
      --no-entry \
      "
    OUTPUT_NAME
      gdl90
)
