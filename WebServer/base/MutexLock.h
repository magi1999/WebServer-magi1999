#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

class MutexLock:noncopyable 
{
    public:
        MutexLock() { pthread_mutex_init(&mutex, NULL); }
        /*PTHREAD_MUTEX_NORMAL 不会检测死锁。重复锁定该互斥锁，则会产生死锁。
        解除由其他线程锁定的互斥锁会产生不确定的行为。重复解锁该互斥锁，则会产生不确定的行为。
        PTHREAD_MUTEX_ERRORCHECK 可提供错误检查。重复锁定互斥锁，则会返回错误。
        解除锁定的互斥锁已经由其他线程锁定，则会返回错误。重复解锁互斥锁，则会返回错误。
        PTHREAD_MUTEX_RECURSIVE 可重复加锁。多次锁定互斥锁需要进行相同次数的解除锁定才可以释放该锁。
        解除锁定的互斥锁已经由其他线程锁定，则会返回错误。重复解锁互斥锁，则会返回错误。
        PTHREAD_MUTEX_DEFAULT 一般情况下=PTHREAD_MUTEX_NORMAL*/
        ~MutexLock() 
        {
            pthread_mutex_lock(&mutex);
            //先拿到mutex，防止还有正在使用mutex的线程
            pthread_mutex_destroy(&mutex);
        }
        void lock() { pthread_mutex_lock(&mutex); }
        void unlock() { pthread_mutex_unlock(&mutex); }
        pthread_mutex_t *get() { return &mutex; }

    private:
        pthread_mutex_t mutex;

    friend class Condition;
};

//作用域后自动解锁
class MutexLockGuard : noncopyable 
{
  public:
      explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) { mutex.lock(); }
      ~MutexLockGuard() { mutex.unlock(); }

  private:
      MutexLock &mutex;
};