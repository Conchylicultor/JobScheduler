#ifndef DEMO_UTILS_H  // Reduce colisions risk with prefix demo_
#define DEMO_UTILS_H


#include <iostream>
#include <sstream>


class ObjLogger;
std::ostream& operator<<(std::ostream& os, const ObjLogger& obj);

/** Convinience class to log an object lifetime flow (move, copy, assignement,...)
  * for instance, when passed as parameter. Only used for debugging purpose.
  */
class ObjLogger
{
public:
    // Constructors
    ObjLogger();
    ObjLogger(const ObjLogger& other);
    ObjLogger(ObjLogger&& other);

    // Assignement
    ObjLogger& operator= (const ObjLogger& other);
    ObjLogger& operator= (ObjLogger&& other);

    // Destructor
    ~ObjLogger();

private:
    static int counter;
    int m_id;

    friend std::ostream& operator<< (std::ostream& os, const ObjLogger& obj);
};

int ObjLogger::counter = 0;

ObjLogger::ObjLogger() : m_id(counter++)
{
    std::cout << *this << ": creation" << std::endl;
}

ObjLogger::ObjLogger(const ObjLogger& other) : m_id(counter++)
{
    std::cout << *this << ": creation by const copy of " << other << std::endl;
}

ObjLogger::ObjLogger(ObjLogger&& other) : m_id(counter++)
{
    std::cout << *this << ": creation by move of " << other << std::endl;
}

ObjLogger& ObjLogger::operator= (const ObjLogger& other)
{
    std::cout << *this << ": assignement of " << other << std::endl;
    return *this;
}

ObjLogger& ObjLogger::operator= (ObjLogger&& other)
{
    std::cout << *this << ": move assignement of " << other << std::endl;
    return *this;
}

ObjLogger::~ObjLogger()
{
    std::cout << *this << ": destruction" << std::endl;
}


std::ostream& operator<<(std::ostream& os, const ObjLogger& obj)
{
    os << "Obj_" << obj.m_id;
    return os;
}


/** Sample worker class
  * Has to overload the operator ()
  */
class WorkerTest
{
public:
    WorkerTest() = delete;
    WorkerTest(int i, const std::string& message, const ObjLogger& ag) : worker_id(i), nb_call(0)
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

    std::unique_ptr<int> operator() ()
    {
        if (_counter < _max_value)
        {
            return std::unique_ptr<int>(new int(_counter++));
        }
        throw job_scheduler::ExpiredException();  // Important: Generator expired
    }

private:
    int _counter;
    int _max_value;
};


/** Thread safe cout class
  * Exemple of use:
  *    PrintThread{} << "Hello world!" << std::endl;
  */
class PrintThread: public std::ostringstream
{
public:
    PrintThread() = default;

    ~PrintThread()
    {
        std::lock_guard<std::mutex> guard(_mutexPrint);
        std::cout << this->str();
    }

private:
    static std::mutex _mutexPrint;
};

std::mutex PrintThread::_mutexPrint{};


#endif
