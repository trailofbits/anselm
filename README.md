Comments probably have helpful information as well.

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

### Structure
* `Anselm.cpp` Wrapper LLVM Pass that passes functions into a context
* `Context.cpp` Searches for a specific function call pattern in a function

### Running
```
docker build -t anselm .
docker run -v $(pwd)/tests:/tests --rm anselm
```
