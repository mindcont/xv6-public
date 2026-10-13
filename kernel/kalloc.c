// xv6 中文导读
// 文件：kernel/kalloc.c
// 文件作用：实现物理内存页的分配和释放
// 所属阶段：物理内存阶段
//
// Physical memory allocator, intended to allocate
// 物理内存分配器，用于分配
// memory for user processes, kernel stacks, page table pages,
// 用户进程、内核栈和页表页面的内存，
// and pipe buffers. Allocates 4096-byte pages.
// 以及管道缓冲区；每次分配 4096 字节的页面。

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file；ELF 加载内核后的第一个地址
                   // defined by the kernel linker script in kernel.ld；由 kernel.ld 链接脚本定义

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Initialization happens in two phases.
// 初始化分为两个阶段。
// 1. main() calls kinit1() while still using entrypgdir to place just
// 1. main() 仍使用 entrypgdir 时调用 kinit1()，只把
// the pages mapped by entrypgdir on free list.
// entrypgdir 映射的页面加入空闲链表。
// 2. main() calls kinit2() with the rest of the physical pages
// 2. 建立映射所有 CPU 的完整页表后，main() 调用 kinit2()，
// after installing a full page table that maps them on all cores.
// 把其余物理页面加入分配器。
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// 释放 v 指向的物理内存页面，
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// 分配一个 4096 字节的物理内存页面。
// Returns a pointer that the kernel can use.
// 返回一个内核可以使用的指针。
// Returns 0 if the memory cannot be allocated.
// 如果无法分配内存，则返回 0。
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

