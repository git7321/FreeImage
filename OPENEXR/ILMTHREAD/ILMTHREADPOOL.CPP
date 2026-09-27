///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005-2012, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
//	class Task, class ThreadPool, class TaskGroup
//
//-----------------------------------------------------------------------------

#include "IlmThread.h"
#include "IlmThreadMutex.h"
#include "IlmThreadSemaphore.h"
#include "IlmThreadPool.h"
#include "Iex.h"
#include <list>

using namespace std;

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_ENTER
namespace {

class WorkerThread: public Thread
{
  public:

    WorkerThread (ThreadPool::Data* data);

    virtual void	run ();
    
  private:

    ThreadPool::Data *	_data;
};

}

struct TaskGroup::Data
{
     Data ();
    ~Data ();
    
    void	addTask () ;
    void	removeTask ();
    
    Semaphore	isEmpty;        // used to signal that the taskgroup is empty
    int         numPending;     // number of pending tasks to still execute
    Mutex       dtorMutex;      // used to work around the glibc bug:
                                // http://sources.redhat.com/bugzilla/show_bug.cgi?id=12674
};

struct ThreadPool::Data
{
     Data ();
    ~Data();
    
    void	finish ();
    bool	stopped () const;
    void	stop ();

    Semaphore taskSemaphore;        // threads wait on this for ready tasks
    Mutex taskMutex;                // mutual exclusion for the tasks list
    list<Task*> tasks;              // the list of tasks to execute
    size_t numTasks;                // fast access to list size
                                    //   (list::size() can be O(n))

    Semaphore threadSemaphore;      // signaled when a thread starts executing
    Mutex threadMutex;              // mutual exclusion for threads list
    list<WorkerThread*> threads;    // the list of all threads
    size_t numThreads;              // fast access to list size
    
    bool stopping;                  // flag indicating whether to stop threads
    Mutex stopMutex;                // mutual exclusion for stopping flag
};

WorkerThread::WorkerThread (ThreadPool::Data* data):
    _data (data)
{
    start();
}

void
WorkerThread::run ()
{
    _data->threadSemaphore.post();

    while (true)
    {
        _data->taskSemaphore.wait();

        {
            Lock taskLock (_data->taskMutex);

            if (_data->numTasks > 0)
            {
                Task* task = _data->tasks.front();
		TaskGroup* taskGroup = task->group();
                _data->tasks.pop_front();
                _data->numTasks--;

                taskLock.release();
                task->execute();
                taskLock.acquire();

                delete task;
                taskGroup->_data->removeTask();
            }
            else if (_data->stopped())
	    {
                break;
	    }
        }
    }
}

TaskGroup::Data::Data (): isEmpty (1), numPending (0)
{
}

TaskGroup::Data::~Data ()
{
    isEmpty.wait ();

    Lock lock (dtorMutex);
}

void
TaskGroup::Data::addTask () 
{
    if (numPending++ == 0)
	isEmpty.wait ();
}

void
TaskGroup::Data::removeTask ()
{
    if (--numPending == 0)
    {
        Lock lock (dtorMutex);
        isEmpty.post ();
    }
}

ThreadPool::Data::Data (): numTasks (0), numThreads (0), stopping (false)
{
}

ThreadPool::Data::~Data()
{
    Lock lock (threadMutex);
    finish ();
}

void
ThreadPool::Data::finish ()
{
    stop();

    for (size_t i = 0; i < numThreads; i++)
    {
	taskSemaphore.post();
	threadSemaphore.wait();
    }

    for (list<WorkerThread*>::iterator i = threads.begin();
	 i != threads.end();
	 ++i)
    {
	delete (*i);
    }

    Lock lock1 (taskMutex);
    Lock lock2 (stopMutex);
    threads.clear();
    tasks.clear();
    numThreads = 0;
    numTasks = 0;
    stopping = false;
}

bool
ThreadPool::Data::stopped () const
{
    Lock lock (stopMutex);
    return stopping;
}

void
ThreadPool::Data::stop ()
{
    Lock lock (stopMutex);
    stopping = true;
}

Task::Task (TaskGroup* g): _group(g)
{
}

Task::~Task()
{
}

TaskGroup*
Task::group ()
{
    return _group;
}

TaskGroup::TaskGroup ():
    _data (new Data())
{
}

TaskGroup::~TaskGroup ()
{
    delete _data;
}

ThreadPool::ThreadPool (unsigned nthreads):
    _data (new Data())
{
    setNumThreads (nthreads);
}

ThreadPool::~ThreadPool ()
{
    delete _data;
}

int
ThreadPool::numThreads () const
{
    Lock lock (_data->threadMutex);
    return _data->numThreads;
}

void
ThreadPool::setNumThreads (int count)
{
    Lock lock (_data->threadMutex);

    if ((size_t)count > _data->numThreads)
    {

        while (_data->numThreads < (size_t)count)
        {
            _data->threads.push_back (new WorkerThread (_data));
            _data->numThreads++;
        }
    }
    else if ((size_t)count < _data->numThreads)
    {
        _data->finish ();

        while (_data->numThreads < (size_t)count)
        {
            _data->threads.push_back (new WorkerThread (_data));
            _data->numThreads++;
        }
    }
}

void
ThreadPool::addTask (Task* task) 
{
    Lock lock (_data->threadMutex);

    if (_data->numThreads == 0)
    {
        task->execute ();
        delete task;
    }
    else
    {
        {
            Lock taskLock (_data->taskMutex);

            _data->tasks.push_back (task);
            _data->numTasks++;
            task->group()->_data->addTask();
        }
        
        _data->taskSemaphore.post ();
    }
}

ThreadPool&
ThreadPool::globalThreadPool ()
{
    static ThreadPool gThreadPool (0);

    return gThreadPool;
}

void
ThreadPool::addGlobalTask (Task* task)
{
    globalThreadPool().addTask (task);
}

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_EXIT
