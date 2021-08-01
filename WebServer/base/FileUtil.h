#pragma once
#include <string>
#include "noncopyable.h"

//向文件中写入数据，线程不安全
class FileUtil : noncopyable 
{
  public:
      explicit FileUtil(std::string filename);
      ~FileUtil();
      void append(const char *logline, const size_t len);
      void flush();

  private:
      size_t write(const char *logline, size_t len);
      FILE *fp_;
      char buffer_[64 * 1024];//用户提供的缓冲区
};