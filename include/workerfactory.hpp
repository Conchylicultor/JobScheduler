#ifndef JS_CONCH_WORKER_FACTORY_H
#define JS_CONCH_WORKER_FACTORY_H

#include <memory>
#include <functional>


namespace js_conch
{

/** Wrapper arround the workers creation
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
    std::unique_ptr<Worker> buildNew();
private:
    int _nbWorker; // Keep count of the workers
    std::function<std::unique_ptr<Worker>(int)> _delayedBuilder;  // Used as proxy to pack the variadic arguments
};


// Template definition

template <class Worker>
template <typename... Args>
WorkerFactory<Worker>::WorkerFactory(Args... args) :
    _nbWorker(0),
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
std::unique_ptr<Worker> WorkerFactory<Worker>::buildNew()
{
    auto newWorker = _delayedBuilder(_nbWorker);
    ++_nbWorker;
    return std::move(newWorker);
}


} // End namespace


#endif
