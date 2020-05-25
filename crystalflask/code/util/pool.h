#pragma once

//TODO:(Gabriel) DELETE EVERYTHING!

#include <future>
#include <queue>
#include <vector>
#include <map>

struct ThreadPool
{
    
    ThreadPool(int numThreads = std::thread::hardware_concurrency()) : Stop(false)
    {
        for (int i = 0; i < numThreads; i++)
        {
            Threads.emplace_back([this]()
                                 {
                                     while (true)
                                     {
                                         std::function<void()> task;
                                         {
                                             std::unique_lock<std::mutex> lock(Mutex);
                                             Condition.wait(lock, [this]()
                                                            {
                                                                return Stop || !Queue.empty();
                                                            });
                                             if (Stop && Queue.empty())
                                             {
                                                 return;
                                             }
                                             task = std::move(Queue.front());
                                             Queue.pop();
                                         }
                                         task();
                                     }
                                 });
        }
    }
    
    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> guard(Mutex);
            Stop = true;
        }
        Condition.notify_all();
        for (auto& thread : Threads)
        {
            thread.join();
        }
    }
    
    u64 NumThreads() const
    {
        return Threads.size();
    }
    
    template<class F, class... Args>
        auto Add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using ReturnType = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<ReturnType> result = task->get_future();
        {
            std::lock_guard<std::mutex> guard(Mutex);
            if (!Stop)
            {
                Queue.emplace([task]()
                              {
                                  (*task)();
                              });
            }
        }
        Condition.notify_one();
        return result;
    }
    
    std::vector<std::thread> Threads;
    std::queue<std::function<void()>> Queue;
    std::mutex Mutex;
    std::condition_variable Condition;
    bool Stop;
};


struct Executor
{
    Executor() : WorkerCount(pool.NumThreads()), SubmitCount(0) {}
    ~Executor() {}
    
    template<class F, class... Args>
        unsigned long long SubmitTask(F&& f, Args&&... args)
    {
        SubmitCount++;
        //results.push_back(pool.Add(f, args...));
        
        resultmap.insert(std::make_pair(SubmitCount, pool.Add(f, args...)));
        //results[CurrentWorkerIndex] = pool.Add(f, args...);
        return SubmitCount;
    }
    
    
    template<class F, class... Args>
        auto SubmitVoid(F&& f, Args&&... args)
    {
        //SubmitCount++;
        //results.push_back(pool.Add(f, args...));
        return pool.Add(f, args...);
        
        //resultmap.insert(std::make_pair(SubmitCount, pool.Add(f, args...)));
        //results[CurrentWorkerIndex] = pool.Add(f, args...);
        //return SubmitCount;
    }
    
    template<class F, class... Args>
        auto Submit(F&& f, Args&&... args)
    {
        auto Result = pool.Add(f, args...);
        return Result.get();
    }
    
    
    
    void Await(unsigned long long Index)
    {
        results[Index].get();
        //resultmap[hash].get();
    }
    
    void AwaitAll()
    {
        
        for (int Index = 0; Index < results.size(); Index++)
        {
            results[Index].get();
        }
        
        /*
        for (auto& res : resultmap)
        {
            res.second.get();
        }
        */
    }
    
    void TryAwait(unsigned long long hash)
    {
        try
        {
            Await(hash);
        } catch (std::exception )
        {
            
        }
    }
    
    void TryAwaitAll()
    {
        try
        {
            AwaitAll();
        } catch (std::exception )
        {
            
        }
    }
    
    ThreadPool pool;
    u64 WorkerCount;
    u64 SubmitCount;
    std::vector<std::future<void>> results;
    //std::map<unsigned long long, std::future<void>> resultmap;
};



