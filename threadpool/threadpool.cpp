#include "threadpool.hpp"

/**
 * This file contains implentations of the functions defined in threadpool.hpp
 * 
 * For Licensing information see the bottom of the file
*/


Threadpool::Threadpool(uint num_threads)
: threads(), jobs(), status(_Threadpool_Status::IDLE), idle_waiting(false),
working_threads(0)
{
    sem_init(&this->worker_semaphore, 0, 0);
    sem_init(&this->wait_for_idle_semaphore, 0, 0);
    // we do not need to take the jobs_mutex because all workers will block 
    // on the worker_semaphore until work is added
    threads.reserve(num_threads);
    for (uint i = 0; i < num_threads; ++i)
        threads.emplace_back(&Threadpool::_run_worker, this);
}

void Threadpool::add_work(std::function<void()> &&work)
{
    const std::lock_guard guard(jobs_mutex);
    jobs.emplace_back(work);
    if(status == _Threadpool_Status::IDLE)
    {
        status = _Threadpool_Status::RUNNING;
        sem_post(&worker_semaphore);
    }
}

void Threadpool::_run_worker()
{
    while(true)
    {
        sem_wait(&worker_semaphore);
        bool first_job = true;
        while(true)
        {
            std::function<void()> unit_of_work;
            {
                const std::lock_guard guard(jobs_mutex);
                if(status == _Threadpool_Status::CLOSING)
                {
                    sem_post(&worker_semaphore);
                    return;
                }

                if (jobs.size() > 0) {
                    unit_of_work = jobs.front();
                    jobs.pop_front();
                    if(first_job) {
                        ++working_threads;
                        first_job = false;
                    }
                    if(jobs.size() > 0 && working_threads < threads.size())
                        sem_post(&worker_semaphore);
                }
                else {
                    if(!first_job && --working_threads == 0) {
                        status = _Threadpool_Status::IDLE;
                        if(idle_waiting) {
                            idle_waiting = false;
                            sem_post(&wait_for_idle_semaphore);
                        }
                    }
                    break;
                }
            }
            unit_of_work();
        }
    }
}

void Threadpool::wait_for_idle()
{
    {
        const std::lock_guard guard(jobs_mutex);
        if(status == _Threadpool_Status::IDLE)
            return;
        idle_waiting = true;
    }
    sem_wait(&wait_for_idle_semaphore);
    if(status != _Threadpool_Status::IDLE)
        abort();
}

Threadpool::~Threadpool()
{
    this->wait_for_idle();
    {
        const std::lock_guard guard(jobs_mutex);
        status = _Threadpool_Status::CLOSING;
    }
    sem_post(&worker_semaphore);
    for (std::thread &thread: threads)
        thread.join();
    sem_destroy(&this->worker_semaphore);
    sem_destroy(&this->wait_for_idle_semaphore);
}

/*
Copyright 2020 Felix Giese

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
