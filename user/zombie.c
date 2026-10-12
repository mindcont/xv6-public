// xv6 中文导读
// 文件：user/zombie.c
// 文件作用：user/zombie.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
// Create a zombie process that
// must be reparented at exit.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  if(fork() > 0)
    sleep(5);  // Let child exit before parent.
  exit();
}
