#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

//a 若文件不存在，则会创建该文件；如果文件存在，则写入的数据会被加到文件尾后
//e Open the file with the O_CLOEXEC flag在子进程中关闭该文件描述符
FileUtil::FileUtil(string filename):fp_(fopen(filename.c_str(), "ae")) 
{
    // 用户提供缓冲区
    setbuffer(fp_, buffer_, sizeof buffer_);
}

FileUtil::~FileUtil() { fclose(fp_); }

size_t FileUtil::write(const char* logline, size_t len) 
{
    return fwrite_unlocked(logline, 1, len, fp_);//线程不安全版本
}

void FileUtil::flush() { fflush(fp_); }

void FileUtil::append(const char* logline, const size_t len) 
{
    size_t n = this->write(logline, len);
    size_t remain = len - n;
    while (remain > 0) 
    {
        size_t x = this->write(logline + n, remain);
        if (x == 0) 
        {
            int err = ferror(fp_);
            if (err) fprintf(stderr, "FileUtil::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
}
