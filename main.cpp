#include <iostream>
#include <string>

// TODO: Should encapsulate includes into include/js_conch/... (and have a include/js_conch.hpp)
//#include <job_scheduler.hpp>
#include <workerfactory.hpp>
#include <feeder.hpp>

#include "utils.hpp"


class WorkerTest
{
public:
    WorkerTest() = delete;
    WorkerTest(int i, const std::string& message, const ArgumentLogger& ag) : worker_id(i), nb_call(0)
    {
        std::cout << "Constructing worker " << i << " (message=" << message << ", arg=" << ag<< ")"<< std::endl;
    }

    std::string operator()(int input)  // Process the data
    {
        ++nb_call;
        return "Worker " + std::to_string(worker_id) + ": process " + std::to_string(input) + "(" + std::to_string(nb_call)+ " call)";
    }

    void print() const
    {
        std::cout << "Worker " << worker_id << " operational" << std::endl;
    }
private:
    int worker_id;
    int nb_call;
};


void testWorkerFactory()
{
    std::cout << "########################## Demo testWorkerFactory ##########################" << std::endl;

    const int nb_workers = 3;

    js_conch::WorkerFactory<WorkerTest> factory{"Shared message", ArgumentLogger{}};  // The factory arguments will be passed to the constructor of each worker
    for (int i = 0 ; i < nb_workers ; ++i)
    {
        std::cout << "Creation of worker " << i << std::endl;
        auto workerTest = factory.buildNew();  // Return a unique_ptr
        workerTest->print();
    }
}


void testFeeder()
{
    std::cout << "########################## Demo testFeeder ##########################" << std::endl;

    const int in_max = 10; // The number of values to generate
    int in_counter = 0;

    js_conch::Feeder<int> feeder([&in_counter, in_max]() -> int { // Generate the numbers from 0 to in_max
        if (in_counter < in_max)
        {
            return in_counter++;
        }
        throw js_conch::ExpiredException();
    });

    bool finished = false;
    while (!finished)
    {
        try
        {
            int val = feeder.getNext();
            std::cout << "Next value generated: " << val << std::endl;
        }
        catch (const js_conch::ExpiredException& e)
        {
            std::cout << "Generator expired" << std::endl;
            finished = true;
        }
    }
}


void testFeederArgs()
{
    std::cout << "########################## Demo testFeeder args ##########################" << std::endl;

    const int in_max = 3; // The number of values to generate
    int in_counter = 0;

    js_conch::Feeder<ArgumentLogger> feeder([&in_counter, in_max]() { // Generate the numbers from 0 to in_max
        if (in_counter < in_max)
        {
            ++in_counter;
            return ArgumentLogger{};
        }
        throw js_conch::ExpiredException();
    });

    bool finished = false;
    while (!finished)
    {
        try
        {
            ArgumentLogger val = feeder.getNext();
            std::cout << "Next value generated: " << val << std::endl;
        }
        catch (const js_conch::ExpiredException& e)
        {
            std::cout << "Generator expired" << std::endl;
            finished = true;
        }
    }
}


/*void testSequencialQueue()
{
    std::cout << "########################## Demo testSequencialQueue ##########################" << std::endl;
    int in = 0;

    // Create the working queue
    js_conch::QueueTread queue{};

    // Launch the job scheduler
    // The job scheduler will feed each workers until the feeder expire
    // and there is no more work to do
    queue.schedule(
        js_conch::Feeder([&in] -> int { // Generate the numbers from 1 to 11
            if (in > 10)
            {
                throw js_conch::Feeder::ExpiredException;
            }
            return ++in;
        }),
        js_conch::worker_factory<Worker>(init_params1, init_params2),  // Will construct workers on the fly, with the init params (TODO: Each worker should also have a unique id)
        nb_workers,
    ); // Launch the job scheduler with a list of workers (Contains the networks,...)

    // The values are poped from the same order they have been added, as soon
    // they are available (processed by worker)
    while(std::unique_ptr<std::string> out = queue.pop()) // Get the next JobKit
    {
        std::cout << *out << endl;
    }
}*/


int main(int argc, char** argv)
{
    (void)argc;  // Unused
    (void)argv;

    std::cout << "Demo JobScheduler Conch - 2016" << std::endl;

    testWorkerFactory();
    testFeeder();
    testFeederArgs(); // Same that testFeeder, but ensure that Copy elision is used (TODO: Should merge both int and ArgsLog classes and declare copy cst private)
    //testSequencialQueue();

    std::cout << "The end" << std::endl;
    return 0;
}
