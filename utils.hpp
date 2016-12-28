#ifndef DEMO_UTILS_H  // Reduce colisions risk with prefix demo_
#define DEMO_UTILS_H


#include <iostream>


class ArgumentLogger;
std::ostream& operator<<(std::ostream& os, const ArgumentLogger& obj);

/** Convinience class to log the variadic arguments flow
  * Only used for debugging
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

#endif
