// xv6 中文导读
// 文件：kernel/bio.c
// 文件作用：实现磁盘块缓存和块级并发访问
// 所属阶段：文件系统阶段
//
// Buffer cache. 块缓存。
//
// The buffer cache is a linked list of buf structures holding
// 块缓存是一个 buf 结构链表，保存
// cached copies of disk block contents.  Caching disk blocks
// 磁盘块内容的缓存副本。把磁盘块缓存到内存中
// in memory reduces the number of disk reads and also provides
// 可以减少磁盘读取次数，并为多个进程使用的磁盘块提供
// a synchronization point for disk blocks used by multiple processes.
// 一个同步点。
//
// Interface:
// 接口约定：
// * To get a buffer for a particular disk block, call bread.
// * 获取指定磁盘块的缓存，调用 bread。
// * After changing buffer data, call bwrite to write it to disk.
// * 修改缓存数据后，调用 bwrite 写回磁盘。
// * When done with the buffer, call brelse.
// * 使用完缓存后，调用 brelse。
// * Do not use the buffer after calling brelse.
// * 调用 brelse 后不能再使用该缓存。
// * Only one process at a time can use a buffer,
// * 一个缓存同时只能被一个进程使用，
//     so do not keep them longer than necessary.
// * 因此不要超过必要时间持有它。
//
// The implementation uses two state flags internally:
// 实现内部使用两个状态标志：
// * B_VALID: the buffer data has been read from the disk.
// * B_VALID：缓存数据已经从磁盘读入。
// * B_DIRTY: the buffer data has been modified
// * B_DIRTY：缓存数据已被修改，
//     and needs to be written to disk.
// * 需要写回磁盘。

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // 通过 prev/next 连接所有缓存。
  // head.next is most recently used.
  // head.next 指向最近使用的缓存。
  struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

//PAGEBREAK!
  // Create linked list of buffers
  // 创建缓存链表。
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// 在设备 dev 的块缓存中查找指定块。
// If not found, allocate a buffer.
// 如果找不到，则分配一个缓存。
// In either case, return locked buffer.
// 无论哪种情况都返回已经加锁的缓存。
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache.lock);

  // Is the block already cached? 该块是否已经缓存？
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer. 未缓存，回收一个未使用的缓存。
  // Even if refcnt==0, B_DIRTY indicates a buffer is in use
  // because log.c has modified it but not yet committed it.
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
    if(b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->flags = 0;
      b->refcnt = 1;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

  // Return a locked buf with the contents of the indicated block.
  // 返回包含指定磁盘块内容且已加锁的 buf。
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if((b->flags & B_VALID) == 0) {
    iderw(b);
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
// 将 b 的内容写入磁盘；调用者必须持有该缓存锁。
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  b->flags |= B_DIRTY;
  iderw(b);
}

// Release a locked buffer.
// 释放已加锁的缓存。
// Move to the head of the MRU list.
// 将它移动到最近使用链表的头部。
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  release(&bcache.lock);
}
//PAGEBREAK!
// Blank page.

