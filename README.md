# Serpent

## Structure

- include: **serpent.hpp** 
- src: **serpent.cpp** -- serpent core implementation
- tables: serpent tables
- test
    - basic-usage -- basic invertability test

## Building

You need cmake, make and compiler supporting C++20 installed
```sh
mkdir build
cd build
cmake ..
make
```

Now you can run test:
```sh
./test/basic-usage/basic_usage
```
