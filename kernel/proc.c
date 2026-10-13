// xv6 中文导读
// 文件：kernel/proc.c
// 文件作用：实现进程控制块、进程创建、调度、睡眠、唤醒和退出
// 所属阶段：进程与调度阶段
//
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
// 调用者必须先关闭中断。
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
// 读取 CPU 结构中的 proc 指针时关闭中断，避免当前 CPU 被重新调度。
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// 在进程表中查找状态为 UNUSED 的进程。
// If found, change state to EMBRYO and initialize
// 找到后将状态改为 EMBRYO，并初始化
// state required to run in the kernel.
// 在内核中运行所需要的状态。
// Otherwise return 0.
// 如果没有空闲进程槽，则返回 0。
static struct proc*
// 中文说明：从进程表中分配一个新进程控制块，并准备内核栈和上下文。
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  // 分配内核栈。
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  // 为陷阱帧预留空间。
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // 设置新上下文，使其从 forkret 开始执行，
  // which returns to trapret.
  // forkret 随后返回到 trapret。
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
// 中文说明：创建第一个用户进程，并把 initcode 放入其用户地址空间。
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// 将当前进程的内存增加 n 字节。
// Return 0 on success, -1 on failure.
// 成功返回 0，失败返回 -1。
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// 复制父进程 p，创建一个新进程。
// Sets up stack to return as if from system call.
// 设置栈，使子进程看起来像是从系统调用返回。
// Caller must set state of returned proc to RUNNABLE.
// 调用者必须把返回进程的状态设置为 RUNNABLE。
int
// 中文说明：复制当前进程的地址空间和资源，创建一个新的子进程。
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process. 分配进程控制块。
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc. 复制父进程的状态。
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  // 清零 %eax，使 fork 在子进程中返回 0。
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// 退出当前进程；函数不会返回。
// An exited process remains in the zombie state
// 退出后的进程会保持僵尸状态，
// until its parent calls wait() to find out it exited.
// 直到父进程调用 wait() 获取其退出信息。
void
// 中文说明：结束当前进程，关闭资源并唤醒等待它的父进程。
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  // 关闭所有打开的文件。
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  // 父进程可能正在 wait() 中睡眠。
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  // 将失去父进程的子进程交给 init 管理。
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// 等待子进程退出并返回其 PID。
// Return -1 if this process has no children.
// 如果当前进程没有子进程，则返回 -1。
int
// 中文说明：等待子进程退出，回收僵尸进程的内核资源。
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
  // Scan through table looking for exited children.
  // 扫描进程表，查找已经退出的子进程。
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one. 找到一个符合条件的子进程。
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    // 没有子进程时继续等待没有意义。
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    // 等待子进程退出（参见 proc_exit() 中的 wakeup1 调用）。
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// 每个 CPU 独立运行的进程调度器。
// Each CPU calls scheduler() after setting itself up.
// 每个 CPU 完成自身初始化后调用 scheduler()。
// Scheduler never returns.  It loops, doing:
// 调度器不会返回，而是循环执行：
//  - choose a process to run
//  - 选择一个要运行的进程
//  - swtch to start running that process
//  - 通过 swtch 切换并开始运行该进程
//  - eventually that process transfers control
//  - 最终该进程把控制权
//      via swtch back to the scheduler.
//      通过 swtch 切回调度器。
void
// 中文说明：在可运行进程中选择下一个进程并进行上下文切换。
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    // 开启当前 CPU 的中断。
    sti();

    // Loop over process table looking for process to run.
    // 遍历进程表，查找可以运行的进程。
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // 切换到选中的进程；进程负责
      // to release ptable.lock and then reacquire it
      // 释放 ptable.lock，并在
      // before jumping back to us.
      // 切回调度器之前重新获取它。
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // 该进程暂时运行结束。
      // It should have changed its p->state before coming back.
      // 它应该在切回之前修改了 p->state。
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// 进入调度器；调用者只能持有 ptable.lock，
// and have changed proc->state. Saves and restores
// 并且必须已经修改 proc->state。函数保存和恢复
// intena because intena is a property of this
// intena，因为它属于当前内核线程，
// kernel thread, not this CPU. It should
// 而不是当前 CPU。它本应存放在
// be proc->intena and proc->ncli, but that would
// proc->intena 和 proc->ncli 中，但这样会
// break in the few places where a lock is held but
// 破坏少数“持锁但
// there's no process.
// 没有当前进程”的场景。
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
// 让出 CPU 一个调度轮次。
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// fork 创建的子进程第一次被 scheduler() 调度时，
// will swtch here.  "Return" to user space.
// 会通过 swtch() 到达这里，然后“返回”用户空间。
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  // 仍然持有 scheduler() 传来的 ptable.lock。
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // 某些初始化函数必须在普通进程上下文中运行，
    // of a regular process (e.g., they call sleep), and thus cannot
    // 例如它们会调用 sleep()，因此不能
    // be run from main().
    // 从 main() 中直接运行。
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
  // 返回到“调用者”，实际是返回 trapret（参见 allocproc）。
}

// Atomically release lock and sleep on chan.
// 原子地释放锁，并在 chan 通道上睡眠。
// Reacquires lock when awakened.
// 被唤醒后重新获取该锁。
void
// 中文说明：原子地释放锁并让当前进程睡眠，等待指定条件发生。
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
// 中文说明：唤醒等待指定通道的进程。
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
