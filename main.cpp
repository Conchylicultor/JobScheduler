#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>

// TODO: Should encapsulate includes into include/job_scheduler/... (and have a include/job_scheduler.hpp)
#include <job_scheduler.hpp>

#include "main_utils.hpp"


/** Test the worker factory to generate new workers on the fly and demonstrate that
  * the arguments of the factory are correctly forwarded to the workers.
  * Use ArgumentLogger to track the copies/moves.
  */
void testWorkerFactory()
{
    std::cout << "########################## Demo testWorkerFactory ##########################" << std::endl;

    const int nb_workers = 3;

    job_scheduler::WorkerFactory<WorkerTest> factory{"Shared message", ArgumentLogger{}};  // The factory arguments will be passed to the constructor of each worker
    for (int i = 0 ; i < nb_workers ; ++i)
    {
        std::cout << "Creation of worker " << i << std::endl;
        auto workerTest = factory.buildNew(i);  // Return a unique_ptr
        std::cout << *workerTest << " operational" << std::endl;
    }
}


/** Simple feeder demonstration which will generate numbers from 1 to 10
  * before expiring
  */
void testFeeder()
{
    std::cout << "########################## Demo testFeeder ##########################" << std::endl;

    const int in_max = 10; // The number of values to generate

    FeederTest feeder(in_max);

    bool finished = false;
    while (!finished)
    {
        try
        {
            int val = feeder();
            std::cout << "Next value generated: " << val << std::endl;
        }
        catch (const job_scheduler::ExpiredException& e)
        {
            std::cout << "Generator expired" << std::endl;
            finished = true;
        }
    }
}


/** Same as testFeeder, but by using ArgumentLogger, we can check that
  * copy elision is used on the inputs.
  */
void testFeederArgs()
{
    std::cout << "########################## Demo testFeederArgs ##########################" << std::endl;

    const int in_max = 3; // The number of values to generate
    int in_counter = 0;

    std::function<ArgumentLogger()> feeder([&in_counter, in_max]() { // Generate the numbers from 0 to in_max
        if (in_counter < in_max)
        {
            ++in_counter;
            return ArgumentLogger{};
        }
        throw job_scheduler::ExpiredException();
    });

    bool finished = false;
    while (!finished)
    {
        try
        {
            ArgumentLogger val = feeder();
            std::cout << "Next value generated: " << val << std::endl;
        }
        catch (const job_scheduler::ExpiredException& e)
        {
            std::cout << "Generator expired" << std::endl;
            finished = true;
        }
    }
}


/** This test demonstrate the QueueThread class. Just a simple Queue thread safe
  * The values are popped in the order they are added. Here all thread try to
  * push on the queue at the same time so the input order in non deterministic.
  */
