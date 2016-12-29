#include <iostream>
#include <string>

// TODO: Should encapsulate includes into include/js_conch/... (and have a include/js_conch.hpp)
//#include <job_scheduler.hpp>
#include <workerfactory.hpp>
#include <feeder.hpp>
#include <queuescheduler.hpp>

#include "utils.hpp"


class WorkerTest
{
public:
    WorkerTest() = delete;
    WorkerTest(int i, const std::string& message, const ArgumentLogger& ag) : worker_id(i), nb_call(0)
    {
        std::cout << "Constructing worker " << i << " (message=" << message << ", arg=" << ag<< ")"<< std::endl;
    }
    WorkerTest(int i, const std::string& message) : worker_id(i), nb_call(0)
    {
        std::cout << "Constructing worker " << i << " (message=" << message << ")"<< std::endl;
    }

    std::unique_ptr<std::string> operator()(int input)  // Process the data
    {
        ++nb_call;
        return std::unique_ptr<std::string>(
            new std::string(
                "Worker " + std::to_string(worker_id) + ": input=" + std::to_string(input) + " (" + std::to_string(nb_call)+ " call)"
            )
        );
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


void testSequencialQueue()
{
    std::cout << "########################## Demo testSequencialQueue ##########################" << std::endl;

    const int in_max = 30; // The number of values to generate
    int in_counter = 0;
    const int nb_workers = 3;

    // Create the working queue and intitialize the workers
    js_conch::QueueScheduler<int, std::string, WorkerTest> queue(
        std::move(js_conch::Feeder<int>([&in_counter, in_max]() { // Generate the numbers from 0 to in_max
            if (in_counter < in_max)
            {
                return in_counter++;
            }
            throw js_conch::ExpiredException();
        })),
        std::move(js_conch::WorkerFactory<WorkerTest>{"Shared message"}),  // Will construct workers on the fly, with the init params (TODO: Each worker should also have a unique id)
        nb_workers
    );

    // Launch the job scheduler
    // The job scheduler will feed each workers until the feeder expire
    // and there is no more work to do
    // Launch the job scheduler with a list of workers (Contains the networks,...)
    queue.launch();

    // The values are poped from the same order they have been added, as soon
    // they are available (processed by worker)
    while(std::unique_ptr<std::string> out = queue.pop()) // Get the next JobKit
    {
        std::cout << "Popped value: " << *out << std::endl;
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
    testSequencialQueue();

    std::cout << "The end" << std::endl;
    return 0;
}
