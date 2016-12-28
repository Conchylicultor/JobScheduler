# JobScheduler

A small C++ library to launch concurrent jobs while keeping the input queue order intact.

To build, as usual:

```bash
mkdir build/
cd build
cmake ..
make
```

Then, to use the library, just include `js_conch.hpp` (present in the `include/` folder) and link the generated `lib/js_conch.so`. After compilation, the examples present in the `build/` folder can be launched with `./a.out`.
