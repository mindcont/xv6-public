// xv6 中文导读
// 文件：user/zombie.c
// 文件作用：user/zombie.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
// Create a zombie process that
// 创建一个
// must be reparented at exit.
// 退出时必须被重新托管给 init 的僵尸进程。

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  if(fork() > 0)
    sleep(5);  // Let child exit before parent；让子进程先于父进程退出
  exit();
}
