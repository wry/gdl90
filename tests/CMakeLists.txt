project(gdl90-tests)

add_executable(gdl90-tests
  src/main.c
)
target_compile_options(gdl90-tests
  PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
target_link_libraries(gdl90-tests
  PRIVATE
    gdl90
)

add_test(NAME GDL90Heartbeat COMMAND gdl90-tests 0)
add_test(NAME GDL90Initialization COMMAND gdl90-tests 2)
add_test(NAME GDL90UplinkData COMMAND gdl90-tests 7)
add_test(NAME GDL90HeightAboveTerrain COMMAND gdl90-tests 9)
add_test(NAME GDL90OwnshipReport COMMAND gdl90-tests 10)
add_test(NAME GDL90OwnshipGeometricAltitude COMMAND gdl90-tests 11)
add_test(NAME GDL90TrafficReport COMMAND gdl90-tests 20)
add_test(NAME GDL90BasicReport COMMAND gdl90-tests 30)
add_test(NAME GDL90LongReport COMMAND gdl90-tests 31)
