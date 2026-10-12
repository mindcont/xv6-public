// xv6 中文导读
// 文件：kernel/spinlock.h
// 文件作用：kernel/spinlock.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint pcs[10];      // The call stack (an array of program counters)
                     // that locked the lock.
};

