#ifndef JS_QUEUETHREAD_H
#define JS_QUEUETHREAD_H

#include <list>
#include <condition_variable>
#include <mutex>


namespace job_scheduler
{

// Value for which the queue don't have a maximum size. Need to be zero (otherwise, 0 would be a blocking queue)
constexpr size_t UNLIMITED = 0;


/** Thread safe queue implementation.
  * Push and pop can be called from any thread
  * The pop call is blocking while the queue is empty. WARNING: It should only
  * be called by a single thread if you want to be sure the data are poped
  * sequencially !
  * The push call is blocking if the maxSize parameter has been set. The maxSize
  * parameter control the maximum size for the queue.
  * This class is used internally by the QueueScheduler
  */
template <typename T>
class QueueThread
{
public:
    QueueThread(size_t maxSize = UNLIMITED);
    QueueThread(const QueueThread&) = delete;
    QueueThread& operator=(const QueueThread&) = delete;
    ~QueueThread() = default;

    void push_back(const T& elem);
    void push_back(T&& elem);

    T pop_front();

    // WARNING: Not thread safe. Just a convinience method. Be also careful
    // to not access the returned reference after QueueThread is destructed
    const std::list<T>& get_data();

private:
    bool is_not_full();  // Used by the condition variable

    std::mutex _mutexQueue;  // Prevent concurent calls to the queue (access or update)
    std::condition_variable _cvEmpty;  // Lock the pop calls when one of the queue is empty
    std::condition_variable _cvFull;  // Lock the push calls when one of the queue is full

    size_t _maxSize;  // Max size of the queue

    std::list<T> _queue;
};

template <typename T>
QueueThread<T>::QueueThread(size_t maxSize) :
    _mutexQueue(),
    _cvEmpty(),
    _cvFull(),
    _maxSize(maxSize),
    _queue()
{
}


template <typename T>
void QueueThread<T>::push_back(const T& elem)
{
    std::unique_lock<std::mutex> guard(_mutexQueue);  // Push and pop are executed sequencially
    _cvFull.wait(guard, std::bind(&QueueThread<T>::is_not_full, this));

    _queue.push_back(elem);

    _cvEmpty.notify_one();  // Eventually unlock pop_front
}


template <typename T>
void QueueThread<T>::push_back(T&& elem)
{
    std::unique_lock<std::mutex> guard(_mutexQueue);
    _cvFull.wait(guard, std::bind(&QueueThread<T>::is_not_full, this));

    _queue.push_back(std::move(elem));

    _cvEmpty.notify_one();
}


template <typename T>
T QueueThread<T>::pop_front()
{
    std::unique_lock<std::mutex> guard(_mutexQueue);
    _cvEmpty.wait(guard, [this]{ return this->_queue.size() > 0; });  // Wait for the queue to be filled

    T elem = std::move(_queue.front());  // If we are here, we are sure that at least one element has been pushed (TODO: Is the move call safe ?)
    _queue.pop_front();

    _cvFull.notify_one();  // Eventually unlock push_back

    return elem;
}


template <typename T>
const std::list<T>& QueueThread<T>::get_data()
{
    return _queue;
}


template <typename T>
bool QueueThread<T>::is_not_full()
{
    if (_maxSize == UNLIMITED)
    {
        return true;  // Queue never full
    }
    return this->_queue.size() < _maxSize;
}


}

#endif