void testQueueThreadPop()
{
    std::cout << "########################## Demo testQueueThreadPop ##########################" << std::endl;

    const int ending_value = -1;
    int nb_thread = 10;

    job_scheduler::QueueThread<int> queue{};

    std::cout << "Main: Launching threads..." << std::endl;
    std::vector<std::thread> list_threads;
    for (int i = 0 ; i < nb_thread ; ++i)
    {
        list_threads.emplace_back([&queue, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            //std::cout << std::this_thread::get_id() << ": Pushing " << i << std::endl;
            queue.push_back(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    std::cout << "Main: All threads launched..." << std::endl;

    auto f = std::async(
        std::launch::async,
        [&list_threads, &queue, ending_value]{
            std::cout << std::this_thread::get_id() << ": Joining threads..." << std::endl;
            for(auto& t : list_threads)
            {
                t.join();
            }
            std::cout << std::this_thread::get_id() << ": Threads joined, pushing exit token..." << std::endl;
            queue.push_back(ending_value);
        }
    );

    int val;
    std::cout << "Main: Waiting for pop..." << std::endl;
    while ((val = queue.pop_front()) != ending_value)
    {
        std::cout << "Main: Popping value: " << val << std::endl;
    }
    std::cout << "Main: exit token received, loop ended" << std::endl;
}


/** Test of QueueThread to demonstrate the maxSize parameter which make pushing
  * to the queue blocking
  */
void testQueueThreadPush()
{
    std::cout << "########################## Demo testQueueThreadPush ##########################" << std::endl;

    const int ending_value = -1;
    int nb_thread = 7;
    size_t maxSize = 3;

    job_scheduler::QueueThread<int> queue{maxSize};

    // All threads try to push the value at the same time, Only the fastest
    // will succed
    std::cout << "Main: Launching threads..." << std::endl;
    std::vector<std::thread> list_threads;
    for (int i = 0 ; i < nb_thread ; ++i)
    {
        list_threads.emplace_back([&queue, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Used to break the sequencial construction
            queue.push_back(i);
            PrintThread{} << i << " pushed..." << std::endl;
        });
    }

    PrintThread{} << "Main: Temporizing..." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    PrintThread{} << "Main: Only " << maxSize << " elements should have been pushed at this time..." << std::endl;

    auto f = std::async(
        std::launch::async,
        [&list_threads, &queue, ending_value]{
            for(auto& t : list_threads)
            {
                t.join();
            }
            PrintThread{} << "All threads joined, pushing exit token..." << std::endl;
            queue.push_back(ending_value);
        }
    );

    int i = 0;
    while (queue.pop_front() != ending_value)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Leave enough time for a new pushes (but only one will succed)
        if(i < nb_thread - static_cast<int>(maxSize))
        {
            PrintThread{} << "Main: One more..." << std::endl; // Only one pop, so only one push
        }
        ++i;
    }
    std::cout << "Main: exit token received, loop ended" << std::endl;
}


/** Main example which demonstrate how to use all previous classes together
  */
void testSequencialQueue()
{
    std::cout << "########################## Demo testSequencialQueue ##########################" << std::endl;

    const int in_max = 30; // The number of values to generate
    const int nb_workers = 3;

    // Create the job queue
    job_scheduler::QueueScheduler<int, std::string, WorkerTest> queue{};

    // Will construct workers on the fly, using the shared params (Each worker also have a unique id)
    queue.add_workers(
        {"Shared message"}, // job_scheduler::WorkerFactory<WorkerTest>{}
        nb_workers
    );

    // Launch the job scheduler on a separate thread
    // The job scheduler will feed each workers until the feeder expire
    // and there is no more work to do
    queue.launch(FeederTest(in_max));

    // The values are popped from the same order they have been added, as soon
    // they are available (processed by worker)
    while(std::unique_ptr<std::string> out = queue.pop()) // Get the next processed output
    {
        std::cout << "Popped value: " << *out << std::endl;
    }
}


/** Example which demonstrate the reusability of the worker with another feeder
  */
void testSequencialQueueReuse()
{
    std::cout << "########################## Demo testSequencialQueueReuse ##########################" << std::endl;

    const int nb_workers = 3;

    job_scheduler::QueueScheduler<int, std::string, WorkerTest> queue{};

    queue.add_workers({}, nb_workers);

    std::function<void(int)> launchFeeder([&queue](int feederSize) {
        std::cout << "Launching generator of " << feederSize << " values" << std::endl;

        queue.launch(FeederTest(feederSize));

        while(std::unique_ptr<std::string> out = queue.pop()) // Get the next processed output
        {
            std::cout << "Popped value: " << *out << std::endl;
        }
    });

    launchFeeder(3);
    launchFeeder(5);
    launchFeeder(2);

}


/** Show how to access directly the underlying workers of the
  */
void testWorkerAccess()
{
    std::cout << "########################## Demo testWorkerAccess ##########################" << std::endl;

    const int nb_workers = 5;

    job_scheduler::QueueScheduler<int, std::string, WorkerTest> queue{};
    queue.add_workers({"Shared message"}, nb_workers);

    for(auto& worker : queue.get_workers())  // Can be const& or &
    {
        std::cout << "Worker: " << *worker << std::endl;
    }
}


int main(int argc, char** argv)
{
    (void)argc;  // Unused
    (void)argv;

    std::cout << "Demo JobScheduler Conch - 2016" << std::endl;

    testWorkerFactory();
    testFeeder();
    testFeederArgs(); // Same that testFeeder, but ensure that Copy elision is used (TODO: Should merge both int and ArgsLog classes and declare copy cst private)
    testQueueThreadPop();
    testQueueThreadPush();
    testSequencialQueue();
    testSequencialQueueReuse();
    testWorkerAccess();

    std::cout << "The end" << std::endl;
    return 0;
}
