Comments probably have helpful information as well.

### Structure
* `Anselm.cpp` Wrapper LLVM Pass that passes functions into a context
* `Context.cpp` Searches for a specific function call pattern in a function

### Running
```
docker build -t anselm .
docker run -v $(pwd)/tests:/tests --rm anselm
```
