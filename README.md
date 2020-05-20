# Anselm

Anselm is a tool that allows you to describe and detect patterns of bad behavior in function calls. Its primary advantage over static analysis is that it can operate on any programming language that compiles to LLVM intermediate representation (IR), or any compiled code that can be lifted back to LLVM's intermediate representation.

### Patterns
In general, patterns will look something like this:
```
EVP_CIPHER_CTX_new x
! EVP_EncryptInit_ex _ x _ _ _ _
EVP_EncryptUpdate _ x _ _ _ _
```

Each line represents a function call by specifying a name. It is then followed by a series of "tokens". The first token is the return value; the rest are the arguments. If we don't care about a specific value, we place an underscore. Any other value can be used to "match" values. Finally, we can add ! to imply that the function call does **not** exist in the pattern at that point.

So the example above is saying: any created context cannot be used to encrypt before it is initialized. Similarly, this example is the classic requirement that you free after allocate:
```
EVP_CIPHER_CTX_new x
! EVP_CIPHER_CTX_free _ x
```

### Example Container
```
docker build -t anselm .
docker run -v $(pwd)/tests:/tests --rm anselm
```

### Usage
To use Anselm yourself you can compile it with CMake or use the Dockerfile to build it with LLVM 9 in a container. Once you've done that you need to compile your program to LLVM IR. A simple example:
```
clang -O3 -emit-llvm /tests/program.c -c -o /tests/program.bc
```
Finally you can use `opt-9` to load the Anselm lib, provide the patterns you're interested in, and the path to your IR compiled binary.
```
opt-9 -load=/path/to/libAnselm.so -anselm -anselm-pattern=pattern.txt < /tests/program.bc
```

### Structure
* `Anselm.cpp` Wrapper LLVM Pass that passes functions into a context
* `Context.cpp` Searches for a specific function call pattern in a function
