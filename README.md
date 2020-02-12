Comments probably have helpful information as well.

### Structure
* `Graph.cpp` Main LLVM Pass code that extracts information from IR
* `Pattern.cpp` Functions and helpers that look for patterns in the graph

### Running
```
clang -O3 -emit-llvm test.c -c -o test.bc
mkdir build
make -C build
opt -load build/libGraph.so -graph < test.bc
```
