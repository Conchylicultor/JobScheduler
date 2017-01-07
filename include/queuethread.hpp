#ifndef JS_QUEUETHREAD_H
#define JS_QUEUETHREAD_H

#include <list>
#include <condition_variable>
#include <mutex>


namespace job_scheduler
{


/** Thread safe queue implementation.
  * Push and pop can be called from any thread
  * The pop call is blocking while the queue is empty. WARNING: It should only
  * be called by a single thread if you want to be sure the data are poped
  * sequencially !
  * This class is used internally by the QueueScheduler
  */
template <typename T>
class QueueThread
{
public:
    QueueThread();
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
    std::mutex _mutexQueue;  // Prevent concurent calls to the queue (access or update)
    std::condition_variable _cvEmpty;  // Lock the calls when one of the queue is empty

    std::list<T> _queue;
};

template <typename T>
QueueThread<T>::QueueThread() :
    _mutexQueue(),
    _cvEmpty()
{
}


template <typename T>
void QueueThread<T>::push_back(const T& elem)
{
    std::lock_guard<std::mutex> guard(_mutexQueue);  // Push and pop are executed sequencially

    _queue.push_back(elem);

    _cvEmpty.notify_one();  // Eventually unlock pop_front
}


template <typename T>
void QueueThread<T>::push_back(T&& elem)
{
    std::lock_guard<std::mutex> guard(_mutexQueue);  // Push and pop are executed sequencially

    _queue.push_back(std::move(elem));

    _cvEmpty.notify_one();  // Eventually unlock pop_front
}


template <typename T>
T QueueThread<T>::pop_front()
{
    std::unique_lock<std::mutex> guard(_mutexQueue);
    _cvEmpty.wait(guard, [this]{ return this->_queue.size() > 0; });  // Wait for the queue to be filled

    T elem = std::move(_queue.front());  // If we are here, we are sure that at least one element has been pushed (TODO: Is the move call safe ?)
    _queue.pop_front();

    return elem;
}


template <typename T>
const std::list<T>& QueueThread<T>::get_data()
{
    return _queue;
}


}

#endif
