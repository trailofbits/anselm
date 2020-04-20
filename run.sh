#!/bin/sh

clang -O3 -emit-llvm /tests/program.c -c -o /tests/program.bc
opt-9 -load=/anselm/libAnselm.so -anselm -anselm-pattern=/tests/pattern.txt < /tests/program.bc > /dev/null
