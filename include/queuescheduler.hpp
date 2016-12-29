#ifndef JS_QUEUESCHEDULER_H
#define JS_QUEUESCHEDULER_H

#include <list>
#include <memory>
#include <mutex>
#include <future>

#include "feeder.hpp"
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
    QueueScheduler(
        const Feeder<Input>& feeder,
        const WorkerFactory<Worker>& factory,
        int nbWorker = 1
    );
    QueueScheduler(const QueueScheduler&) = delete;
    QueueScheduler& operator=(const QueueScheduler&) = delete;
    ~QueueScheduler() = default;

    /** Start launching the workers
      */
    void launch();

    // Queues modifiers

    /** Block while the list is empty.
      * Return the First-In has soon as it has been released
      */
    std::unique_ptr<Output> pop();

    /** Final token. Make the pop call non blocking
      */
    void push_release();

private:
    /** Launch the workers and feed them
      * Run asynchronusly
      */
    void schedulerJob();

    /** Manage the job of a particular worker.
      */
    std::unique_ptr<Output> workerJob(std::unique_ptr<Worker> worker, const Input& input);

    // Protect any queue access and modifications
    std::mutex _mutexOutputQueue;
    std::mutex _mutexWorkerQueue;
    std::mutex _mutexWorkerQueueEmpty;  // Lock the calls when one of the queue is empty
    std::mutex _mutexOutputQueueEmpty;

    std::list<std::unique_ptr<Worker>> _availableWorkers;
    std::list<std::future<std::unique_ptr<Output>>> _outputQueue;

    // Input and output connectors
    Feeder<Input> _feeder;
    WorkerFactory<Worker> _factory;
};


template <typename Input, typename Output, class Worker>
QueueScheduler<Input, Output, Worker>::QueueScheduler(
        const Feeder<Input>& feeder,
        const WorkerFactory<Worker>& factory,
        int nbWorker
    ) :
    _mutexOutputQueue(),
    _mutexWorkerQueue(),
    _feeder(feeder),
    _factory(factory),
    _availableWorkers{},
    _outputQueue{}
{
    for (int i = 0 ; i < nbWorker ; ++i)
    {
        _availableWorkers.push_back(_factory.buildNew());
    }
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::launch()
{
    // Will launch the scheduler
    std::async(  // TODO: To avoid blocking call, need to capture the future in a member variable (future has blocking destructor)
        std::launch::async,
        [this]{this->scheldulerJob();}  // Could be replaced by &QueueScheduler::scheldulerJob, this
    );
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::scheldulerJob()
{
    try
    {
        while(true)  // Exit when the feeder expire (TODO: Could also add a timeout or other exit conditions)
        {
            // Fetch next input
            Input input = _feeder.getNext();  // Get the next input (eventually exit)

            // In case of exit, even if there has been some threads which, all previous futures have
            // already been pushed to the Queue

            // Wait for an available worker
            // Use semaphore
            // Pop left (TODO: protected call)

            // Launch the task (encapsulate the worker )
            std::future<std::unique_ptr<Output>> returnedValue = std::async(
                std::launch::async,
                [input&]{
                    // Launch the task
                    // output = worker(input)
                    // output = (*(_availableWorkers.back()))(input);

                    // Link the ouput with the output wrapper
                    // Release the output wrapper
                    // Push the worker back on the stack, (TODO: Protected call)
                    // Release the semaphore
                }
            );

            // Push the returnedValue into the output queue
            // the order is concerved
            // (will be used to reference the output while keeping the order)
        }
    }
    catch (const ExpiredException& e)
    {
        // Release the queue
        push_release();
    }
}


template <typename Input, typename Output, class Worker>
std::unique_ptr<Output> QueueScheduler<Input, Output, Worker>::workerJob(std::unique_ptr<Worker> worker, const Input& input)
{

}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::push_release()
{
    std::lock_guard<std::mutex> guard(_mutexOutputQueue);  // Lock the output Queue while modification
}


template <typename Input, typename Output, class Worker>
std::unique_ptr<Output> QueueScheduler<Input, Output, Worker>::pop()
{
    // Wait that queue not empty
    // std::lock_guard<std::mutex> guard(_mutexOutputQueue); // Lock the queue
    // TODO: Make sure this function is called only one ?
    // Push a future on the queue
    _outputQueue.push_back(std::unique_ptr<Output>(nullptr));
}



} // End namespace


#endif
