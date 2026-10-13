// xv6 中文导读
// 文件：kernel/sleeplock.h
// 文件作用：kernel/sleeplock.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Long-term locks for processes
// 允许进程睡眠的长期锁。
struct sleeplock {
  uint locked;       // Is the lock held?；锁是否已被持有
  struct spinlock lk; // spinlock protecting this sleep lock；保护睡眠锁的自旋锁
  
  // For debugging: 调试信息：
  char *name;        // Name of lock；锁名称
  int pid;           // Process holding lock；持有锁的进程 ID
};

