# JobScheduler

A small C++ library to launch concurrent jobs while keeping the input queue order intact.

The original goal for which the library has been developed, was to process each frames of a video independently using multiple GPUs. The number of workers (here one per GPU) was negligible compare to the number of frames, and the number of frame was not known in advance (potentially needed to be used to process video streams). After processing, the processed informations needed to be in the same order than the frames. This library allows to distribute continuously the jobs among the workers, and while parallelizing the work, keeps the processed order intact. Also the frames are pre-fetched on an input queue even before a worker becomes available.

To build, as usual:

```bash
mkdir build/
cd build
cmake ..
make
```

To use the library, just include the headers (present in the `include/` folder). No `.cpp` files requires as this library is template based. After compilation, the examples present in the `build/` folder can be launched with `./a.out`. Here is a sample code of how to use the library:

```cpp
const int nb_workers = 2;

// Create the job queue (of the form <Worker>)
// The Worker class  has to override std::unique_ptr<Output> operator() (const Input&)
job_scheduler::QueueScheduler<PersonCounter> queue{};

// Initialize (construct) the workers
queue.add_workers(
    {},  // Factory to wrap the worker construction (constructed using PersonCounter{workerId, Args...})
    nb_workers // Here nb_workers == nb_gpu
);

// Launch the job scheduler on a separate thread
// The job scheduler will feed each workers until the feeder expire
// and there is no more work to do
queue.launch(
    VideoFeeder("vid.mp4") // Will read the video frame by frame
);

// The values are popped from the same order they have been added, as soon
// they are available (processed by worker)
int frame_id = 0;
while(std::unique_ptr<int> out = queue.pop()) // Get the next processed output
{
    std::cout << "Frame " << frame_id << " contains " << *out << " persons." << std::endl;
    ++frame_id;
}
```

Note that the work is not evenly distributed among the workers. If a worker process the jobs more quickly, it will receive more job to process. Also there is no temporisation mechanism by default so the main thread need to pop the output values faster than they are pushed by the workers, otherwise, the output queue can grow indefinitely (in case of an infinite feeder). You can set a maximum output or input size for the queues
