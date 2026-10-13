// xv6 中文导读
// 文件：kernel/proc.h
// 文件作用：定义进程状态、进程控制块和 CPU 相关数据结构
// 所属阶段：进程与调度阶段
//
// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state；每个进程的状态
struct proc {
  uint sz;                     // Size of process memory (bytes)；进程内存大小（字节）
  pde_t* pgdir;                // Page table；页表
  char *kstack;                // Bottom of kernel stack for this process；该进程内核栈底
  enum procstate state;        // Process state；进程状态
  int pid;                     // Process ID；进程 ID
  struct proc *parent;         // Parent process；父进程
  struct trapframe *tf;        // Trap frame for current syscall；当前系统调用的陷阱帧
  struct context *context;     // swtch() here to run process；通过 swtch() 切换到此处运行进程
  void *chan;                  // If non-zero, sleeping on chan；非零时表示正在该通道睡眠
  int killed;                  // If non-zero, have been killed；非零时表示已被杀死
  struct file *ofile[NOFILE];  // Open files；打开的文件
  struct inode *cwd;           // Current directory；当前目录
  char name[16];               // Process name (debugging)；进程名（调试用）
};

// Process memory is laid out contiguously, low addresses first:
// 进程内存从低地址到高地址连续布局：
//   text
//   代码段
//   original data and bss
//   初始数据段和 BSS 段
//   fixed-size stack
//   固定大小的栈
//   expandable heap
//   可扩展的堆
