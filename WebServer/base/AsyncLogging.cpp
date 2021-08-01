#include "AsyncLogging.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <functional>
#include "LogFile.h"

AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval):flushInterval_(flushInterval),
running_(false),basename_(logFileName_),thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
mutex_(),cond_(mutex_),currentBuffer_(new Buffer),nextBuffer_(new Buffer),
buffers_(),latch_(1) 
{
    assert(logFileName_.size() > 1);
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}//创建currentBuffer_和nextBuffer_两个缓冲区

void AsyncLogging::append(const char* logline, int len) 
{
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len)
        currentBuffer_->append(logline, len);
    else 
    {
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();
        if (nextBuffer_)
        currentBuffer_ = std::move(nextBuffer_);
        else
        currentBuffer_.reset(new Buffer);
        currentBuffer_->append(logline, len);
        cond_.notify();
    }
}//当有一个缓冲区填满时，唤醒threadFunc()

void AsyncLogging::threadFunc() 
{
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_) 
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        {
            MutexLockGuard lock(mutex_);
            if (buffers_.empty())  //还没内容需要输出，等待flushInterval_秒
            {
                cond_.waitForSeconds(flushInterval_);
            }
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_) 
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25) 
        {
            buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
        }//日志线程太过繁忙，删除多余输出内容
        for (size_t i = 0; i < buffersToWrite.size(); ++i) 
        {
            output.append_unlocked(buffersToWrite[i]->data(), buffersToWrite[i]->length());
            //将buffersToWrite[i]->data()写入到实际文件中
        }
        if (buffersToWrite.size() > 2) 
        {
            //删除多余内容，保留2个作为常驻缓冲区
            buffersToWrite.resize(2);
        }
        if (!newBuffer1) 
        {
            //重复使用buffersToWrite中的shared_ptr指针，减少重复申请空间的开销
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if (!newBuffer2) 
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();//每次写完完整的log直接输出到文件
    }
    output.flush();
}
