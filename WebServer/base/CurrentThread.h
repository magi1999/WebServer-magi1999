#pragma once
#include <stdint.h>

namespace CurrentThread 
{
    extern __thread int t_cachedTid;//线程ID
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;
    void cacheTid();
    inline int tid() 
    {
        if (__builtin_expect(t_cachedTid == 0, 0)) //分支预测 t_cachedTId==0可能性小
        {
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString()  // tid字符串形式
    {
        return t_tidString;
    }

    inline int tidStringLength()  // tid字符串长度
    {
        return t_tidStringLength;
    }

    inline const char* name() { return t_threadName; }
}
