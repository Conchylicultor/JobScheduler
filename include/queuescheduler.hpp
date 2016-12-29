#ifndef JS_CONCH_QUEUESCHEDULER_H
#define JS_CONCH_QUEUESCHEDULER_H

#include <vector>
#include <memory>
//#include <functional>

#include "feeder.hpp"
#include "workerfactory.hpp"


namespace js_conch
{

/** QueueScheduler allows to parallelize the work among threads while keeping the
  * output sequencial with respect to the input.
  * The pop call will be blocking while the release token hasn't been pushed.
  * This class is thead safe. Push, pop and release can be called from anywhere
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

    /** Start launcing the workers
      */
    void launch();

    // Queues modifiers

    /** Add a job to the queue.
      */
    void push_back(const Input&& new_elem);

    /** Block while the list is empty.
      * Return the First-In has soon as it has been released
      */
    std::unique_ptr<Output> pop();

    /** Final token. Make the pop call non blocking
      */
    void push_release();

private:
    Feeder<Input> _feeder;
    WorkerFactory<Worker> _factory;

    std::vector<std::unique_ptr<Worker>> _availableWorkers;
    /** Use shared_ptr because the job kits are shared among the queue,
      * the workers and the main program. (could we make it unique_ptr ?)
      */
    //std::list<std::shared_ptr<JobKit>> _queue;

    //friend class JobKit; // QueueThread has direct access to JobKit mutex
};


template <typename Input, typename Output, class Worker>
QueueScheduler<Input, Output, Worker>::QueueScheduler(
        const Feeder<Input>& feeder,
        const WorkerFactory<Worker>& factory,
        int nbWorker
    ) :
     _feeder(feeder),
     _factory(factory),
     _availableWorkers{}
{
    for (int i = 0 ; i < nbWorker ; ++i)
    {
        _availableWorkers.push_back(_factory.buildNew());
    }
}


template <typename Input, typename Output, class Worker>
void QueueScheduler<Input, Output, Worker>::launch()
{
}


template <typename Input, typename Output, class Worker>
std::unique_ptr<Output> QueueScheduler<Input, Output, Worker>::pop()
{
    try
    {
        Input input = _feeder.getNext();  // Get the next input
        return (*(_availableWorkers.back()))(input);
    }
    catch (const ExpiredException& e)
    {
        return std::unique_ptr<Output>(nullptr);
    }
}



} // End namespace


#endif
