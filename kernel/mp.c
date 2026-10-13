// xv6 中文导读
// 文件：kernel/mp.c
// 文件作用：kernel/mp.c 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Multiprocessor support
// 多处理器支持：在内存中搜索 MP 描述结构。
// http://developer.intel.com/design/pentium/datashts/24201606.pdf

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mp.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"

struct cpu cpus[NCPU];
int ncpu;
uchar ioapicid;

static uchar
sum(uchar *addr, int len)
{
  int i, sum;

  sum = 0;
  for(i=0; i<len; i++)
    sum += addr[i];
  return sum;
}

// Look for an MP structure in the len bytes at addr.
// 在 addr 开始的 len 字节中查找 MP 结构。
static struct mp*
mpsearch1(uint a, int len)
{
  uchar *e, *p, *addr;

  addr = P2V(a);
  e = addr+len;
  for(p = addr; p < e; p += sizeof(struct mp))
    if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
      return (struct mp*)p;
  return 0;
}

// Search for the MP Floating Pointer Structure, which according to the
// 搜索 MP 浮动指针结构。根据规范，它位于以下位置之一：
// spec is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 1）EBDA 的前 1 KB；
// 2) in the last KB of system base memory;
// 2）系统基本内存的最后 1 KB；
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
// 3）0xE0000 到 0xFFFFF 之间的 BIOS ROM。
static struct mp*
mpsearch(void)
{
  uchar *bda;
  uint p;
  struct mp *mp;

  bda = (uchar *) P2V(0x400);
  if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
    if((mp = mpsearch1(p, 1024)))
      return mp;
  } else {
    p = ((bda[0x14]<<8)|bda[0x13])*1024;
    if((mp = mpsearch1(p-1024, 1024)))
      return mp;
  }
  return mpsearch1(0xF0000, 0x10000);
}

// Search for an MP configuration table.  For now,
// 搜索 MP 配置表。目前
// don't accept the default configurations (physaddr == 0).
// 不接受默认配置（physaddr == 0）。
// Check for correct signature, calculate the checksum and,
// 检查签名是否正确、计算校验和，并且
// if correct, check the version.
// 在校验正确时检查版本。
// To do: check extended table checksum.
// 待完成：检查扩展表校验和。
static struct mpconf*
mpconfig(struct mp **pmp)
{
  struct mpconf *conf;
  struct mp *mp;

  if((mp = mpsearch()) == 0 || mp->physaddr == 0)
    return 0;
  conf = (struct mpconf*) P2V((uint) mp->physaddr);
  if(memcmp(conf, "PCMP", 4) != 0)
    return 0;
  if(conf->version != 1 && conf->version != 4)
    return 0;
  if(sum((uchar*)conf, conf->length) != 0)
    return 0;
  *pmp = mp;
  return conf;
}

void
mpinit(void)
{
  uchar *p, *e;
  int ismp;
  struct mp *mp;
  struct mpconf *conf;
  struct mpproc *proc;
  struct mpioapic *ioapic;

  if((conf = mpconfig(&mp)) == 0)
    panic("Expect to run on an SMP");
  ismp = 1;
  lapic = (uint*)conf->lapicaddr;
  for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
    switch(*p){
    case MPPROC:
      proc = (struct mpproc*)p;
      if(ncpu < NCPU) {
        cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu；apicid 可能不同于 ncpu
        ncpu++;
      }
      p += sizeof(struct mpproc);
      continue;
    case MPIOAPIC:
      ioapic = (struct mpioapic*)p;
      ioapicid = ioapic->apicno;
      p += sizeof(struct mpioapic);
      continue;
    case MPBUS:
    case MPIOINTR:
    case MPLINTR:
      p += 8;
      continue;
    default:
      ismp = 0;
      break;
    }
  }
  if(!ismp)
    panic("Didn't find a suitable machine");

  if(mp->imcrp){
    // Bochs doesn't support IMCR, so this doesn't run on Bochs.
    // Bochs 不支持 IMCR，因此这段代码不会在 Bochs 中执行。
    // But it would on real hardware.
    // 但在真实硬件上会执行。
    outb(0x22, 0x70);   // Select IMCR；选择 IMCR
    outb(0x23, inb(0x23) | 1);  // Mask external interrupts；屏蔽外部中断
  }
}
