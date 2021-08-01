#include "Thread.h"
#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "CurrentThread.h"
#include <iostream>
using namespace std;

namespace CurrentThread 
{
  __thread int t_cachedTid = 0;
  __thread char t_tidString[32];
  __thread int t_tidStringLength = 6;
  __thread const char* t_threadName = "default";
}

pid_t gettid() 
{ 
    return static_cast<pid_t>(::syscall(SYS_gettid)); 
    //SYS_gettid 得到进程真实ID pthread_create返回的不是真实进程ID
    //pthread_t只在进程内唯一
}

void CurrentThread::cacheTid() 
{
    if (t_cachedTid == 0) 
    {
        t_cachedTid = gettid();
        t_tidStringLength =snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
        //将t_cachedTid以"%5d "的格式写入t_tidString，最大长度为sizeof t_tidString
    }
}

struct ThreadData 
{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(const ThreadFunc& func,const string& name,pid_t* tid,CountDownLatch* latch)
    :func_(func),name_(name),tid_(tid),latch_(latch) {}

    void runInThread() 
    {
        *tid_ = CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();//告知runInThread()已经启动
        latch_ = NULL;
        CurrentThread::t_threadName = name_.empty()?"Thread":name_.c_str();
        prctl(PR_SET_NAME, CurrentThread::t_threadName);
        //设置进程名字
        func_();
        CurrentThread::t_threadName = "finished";
    }
};

void* startThread(void* obj) 
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    //回收指针资源
    return NULL;
}

Thread::Thread(const ThreadFunc& func,const string& n):started_(false),joined_(false),pthreadId_(0),
tid_(0),func_(func),name_(n),latch_(1) 
{
    setDefaultName();
}

Thread::~Thread() 
{
  if (started_ && !joined_) pthread_detach(pthreadId_);
  //线程分离自己回收资源
}

void Thread::setDefaultName() 
{
    if (name_.empty()) 
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread");
        name_ = buf;
    }
}

void Thread::start() 
{
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
    //将线程信息存入ThreadData并传给pthread_create
    if (pthread_create(&pthreadId_, NULL, &startThread, data)) 
    {
        started_ = false;
        delete data;
    } 
    else 
    {
        latch_.wait();
        //等待runInThread()真正开始才结束start()
        assert(tid_ > 0);
    }
}

int Thread::join() 
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
    //等待创建的线程结束
}