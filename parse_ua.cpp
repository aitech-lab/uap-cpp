#include <iostream>
#include <vector>
#include <chrono>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <string>

using namespace std;

class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> future<typename result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    vector< thread > workers;
    // the task queue
    queue< function<void()> > tasks;
    
    // synchronization
    mutex queue_mutex;
    condition_variable condition;
    bool stop;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    function<void()> task;

                    {
                        unique_lock<mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> future<typename result_of<F(Args...)>::type>
{
    using return_type = typename result_of<F(Args...)>::type;

    auto task = make_shared< packaged_task<return_type()> >(
            bind(forward<F>(f), forward<Args>(args)...)
        );
        
    future<return_type> res = task->get_future();
    {
        unique_lock<mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        unique_lock<mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(thread &worker: workers)
        worker.join();
}


#include "UaParser.h"
using namespace uap_cpp;
UserAgentParser p("regexes.yaml");

const size_t n = 100000;
vector<string> b(n);

int main(int argc, char** argv)
{
    int threads = 8;

    if (argc>=2) threads = stoi(argv[1]);
    
    ThreadPool pool(threads);
    
    while (true) {
        
        // Fill buffer
        size_t i=0; 
        for(;getline(cin, b[i]);) {
            i++;
            if(i==n) break;
        }
        // if no new lines - break while
        if(i==0) break; 

        // process buffer
        vector< future<UserAgent> > results;
        for(size_t j=0; j<i; ++j) {
            results.emplace_back(
                pool.enqueue([j] {
                    return p.parse(b[j]);
                })
            );
        };        
        i=0;
        
        // out results
        for(auto&& result: results) {
            UserAgent ua = result.get();
            cout
            // << b[i] << "\t"
            << ua.browser.family      << "\t"
            << ua.browser.major       << "\t"
            << ua.browser.minor       << "\t"
            << ua.browser.patch       << "\t"
            << ua.browser.patch_minor << "\t"
            << ua.os.family           << "\t"
            << ua.os.major            << "\t"
            << ua.os.minor            << "\t"
            << ua.os.patch            << "\t"
            << ua.os.patch_minor      << "\t"
            << ua.device.family       << "\t"
            << ua.device.brand        << "\t"
            << ua.device.model        << "\t"
            << "\n";
            i++;
        }
        i=0;
        
    }       
    return 0;
}
