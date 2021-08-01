#pragma once
//私有拷贝构造函数和重载赋值运算符 -> 无法复制
class noncopyable 
{
  protected:
      noncopyable() {}
      ~noncopyable() {}
  private:
      noncopyable(const noncopyable&);
      const noncopyable& operator=(const noncopyable&);
};