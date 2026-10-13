// xv6 中文导读
// 文件：kernel/uart.c
// 文件作用：实现 UART 串口初始化、发送和接收
// 所属阶段：设备驱动阶段
//
// Intel 8250 serial port (UART).
// Intel 8250 串口（UART）。

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define COM1    0x3f8

static int uart;    // is there a uart?；是否存在串口

void
uartinit(void)
{
  char *p;

  // Turn off the FIFO；关闭 FIFO
  outb(COM1+2, 0);

  // 9600 baud, 8 data bits, 1 stop bit, parity off.
  // 9600 波特率、8 个数据位、1 个停止位、不使用奇偶校验。
  outb(COM1+3, 0x80);    // Unlock divisor；解锁分频器
  outb(COM1+0, 115200/9600);
  outb(COM1+1, 0);
  outb(COM1+3, 0x03);    // Lock divisor, 8 data bits；锁定分频器，8 个数据位
  outb(COM1+4, 0);
  outb(COM1+1, 0x01);    // Enable receive interrupts；开启接收中断

  // If status is 0xFF, no serial port.
  // 如果状态为 0xFF，表示不存在串口。
  if(inb(COM1+5) == 0xFF)
    return;
  uart = 1;

  // Acknowledge pre-existing interrupt conditions;
  // enable interrupts.
  // 确认并清除已有的中断状态，然后开启中断。
  inb(COM1+2);
  inb(COM1+0);
  ioapicenable(IRQ_COM1, 0);

  // Announce that we're here. 宣布串口驱动已经就绪。
  for(p="xv6...\n"; *p; p++)
    uartputc(*p);
}

void
uartputc(int c)
{
  int i;

  if(!uart)
    return;
  for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
    microdelay(10);
  outb(COM1+0, c);
}

static int
uartgetc(void)
{
  if(!uart)
    return -1;
  if(!(inb(COM1+5) & 0x01))
    return -1;
  return inb(COM1+0);
}

void
uartintr(void)
{
  consoleintr(uartgetc);
}
