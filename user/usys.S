// xv6 中文导读
// 文件：user/usys.S
// 文件作用：提供用户态系统调用汇编存根，将请求转入内核
// 所属阶段：系统调用入口阶段
//
#include "syscall.h"
#include "traps.h"

// 每个宏实例生成一个用户态系统调用存根：把系统调用号放入 eax，
// 通过 int 指令进入内核，返回时继续执行用户程序。

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
