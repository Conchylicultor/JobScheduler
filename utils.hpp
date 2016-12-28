#ifndef DEMO_UTILS_H  // Reduce colisions risk with prefix demo_
#define DEMO_UTILS_H


/** Convinience class to log the variadic arguments flow
  * Only used for debugging
  */
class ArgumentLogger
{
public:
    ArgumentLogger()
    {
        std::cout << "Arg " << this << ": creation " << std::endl;
    }
    ArgumentLogger(const ArgumentLogger& other)
    {
        std::cout << "Arg " << this << ": creation by const copy of " << &other << std::endl;
    }
    ArgumentLogger(ArgumentLogger& other)
    {
        std::cout << "Arg " << this << ": creation by copy of " << &other << std::endl;
    }
    ArgumentLogger(ArgumentLogger&& other)
    {
        std::cout << "Arg " << this << ": creation by move of" << &other << std::endl;
    }
    ArgumentLogger& operator= (const ArgumentLogger& other)
    {
        std::cout << "Arg " << this << ": assignement by " << &other << std::endl;
        return *this;
    }
    ArgumentLogger& operator= (ArgumentLogger&& other)
    {
        std::cout << "Arg " << this << ": move assignement by " << &other << std::endl;
        return *this;
    }
    ~ArgumentLogger()
    {
        std::cout << "Arg " << this << ": destruction" << std::endl;
    }
    void print() const
    {
        std::cout << "Arg " << this << ": printing" << std::endl;
    }
};


#endif
