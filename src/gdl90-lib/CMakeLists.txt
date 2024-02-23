project(gdl90-lib VERSION 0.0.1)

add_library(gdl90 STATIC)

set_target_properties(gdl90
  PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
target_sources(gdl90
  PRIVATE
    src/gdl90.c
)
target_include_directories(gdl90
  PUBLIC
    src
)
install(
    TARGETS gdl90
    ARCHIVE DESTINATION lib
)
install(
    FILES src/gdl90.h DESTINATION include
)