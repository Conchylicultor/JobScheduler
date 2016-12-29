# JobScheduler

A small C++ library to launch concurrent jobs while keeping the input queue order intact.

The original goal for which the library has been developed, was to process each frames of a video independently using multiple GPUs. The number of workers (here one per GPU) was negligible compare to the number of frames, and the number of frame was not known in advance (potentially needed to be used to process video streams). After processing, the processed informations needed to be in the same order than the frames. This library allows to distribute continuously the jobs among the workers, and while parallelizing the work, keeps the processed order intact.

To build, as usual:

```bash
mkdir build/
cd build
cmake ..
make
```

Then, to use the library, just include `js_conch.hpp` (present in the `include/` folder) and link the generated `lib/js_conch.so`. After compilation, the examples present in the `build/` folder can be launched with `./a.out`.
