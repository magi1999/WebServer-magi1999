#include "LogFile.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "FileUtil.h"


using namespace std;

LogFile::LogFile(const string& basename, int flushEveryN):basename_(basename),
flushEveryN_(flushEveryN),count_(0),mutex_(new MutexLock) {
    file_.reset(new FileUtil(basename));
}

LogFile::~LogFile() {}

//调用FileUtil的append 线程不安全 每flushEveryN刷新
void LogFile::append_unlocked(const char* logline, int len) 
{
    file_->append(logline, len);
    ++count_;
    if (count_>= flushEveryN_) 
    {
        count_ = 0;
        file_->flush();
    }
}

//线程安全
void LogFile::append(const char* logline, int len) 
{
  MutexLockGuard lock(*mutex_);
  append_unlocked(logline, len);
}

//flush属于写操作，需要加锁
void LogFile::flush() 
{
    MutexLockGuard lock(*mutex_);
    file_->flush();
}