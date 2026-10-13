// xv6 中文导读
// 文件：boot/bootmain.c
// 文件作用：读取并解析内核 ELF，将内核段加载到内存后跳转到内核入口
// 所属阶段：Bootloader 加载内核阶段
//
// Boot loader. 启动加载器。
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// 它与 bootasm.S 共同组成启动块，由 bootasm.S 调用 bootmain()。
// bootasm.S has put the processor into protected 32-bit mode.
// bootasm.S 已经把处理器切换到 32 位保护模式。
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.
// 从第 1 扇区开始读取 ELF 内核镜像，随后跳转到内核入口。

#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

void
// 中文说明：读取内核 ELF 并把各段加载到内存，完成 Bootloader 到内核的跳转。
bootmain(void)
{
  struct elfhdr *elf;
  struct proghdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  elf = (struct elfhdr*)0x10000;  // scratch space；临时工作区

  // Read 1st page off disk；从磁盘读取第一个页面
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?；检查是否为 ELF 可执行文件
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error；交由 bootasm.S 处理错误

  // Load each program segment (ignores ph flags).
  // 加载每个程序段（忽略 ph 中的权限标志）。
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    readseg(pa, ph->filesz, ph->off);
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }

  // Call the entry point from the ELF header.
  // Does not return!
  // 该调用不会返回！
  entry = (void(*)(void))(elf->entry);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready. 等待磁盘就绪。
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
// 将指定偏移处的一个扇区读取到 dst。
void
readsect(void *dst, uint offset)
{
  // Issue command. 向磁盘控制器发出读取命令。
  waitdisk();
  outb(0x1F2, 1);   // count = 1；读取 1 个扇区
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors；0x20 表示读取扇区

  // Read data. 读取扇区数据。
  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
// 从内核镜像偏移 offset 处读取 count 字节到物理地址 pa，实际可能多读一些。
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary. 向下对齐到扇区边界。
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  // 将字节偏移转换为扇区号；内核从第 1 扇区开始。
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // 如果逐扇区读取太慢，可以一次读取多个扇区。
  // We'd write more to memory than asked, but it doesn't matter --
  // 虽然会比请求多写入一些内存，但这没有影响，
  // we load in increasing order.
  // 因为加载地址始终递增。
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
