// xv6 中文导读
// 文件：kernel/syscall.c
// 文件作用：根据系统调用号，将用户请求分派给具体内核函数
// 所属阶段：系统调用阶段
//
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// User code makes a system call with INT T_SYSCALL.
// 用户代码通过 INT T_SYSCALL 发起系统调用。
// System call number in %eax.
// 系统调用号保存在 %eax 中。
// Arguments on the stack, from the user call to the C
// 参数位于用户栈上，从用户调用传递到 C
// library system call function. The saved user %esp points
// 库中的系统调用函数。保存的用户 %esp 先指向
// to a saved program counter, and then the first argument.
// 保存的程序计数器，再指向第一个参数。

// Fetch the int at addr from the current process.
// 从当前进程地址空间的 addr 处读取一个整数。
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// 从当前进程读取 addr 处的 NUL 结尾字符串。
// Doesn't actually copy the string - just sets *pp to point at it.
// 函数不会复制字符串，只让 *pp 指向它。
// Returns length of string, not including nul.
// 返回不包含 NUL 字符的字符串长度。
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
// 获取第 n 个 32 位系统调用参数。
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// 将第 n 个字长系统调用参数作为指针读取，
// to a block of memory of size bytes.  Check that the pointer
// 它指向大小为 bytes 的内存块，并检查该指针
// lies within the process address space.
// 是否位于进程地址空间内。
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// 将第 n 个字长系统调用参数作为字符串指针读取。
// Check that the pointer is valid and the string is nul-terminated.
// 检查指针有效且字符串以 NUL 结尾。
// (There is no shared writable memory, so the string can't change
// 不存在共享可写内存，因此字符串不会在检查后被修改，
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
};

void
// 中文说明：读取系统调用号和参数，并分派到对应的内核实现。
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
