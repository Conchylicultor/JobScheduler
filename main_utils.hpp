#ifndef DEMO_UTILS_H  // Reduce colisions risk with prefix demo_
#define DEMO_UTILS_H


#include <iostream>


class ArgumentLogger;
std::ostream& operator<<(std::ostream& os, const ArgumentLogger& obj);

/** Convinience class to log the variadic arguments flow
  * Only used for debugging
  * TODO: Use a static hashmap to map adresses into human readable
  * name (instead of priniting *this) or simpler add a member std::string name
  */
class ArgumentLogger
{
public:
    ArgumentLogger()
    {
        std::cout << *this << ": creation " << std::endl;
    }
    ArgumentLogger(const ArgumentLogger& other)
    {
        std::cout << *this << ": creation by const copy of " << &other << std::endl;
    }
    ArgumentLogger(ArgumentLogger&& other)
    {
        std::cout << *this << ": creation by move of" << &other << std::endl;
    }
    ArgumentLogger& operator= (const ArgumentLogger& other)
    {
        std::cout << *this << ": assignement of " << &other << std::endl;
        return *this;
    }
    ArgumentLogger& operator= (ArgumentLogger&& other)
    {
        std::cout << *this << ": move assignement of " << &other << std::endl;
        return *this;
    }
    ~ArgumentLogger()
    {
        std::cout << *this << ": destruction" << std::endl;
    }
};

std::ostream& operator<<(std::ostream& os, const ArgumentLogger& obj)
{
    os << "Arg " << &obj;
    return os;
}


/** Sample worker class
  * Has to overload the operator ()
  */
class WorkerTest
{
public:
    WorkerTest() = delete;
    WorkerTest(int i, const std::string& message, const ArgumentLogger& ag) : worker_id(i), nb_call(0)
    {
        std::cout << "Constructing worker " << i << " (message=" << message << ", arg=" << ag<< ")"<< std::endl;
    }
    WorkerTest(int i, const std::string& message = "") : worker_id(i), nb_call(0)
    {
        std::cout << "Constructing worker " << i ;
        if (!message.empty())
        {
            std::cout << " (message=" << message << ")";
        }
        std::cout << std::endl;
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
private:
    int worker_id;
    int nb_call;

    friend std::ostream& operator<<(std::ostream& os, const WorkerTest& obj);
};

std::ostream& operator<<(std::ostream& os, const WorkerTest& obj)
{
    os << "Worker " << obj.worker_id;
    return os;
}


/** Sample feeder class
  * Generate the input values for the workers
  */
class FeederTest
{
public:
    FeederTest(int max_value) : _counter(0), _max_value(max_value)
    {}

    int operator() ()
    {
        if (_counter < _max_value)
        {
            return _counter++;
        }
        throw job_scheduler::ExpiredException();  // Important: Generator expired
    }

private:
    int _counter;
    int _max_value;
};

#endif
