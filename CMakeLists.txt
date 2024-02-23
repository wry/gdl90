cmake_minimum_required(VERSION 3.22)

project(gdl90-dev)

add_subdirectory(src)

add_subdirectory(examples)

target_compile_options(gdl90
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

if (TARGET gdl90-wasm)
    target_compile_options(gdl90-wasm
        PRIVATE
            $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    )
endif()

if (TARGET gdl90-cli)
    target_compile_options(gdl90-cli
        PRIVATE
            $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
            $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    )
endif()

if (NOT DEFINED EMSCRIPTEN)
    enable_testing()
    add_subdirectory(tests)
endif()
