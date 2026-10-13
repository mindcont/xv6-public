// xv6 中文导读
// 文件：kernel/picirq.c
// 文件作用：kernel/picirq.c 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
#include "types.h"
#include "x86.h"
#include "traps.h"

// I/O Addresses of the two programmable interrupt controllers
// 两个可编程中断控制器的 I/O 地址。
#define IO_PIC1         0x20    // Master (IRQs 0-7)；主控制器
#define IO_PIC2         0xA0    // Slave (IRQs 8-15)；从控制器

// Don't use the 8259A interrupt controllers.  Xv6 assumes SMP hardware.
// 不使用 8259A 中断控制器；xv6 假设运行在 SMP 硬件上。
void
picinit(void)
{
  // mask all interrupts；屏蔽所有中断
  outb(IO_PIC1+1, 0xFF);
  outb(IO_PIC2+1, 0xFF);
}

//PAGEBREAK!
// Blank page.
