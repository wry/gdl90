project(gdl90-cli)

add_executable(gdl90-cli)

set_target_properties(gdl90-cli
  PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
target_sources(gdl90-cli
  PRIVATE
    src/main.c
)
target_link_libraries(gdl90-cli
  PRIVATE
    gdl90
)
