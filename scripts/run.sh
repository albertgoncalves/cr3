#!/usr/bin/env bash

set -eu

    # "-fsanitize=address"
    # "-fsanitize=bounds"
    # "-fsanitize=float-divide-by-zero"
    # "-fsanitize=implicit-conversion"
    # "-fsanitize=integer"
    # "-fsanitize=nullability"
    # "-fsanitize=undefined"
flags=(
    -D_DEFAULT_SOURCE
    -D_POSIX_C_SOURCE
    "-ferror-limit=1"
    -fshort-enums
    -g
    -lGL
    -lglfw
    "-march=native"
    -O3
    "-std=c99"
    -Werror
    -Weverything
    -Wno-c2x-extensions
    -Wno-declaration-after-statement
    -Wno-disabled-macro-expansion
    -Wno-padded
    -Wno-reserved-macro-identifier
    -Wno-unused-function
)

clang-format -i -verbose "$WD/src/"*
mold -run clang "${flags[@]}" -o "$WD/bin/main" "$WD/src/main.c"
"$WD/bin/main" "$WD/src/vert.glsl" "$WD/src/frag.glsl"
