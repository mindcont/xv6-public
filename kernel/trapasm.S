// xv6 中文导读
// 文件：kernel/trapasm.S
// 文件作用：保存和恢复陷阱现场，并连接汇编中断入口与 C 处理函数
// 所属阶段：中断入口阶段
//
#include "mmu.h"

  # vectors.S sends all traps here.
  # vectors.S 将所有陷阱发送到这里。
.globl alltraps
alltraps:
  # Build trap frame.
  # 构造陷阱帧。
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data segments.
  # 设置数据段寄存器。
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  # 调用 trap(tf)，此时 tf=%esp。
  pushl %esp
  call trap
  addl $4, %esp

  # Return falls through to trapret...
  # 返回路径继续落入 trapret。
.globl trapret
trapret:
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret
