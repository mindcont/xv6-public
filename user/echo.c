// xv6 中文导读
// 文件：user/echo.c
// 文件作用：user/echo.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  for(i = 1; i < argc; i++)
    printf(1, "%s%s", argv[i], i+1 < argc ? " " : "\n");
  exit();
}
