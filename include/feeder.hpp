#ifndef JS_CONCH_FEEDER_H
#define JS_CONCH_FEEDER_H

//#include <memory>
//#include <functional>
#include <exception>


namespace js_conch
{

/** The feeder sequencially generate the next output
  * for the workers. When the generator expire, it
  * must throw an ExpiredException
  */
template <typename Input>
class Feeder
{
public:
    Feeder(std::function<Input()> generator);
    Feeder(const Feeder&) = delete;
    Feeder& operator= (const Feeder&) = delete;
    ~Feeder() = default;

    Input getNext(); // throw(ExpiredException)

private:
    std::function<Input()> _generator;
};


// TODO: Try to encapsulate tht class inside Feeder without having to declare
// the template Feeder<void>::ExpiredException
class ExpiredException : public std::exception
{
};


template <typename Input>
Feeder<Input>::Feeder(std::function<Input()> generator) :
    _generator(generator)
{
}


template <typename Input>
Input Feeder<Input>::getNext()
{
    return _generator();
}


} // End namespace


#endif