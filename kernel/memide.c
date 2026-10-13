// xv6 中文导读
// 文件：kernel/memide.c
// 文件作用：kernel/memide.c 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Fake IDE disk; stores blocks in memory.
// 虚拟 IDE 磁盘；把磁盘块存储在内存中。
// Useful for running kernel without scratch disk.
// 用于没有真实磁盘时运行内核。

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

extern uchar _binary_fs_img_start[], _binary_fs_img_size[];

static int disksize;
static uchar *memdisk;

void
ideinit(void)
{
  memdisk = _binary_fs_img_start;
  disksize = (uint)_binary_fs_img_size/BSIZE;
}

// Interrupt handler.
// 中断处理函数。
void
ideintr(void)
{
  // no-op；空操作
}

// Sync buf with disk.
// 同步 buf 与磁盘内容。
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// 如果设置 B_DIRTY，则写入磁盘、清除 B_DIRTY 并设置 B_VALID。
// Else if B_VALID is not set, read buf from disk, set B_VALID.
// 否则如果未设置 B_VALID，则从磁盘读取 buf 并设置 B_VALID。
void
iderw(struct buf *b)
{
  uchar *p;

  if(!holdingsleep(&b->lock))
    panic("iderw: buf not locked");
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  if(b->dev != 1)
    panic("iderw: request not for disk 1");
  if(b->blockno >= disksize)
    panic("iderw: block out of range");

  p = memdisk + b->blockno*BSIZE;

  if(b->flags & B_DIRTY){
    b->flags &= ~B_DIRTY;
    memmove(p, b->data, BSIZE);
  } else
    memmove(b->data, p, BSIZE);
  b->flags |= B_VALID;
}
