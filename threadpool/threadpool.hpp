#pragma once

/**
 * This header contains the type definition for a simple Work queue style Threadpool
 * 
 * For licensing information see the end of the file.
*/

#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <deque>
#include <semaphore.h>
#include <atomic>

enum _Threadpool_Status {
    IDLE,
    RUNNING,
    CLOSING,
};

class Threadpool{
public:
    Threadpool(Threadpool&) = delete;
    Threadpool operator=(Threadpool&) = delete;

    /**
     * starts a new threadpool with the specified number of threads
     */
    Threadpool(uint num_threads);

    Threadpool(): Threadpool(std::thread::hardware_concurrency()) {}

    /**
     * Adds the specified unit of work to the pool to be picked up by a worker thread
     */
    void add_work(std::function<void()> &&work);

    /**
     * Waits for the threadpool to enter idle status
     * This occurs when the job queue is drained completely
     * Blocks the calling thread
     */
    void wait_for_idle();

    /**
     * Destructor
     * Waits for the pool to idle, and joins all threads
     */
    ~Threadpool();
private:
    std::vector<std::thread> threads; 
    // this vector should not be modified while the threadpool is still active

    std::mutex jobs_mutex;
    std::deque<std::function<void()>> jobs; // protected by jobs_mutex
    _Threadpool_Status status; // protected by jobs_mutex
    bool idle_waiting; // protected by jobs_mutex
    uint working_threads; // protected by jobs_mutex

    sem_t worker_semaphore; // limits how many workers are active at the same time
    // to avoid a thundering herd when adding new jobs
    sem_t wait_for_idle_semaphore; // a sem for the thread wait_for_idle thread to
    // wait on. should only be notified while the jobs_mutex is held

    void _run_worker();
};

template <typename T>
class ThreadpoolResultCollector {
public:
    ThreadpoolResultCollector(ThreadpoolResultCollector&) = delete;
    ThreadpoolResultCollector operator=(ThreadpoolResultCollector&) = delete;

    /**
     * Constructs a new threadpool instance.
     * Requires the reference to remain valid for the lifetime of this object
     */
    ThreadpoolResultCollector(Threadpool& inner) : inner(inner){}

    void add_work(std::function<T()> func) {
        inner.add_work([&, func]() {
            T res = func();
            {
                const std::lock_guard guard(results_mutex);
                results.emplace_back(std::move(res));
            }
        });
    }

    std::vector<T> get_results() {
        inner.wait_for_idle();
        std::vector<T> rv;
        {
            const std::lock_guard guard(results_mutex);
            std::swap(rv, results);
            return rv;
        }
    }

    ~ThreadpoolResultCollector() {
        inner.wait_for_idle();
    }
private:
    std::mutex results_mutex;
    std::vector<T> results;
    Threadpool& inner;
};

/*
Copyright <YEAR> <COPYRIGHT HOLDER>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


