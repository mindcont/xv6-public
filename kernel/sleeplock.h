// xv6 中文导读
// 文件：kernel/sleeplock.h
// 文件作用：kernel/sleeplock.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Long-term locks for processes
struct sleeplock {
  uint locked;       // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock
  
  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

