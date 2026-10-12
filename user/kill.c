// xv6 中文导读
// 文件：user/kill.c
// 文件作用：user/kill.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 2){
    printf(2, "usage: kill pid...\n");
    exit();
  }
  for(i=1; i<argc; i++)
    kill(atoi(argv[i]));
  exit();
}
