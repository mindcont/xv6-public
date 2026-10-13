// xv6 中文导读
// 文件：kernel/spinlock.h
// 文件作用：kernel/spinlock.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Mutual exclusion lock.
// 互斥锁。
struct spinlock {
  uint locked;       // Is the lock held?；锁是否已被持有

  // For debugging: 调试信息：
  char *name;        // Name of lock；锁名称
  struct cpu *cpu;   // The cpu holding the lock；持有锁的 CPU
  uint pcs[10];      // The call stack (an array of program counters)；调用栈（程序计数器数组）
                     // that locked the lock；获取该锁时的调用栈
};

