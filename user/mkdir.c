// xv6 中文导读
// 文件：user/mkdir.c
// 文件作用：user/mkdir.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    printf(2, "Usage: mkdir files...\n");
    exit();
  }

  for(i = 1; i < argc; i++){
    if(mkdir(argv[i]) < 0){
      printf(2, "mkdir: %s failed to create\n", argv[i]);
      break;
    }
  }

  exit();
}
