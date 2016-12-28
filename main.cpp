#include <iostream>
#include <string>

// TODO: Should encapsulate includes into include/js_conch/... (and have a include/js_conch.hpp)
//#include <job_scheduler.hpp>
#include <workerfactory.hpp>

#include "utils.hpp"


class WorkerTest
{
public:
    WorkerTest() = delete;
    WorkerTest(int i, const std::string& message, const ArgumentLogger& ag) : worker_id(i), nb_call(0)
    {
        std::cout << "Constructing worker " << i << " (message=" << message << ")"<< std::endl;
        ag.print();
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


/*void test_sequencial_queue()
{
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
    //test_sequencial_queue();

    std::cout << "The end" << std::endl;
    return 0;
}