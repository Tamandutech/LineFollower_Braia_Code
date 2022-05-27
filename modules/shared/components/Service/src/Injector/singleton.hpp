#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include "thread.hpp"
#include <atomic>
#include <mutex>

 template<class T> class Singleton
 {
     public:
        static  T *getInstance(std::string _name = "Thread", uint32_t _stackDepth = 1000, UBaseType_t _priority = 0)
                {
                        T *sin = instance.load(std::memory_order_acquire);
                        if (!sin)
                        {
                                std::lock_guard<std::mutex> myLock(instanceMutex);
                                sin = instance.load(std::memory_order_relaxed);
                                if (!sin)
                                {
                                        sin = new T(_name,_stackDepth,_priority);
                                        instance.store(sin, std::memory_order_release);
                                }
                        }

                        return sin;
                }
    private:
        static std::atomic<T *> instance;
        static std::mutex instanceMutex;


 };

template<class T> std::atomic<T*> Singleton<T>::instance;
template<class T> std::mutex Singleton<T>::instanceMutex;

#endif