// xv6 中文导读
// 文件：boot/initcode.S
// 文件作用：第一个用户态程序，准备参数并通过 exec() 启动 /init
// 所属阶段：第一个用户进程阶段
//
# Initial process execs /init.
# 第一个进程通过 exec() 启动 /init；这段代码运行在用户态。

#include "syscall.h"
#include "traps.h"


# exec(init, argv)
.globl start
start:
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be；伪造调用者程序计数器的位置
  movl $SYS_exec, %eax
  int $T_SYSCALL

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $T_SYSCALL
  jmp exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0

