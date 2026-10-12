// xv6 中文导读
// 文件：kernel/buf.h
// 文件作用：kernel/buf.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
struct buf {
  int flags;
  uint dev;
  uint blockno;
  struct sleeplock lock;
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf *qnext; // disk queue
  uchar data[BSIZE];
};
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

