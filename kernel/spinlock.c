// xv6 中文导读
// 文件：kernel/spinlock.c
// 文件作用：实现多 CPU 环境下的自旋锁
// 所属阶段：并发与锁阶段
//
// Mutual exclusion spin locks.
// 互斥自旋锁。

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// 获取锁。
// Loops (spins) until the lock is acquired.
// 循环自旋，直到成功获取锁。
// Holding a lock for a long time may cause
// 长时间持有锁可能导致
// other CPUs to waste time spinning to acquire it.
// 其他 CPU 浪费时间等待该锁。
void
acquire(struct spinlock *lk)
{
  pushcli(); // disable interrupts to avoid deadlock；关闭中断以避免死锁
  if(holding(lk))
    panic("acquire");

  // The xchg is atomic.
  // xchg 指令是原子的。
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // 告诉 C 编译器和处理器不要移动读写操作，
  // past this point, to ensure that the critical section's memory
  // 确保临界区的内存访问
  // references happen after the lock is acquired.
  // 发生在获取锁之后。
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  // 记录获取锁的信息，供调试使用。
  lk->cpu = mycpu();
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
// 释放锁。
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // Tell the C compiler and the processor to not move loads or stores
  // 告诉 C 编译器和处理器不要移动读写操作，
  // past this point, to ensure that all the stores in the critical
  // 确保临界区中的所有写操作
  // section are visible to other cores before the lock is released.
  // 在释放锁之前对其他 CPU 可见。
  // Both the C compiler and the hardware may re-order loads and
  // C 编译器和硬件都可能重新排序读写操作，
  // stores; __sync_synchronize() tells them both not to.
  // __sync_synchronize() 会阻止这种重排序。
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // 释放锁，效果等价于 lk->locked = 0。
  // This code can't use a C assignment, since it might
  // 这里不能直接使用 C 赋值，因为它可能
  // not be atomic. A real OS would use C atomics here.
  // 不是原子的；真正的操作系统会使用 C 原子操作。
  asm volatile("movl $0, %0" : "+m" (lk->locked) : );

  popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
// 沿着 %ebp 链记录当前调用栈到 pcs[]。
void
getcallerpcs(void *v, uint pcs[])
{
  uint *ebp;
  int i;

  ebp = (uint*)v - 2;
  for(i = 0; i < 10; i++){
    if(ebp == 0 || ebp < (uint*)KERNBASE || ebp == (uint*)0xffffffff)
      break;
    pcs[i] = ebp[1];     // saved %eip
    ebp = (uint*)ebp[0]; // saved %ebp
  }
  for(; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
// 检查当前 CPU 是否持有该锁。
int
holding(struct spinlock *lock)
{
  int r;
  pushcli();
  r = lock->locked && lock->cpu == mycpu();
  popcli();
  return r;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// pushcli/popcli 类似 cli/sti，但它们成对匹配：
// it takes two popcli to undo two pushcli.  Also, if interrupts
// 两次 pushcli 需要两次 popcli 才能抵消；如果中断
// are off, then pushcli, popcli leaves them off.
// 原本关闭，那么 pushcli、popcli 后仍保持关闭。

void
pushcli(void)
{
  int eflags;

  eflags = readeflags();
  cli();
  if(mycpu()->ncli == 0)
    mycpu()->intena = eflags & FL_IF;
  mycpu()->ncli += 1;
}

void
popcli(void)
{
  if(readeflags()&FL_IF)
    panic("popcli - interruptible");
  if(--mycpu()->ncli < 0)
    panic("popcli");
  if(mycpu()->ncli == 0 && mycpu()->intena)
    sti();
}

