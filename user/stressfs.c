// xv6 中文导读
// 文件：user/stressfs.c
// 文件作用：user/stressfs.c 的声明、数据结构或实现代码
// 所属阶段：用户程序阶段
//
// Demonstrate that moving the "acquire" in iderw after the loop that
// 演示如果把 iderw 中的 acquire 移到
// appends to the idequeue results in a race.
// 向 idequeue 追加请求的循环之后，会产生竞态条件。

// For this to work, you should also add a spin within iderw's
// 若要观察该问题，还应在 iderw 的
// idequeue traversal loop.  Adding the following demonstrated a panic
// 遍历 idequeue 的循环中加入自旋；下面的代码曾导致
// after about 5 runs of stressfs in QEMU on a 2.1GHz CPU:
// 在 2.1 GHz CPU 的 QEMU 中运行 stressfs 约 5 次后触发 panic：
//    for (i = 0; i < 40000; i++)
//      asm volatile("");

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
  int fd, i;
  char path[] = "stressfs0";
  char data[512];

  printf(1, "stressfs starting\n");
  memset(data, 'a', sizeof(data));

  for(i = 0; i < 4; i++)
    if(fork() > 0)
      break;

  printf(1, "write %d\n", i);

  path[8] += i;
  fd = open(path, O_CREATE | O_RDWR);
  for(i = 0; i < 20; i++)
//    printf(fd, "%d\n", i);
    write(fd, data, sizeof(data));
  close(fd);

  printf(1, "read\n");

  fd = open(path, O_RDONLY);
  for (i = 0; i < 20; i++)
    read(fd, data, sizeof(data));
  close(fd);

  wait();

  exit();
}
