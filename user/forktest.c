// xv6 中文导读
// 文件：user/forktest.c
// 文件作用：user/forktest.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
// Test that fork fails gracefully.
// 测试 fork 失败时是否能够正常返回错误。
// Tiny executable so that the limit can be filling the proc table.
// 使用很小的可执行程序，便于快速填满进程表并触达进程数量限制。

#include "types.h"
#include "stat.h"
#include "user.h"

#define N  1000

void
printf(int fd, const char *s, ...)
{
  write(fd, s, strlen(s));
}

void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<N; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit();
  }

  if(n == N){
    printf(1, "fork claimed to work N times!\n", N);
    exit();
  }

  for(; n > 0; n--){
    if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }
  }

  if(wait() != -1){
    printf(1, "wait got too many\n");
    exit();
  }

  printf(1, "fork test OK\n");
}

int
main(void)
{
  forktest();
  exit();
}
