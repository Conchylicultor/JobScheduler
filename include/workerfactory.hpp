#ifndef JS_WORKER_FACTORY_H
#define JS_WORKER_FACTORY_H

#include <memory>
#include <functional>


namespace job_scheduler
{

/** Wrapper arround the workers creation
  * The WorkerFactory just forward the given arguments when creating the workers.
  * In addition, the worker id is given as first parameter.
  */
template <class Worker>
class WorkerFactory
{
public:
    /** Every time a new worker will be created, it will be constructed with the given factory
      * arguments. In addition, the worker id will be given as first parameter
      */
    template <typename... Args>
    WorkerFactory(Args... args);  // TODO: Investigate if Args&&... and cie would be more efficient (using perfect forwarding)
    WorkerFactory(const WorkerFactory&) = default;
    WorkerFactory& operator=(const WorkerFactory&) = delete;
    ~WorkerFactory() = default;

    /** Wrapper arround the workers creation
      */
    std::unique_ptr<Worker> buildNew(int workerId) const;
private:
    std::function<std::unique_ptr<Worker>(int)> _delayedBuilder;  // Used as proxy to pack the variadic arguments
};


// Template definition

template <class Worker>
template <typename... Args>
WorkerFactory<Worker>::WorkerFactory(Args... args) :
    _delayedBuilder()
{
    // Save the args for later use
    auto make_new = [](int workerId, Args... args)  // Add 'mutable' if Args are not const ref ?
    {
        return std::unique_ptr<Worker>(new Worker(workerId, std::forward<Args>(args)...));
    };
    _delayedBuilder = std::bind(make_new, std::placeholders::_1, args...);  // Need to bind the function in order to save the arguments
}

template <class Worker>
std::unique_ptr<Worker> WorkerFactory<Worker>::buildNew(int workerId) const
{
    auto newWorker = _delayedBuilder(workerId);
    return std::move(newWorker);
}


} // End namespace


#endif
