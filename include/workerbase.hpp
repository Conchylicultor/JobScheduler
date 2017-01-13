#ifndef JS_WORKERBASE_H
#define JS_WORKERBASE_H


#include <memory>


namespace job_scheduler
{


template <typename Input, typename Output>
class WorkerBase
{
public:
    using input_type = Input;
    using output_type = Output;

    WorkerBase(int id) : m_worker_id(id) {};
    WorkerBase(const WorkerBase&) = delete;
    WorkerBase& operator=(const WorkerBase&) = delete;
    virtual ~WorkerBase() = default;

    virtual std::unique_ptr<Output> operator() (const Input& input) = 0;

protected:
    int m_worker_id;
};



} // End namespace

#endif
