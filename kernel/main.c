// xv6 中文导读
// 文件：kernel/main.c
// 文件作用：内核总入口，按顺序初始化 CPU、内存、中断、设备、文件系统和进程
// 所属阶段：内核初始化阶段
//
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file；ELF 加载内核后的第一个地址

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
// 引导处理器从这里开始执行 C 代码，先完成内存分配器所需的初始化，
// 分配真正的内核栈并切换到该栈上。
int
// 中文说明：内核启动总入口，按顺序初始化 CPU、内存、中断、设备、文件系统和进程。
main(void)
{
  kinit1(end, P2V(4*1024*1024)); // phys page allocator；初始化物理页分配器
  kvmalloc();      // kernel page table；建立内核页表
  mpinit();        // detect other processors；检测其他处理器
  lapicinit();     // interrupt controller；初始化局部 APIC
  seginit();       // segment descriptors；初始化段描述符
  picinit();       // disable pic；禁用旧式 PIC
  ioapicinit();    // another interrupt controller；初始化 IOAPIC
  consoleinit();   // console hardware；初始化控制台
  uartinit();      // serial port；初始化串口
  pinit();         // process table；初始化进程表
  tvinit();        // trap vectors；初始化陷阱向量
  binit();         // buffer cache；初始化块缓存
  fileinit();      // file table；初始化文件表
  ideinit();       // disk；初始化 IDE 磁盘
  startothers();   // start other processors；启动其他处理器
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
  userinit();      // first user process；创建第一个用户进程
  mpmain();        // finish this processor's setup；完成当前处理器初始化
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}

// Common CPU setup code.
static void
// 中文说明：每个 CPU 的内核主流程，完成本 CPU 初始化后进入调度器。
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  xchg(&(mycpu()->started), 1); // tell startothers() we're up
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
static void
// 中文说明：复制并启动其他 CPU 的入口代码，为每个 AP 配置栈和启动参数。
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void(**)(void))(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(c->started == 0)
      ;
  }
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
  [0] = (0) | PTE_P | PTE_W | PTE_PS,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

