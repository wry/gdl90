project(gdl90-cli)

add_executable(gdl90-cli)

target_link_libraries(gdl90-cli
  PRIVATE
    gdl90
)

target_sources(gdl90-cli
  PRIVATE
    src/main.c
)
