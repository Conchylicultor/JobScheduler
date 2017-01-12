#ifndef JS_QUEUESCHEDULER_H
#define JS_QUEUESCHEDULER_H

#include <list>
#include <memory>
#include <mutex>
#include <future>

#include "workerfactory.hpp"
#include "queuethread.hpp"


namespace job_scheduler
{


/** QueueScheduler allows to parallelize the work among threads while keeping the
  * output sequencial with respect to the input.
  * The pop call will be blocking while the release token hasn't been pushed.
  */
template <typename Input, typename Output, class Worker>  // TODO: Could the number of templates args could be reduced (redundancy) ?
class QueueScheduler
{

using InputPtr = std::unique_ptr<Input>;
using OutputPtr = std::unique_ptr<Output>;
using WorkerPtr = std::unique_ptr<Worker>;

public:
    QueueScheduler() = default;
    QueueScheduler(const QueueScheduler&) = delete;
    QueueScheduler& operator=(const QueueScheduler&) = delete;
    ~QueueScheduler() = default;

    /** Construct some workers using the given factory
      * TODO: What to do with the worker ids ? Reset each time ?
      * or increment continuously ?
      */
    void add_workers(
        const WorkerFactory<Worker>& factory = {},
        int nbWorker = 1
    );

    /** Start launching the workers, with the given feeder
      * Warning: if two feeders are launched at the same time,
      * the behavior is undefined.
      */
    void launch(const std::function<InputPtr()>& feeder);

    // Queues modifiers

    /** Block while the list is empty.
      * Return the First-In has soon as it has been released
      */
    OutputPtr pop();

    /** Final token. Make the pop call non blocking
      */
    void push_release();

    // Utils

    /** Convinience method for communication between main thread and the
      * workers.
      * WARNING: Not thread safe. Should be only used before the launch
      * call while no worker is working
      */
    const std::list<WorkerPtr>& get_workers();

private:
    /** Launch the workers and feed them
      * Run asynchronusly
      */
    void scheduler_job(const std::function<InputPtr()>& feeder);

    /** Feeder thread which tries to permanatly feed the queue
      * Wait when the queue is full
      */
    void feeder_job();

    /** Worker thread which process a single input and update the future result
      * previously pushed on the queue
      */
    OutputPtr worker_job(WorkerPtr worker, InputPtr input);

    // Thread safe collections
    QueueThread<WorkerPtr> _availableWorkers;
    QueueThread<InputPtr> _inputQueue;
    QueueThread<std::future<OutputPtr>> _outputQueue;

    size_t maxInputSize = JS_UNLIMITED;
    size_t maxOutputSize = JS_UNLIMITED;

    std::future<void> _schedulerFutur;  // Is linked to the schedulerFutur (is necessary to avoid blocking async)
};


// TODO: Try to encapsulate the class inside Feeder without having to declare
// the template Feeder<void>::ExpiredException
class ExpiredException : public std::exception
{
};


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::add_workers(
    const WorkerFactory<Worker>& factory,
    int nbWorker
)
{
    for (int i = 0 ; i < nbWorker ; ++i)
    {
        _availableWorkers.push_back(factory.buildNew(i));
    }
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::launch(const std::function<InputPtr()>& feeder)
{
    // Will launch the scheduler
    _schedulerFutur = std::async(  // To avoid blocking call, need to capture the future in a member variable (future has blocking destructor)
        std::launch::async,
        &QueueScheduler::scheduler_job, this, // Will call this->scheduler_job(feeder)
        feeder
    );
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::scheduler_job(const std::function<InputPtr()>& feeder)
{
    try
    {
        while(true)  // Exit when the feeder expire (TODO: Could also add a timeout or other exit conditions)
        {
            // Fetch next input
            InputPtr input = feeder();  // Get the next input (eventually exit)

            // In case of exit, even if there has been some threads which did not
            // finished yet, all previous futures have already been pushed to the
            // Queue, so the main program will grab all the frames

            // Launch the task (encapsulate the worker)
            std::future<OutputPtr> returnedValue = std::async(
                std::launch::async,
                &QueueScheduler::worker_job, this,
                _availableWorkers.pop_front(),  // Wait for an available worker
                std::move(input)
            );

            // Push the returnedValue into the output queue
            // the order is concerved (will be used to reference the output
            // while keeping track of the  order)
            _outputQueue.push_back(std::move(returnedValue));
        }
    }
    catch (const ExpiredException& e)
    {
        // Release the queue
        push_release();
    }
}


template <typename Input, typename Output, class Worker>
auto QueueScheduler<Input, Output, Worker>::worker_job(WorkerPtr worker, InputPtr input) -> OutputPtr
{
    // Launch the task
    OutputPtr output = (*worker)(*input.get());

    // The worker finished its job, so can be used again
    _availableWorkers.push_back(std::move(worker));

    // Release the future
    return output;
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::push_release()
{
    // TODO: Make sure this function is called only once ? <= In that case,
    // be sure to reinitialize when calling launch again
    std::promise<OutputPtr> promise;
    std::future<OutputPtr> finalToken = promise.get_future();
    promise.set_value(OutputPtr(nullptr));

    _outputQueue.push_back(std::move(finalToken));
}


template <typename Input, typename Output, class Worker>
auto QueueScheduler<Input, Output, Worker>::pop() -> OutputPtr
{
    std::future<OutputPtr> output = _outputQueue.pop_front();
    return output.get();  // Will wait for the worker to finish
}


template <typename Input, typename Output, class Worker>
auto QueueScheduler<Input, Output, Worker>::get_workers() -> const std::list<WorkerPtr>&
{
    return _availableWorkers.get_data();
}



} // End namespace


#endif
