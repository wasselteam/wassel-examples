#!/usr/bin/env bash

set -xeu

mkdir -p bindings
wit-bindgen c ../wit -w http-plugin --out-dir bindings

mkdir -p build

CC=/opt/wasi-sdk/bin/wasm32-wasip2-clang
CFLAGS="-Ibindings"

CXX=/opt/wasi-sdk/bin/wasm32-wasip2-clang++
CXXFLAGS="-std=c++20 -Ibindings"

cp ./bindings/http_plugin_component_type.o ./build/
$CC $CFLAGS -o ./build/http_plugin.o -x c -c ./bindings/http_plugin.c
$CXX $CXXFLAGS -o plugin.wasm -mexec-model=reactor ./src/lib.cpp build/*.o
