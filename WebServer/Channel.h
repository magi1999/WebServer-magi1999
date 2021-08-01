#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"
#include"base/Logging.h"

class EventLoop;
class HttpData;

//事件属性
class Channel 
{
  private:
        typedef std::function<void()> CallBack;
        EventLoop *loop_;
        int fd_;
        __uint32_t events_;//事件类型
        __uint32_t revents_;//回调事件类型
        __uint32_t lastEvents_;//减少修改次数
        // 方便找到上层持有该Channel的对象
        std::weak_ptr<HttpData> holder_;
  private:
        CallBack readHandler_;
        CallBack writeHandler_;
        CallBack errorHandler_;
        CallBack connHandler_;

  public:
      Channel(EventLoop *loop);
      Channel(EventLoop *loop, int fd);
      ~Channel();
      int getFd();
      void setFd(int fd);
      void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }
      std::shared_ptr<HttpData> getHolder() 
      {
          std::shared_ptr<HttpData> ret(holder_.lock());
          return ret;
      }
      void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
      void setWriteHandler(CallBack &&writeHandler) { writeHandler_ = writeHandler; }
      void setErrorHandler(CallBack &&errorHandler) { errorHandler_ = errorHandler; }
      void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }

      void handleEvents() 
      {
          events_ = 0;
          //EPOLLHUP TCP连接被对方关闭
          if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) 
          {
              events_ = 0;
              return;
          }
          if (revents_ & EPOLLERR) 
          {
              handleError();
              events_ = 0;
              return;
          }
          if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) //有数据可读
          {
              handleRead();
          }
          if (revents_ & EPOLLOUT) //有数据可写
          {
              handleWrite();
          }
          handleConn();
      }
      void handleRead();
      void handleWrite();
      void handleError();
      void handleConn();
      void setRevents(__uint32_t ev) { revents_ = ev; }
      void setEvents(__uint32_t ev) { events_ = ev; }
      __uint32_t &getEvents() { return events_; }
      bool EqualAndUpdateLastEvents() 
      {
          bool ret = (lastEvents_ == events_);
          lastEvents_ = events_;
          return ret;
      }
      __uint32_t getLastEvents() { return lastEvents_; }
};

typedef std::shared_ptr<Channel> SP_Channel;