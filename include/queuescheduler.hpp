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
    void launch(const std::function<Input()>& feeder);

    // Queues modifiers

    /** Block while the list is empty.
      * Return the First-In has soon as it has been released
      */
    std::unique_ptr<Output> pop();

    /** Final token. Make the pop call non blocking
      */
    void push_release();

    // Utils

    /** Convinience method for communication between main thread and the
      * workers.
      * WARNING: Not thread safe. Should be only used before the launch
      * call while no worker is working
      */
    const std::list<std::unique_ptr<Worker>>& get_workers();

private:
    /** Launch the workers and feed them
      * Run asynchronusly
      */
    void scheduler_job(const std::function<Input()>& feeder);

    /** Manage the job of a particular worker.
      */
    std::unique_ptr<Output> worker_job(std::unique_ptr<Worker> worker, const Input& input);

    // Thread safe collections
    QueueThread<std::unique_ptr<Worker>> _availableWorkers;
    QueueThread<std::future<std::unique_ptr<Output>>> _outputQueue;

    std::future<void> _schedulerFutur;  // Is linked to the schedulerFutur (is necessary to avoid blocking async)
};


// TODO: Try to encapsulate tht class inside Feeder without having to declare
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
void QueueScheduler<Input, Output, Worker>::launch(const std::function<Input()>& feeder)
{
    // Will launch the scheduler
    _schedulerFutur = std::async(  // To avoid blocking call, need to capture the future in a member variable (future has blocking destructor)
        std::launch::async,
        [this, feeder]{this->scheduler_job(feeder);}  // Should be replaced by &QueueScheduler::scheduler_job, this
    );
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::scheduler_job(const std::function<Input()>& feeder)
{
    try
    {
        while(true)  // Exit when the feeder expire (TODO: Could also add a timeout or other exit conditions)
        {
            // Fetch next input
            Input input = feeder();  // Get the next input (eventually exit)

            // In case of exit, even if there has been some threads which did not
            // finished yet, all previous futures have already been pushed to the
            // Queue, so the main program will grab all the frames

            // Wait for an available worker
            std::unique_ptr<Worker> worker = _availableWorkers.pop_front();

            // Launch the task (encapsulate the worker)
            std::future<std::unique_ptr<Output>> returnedValue = std::async(
                std::launch::async,
                [worker = std::move(worker), input, this] () mutable {  // TODO: Replace by this->worker_job(worker, input) ?
                    // Launch the task
                    std::unique_ptr<Output> output = (*worker)(input);

                    // The worker finished its job, so can be used again
                    this->_availableWorkers.push_back(std::move(worker));

                    // Release the future
                    return output;
                }
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
void QueueScheduler<Input, Output, Worker>::push_release()
{
    // TODO: Make sure this function is called only once ? <= In that case,
    // be sure to reinitialize when calling launch again
    std::promise<std::unique_ptr<Output>> promise;
    std::future<std::unique_ptr<Output>> finalToken = promise.get_future();
    promise.set_value(std::unique_ptr<Output>(nullptr));

    _outputQueue.push_back(std::move(finalToken));
}


template <typename Input, typename Output, class Worker>
std::unique_ptr<Output> QueueScheduler<Input, Output, Worker>::pop()
{
    std::future<std::unique_ptr<Output>> output = _outputQueue.pop_front();
    return output.get();  // Will wait for the worker to finish
}


template <typename Input, typename Output, class Worker>
const std::list<std::unique_ptr<Worker>>& QueueScheduler<Input, Output, Worker>::get_workers()
{
    return _availableWorkers.get_data();
}



} // End namespace


#endif
