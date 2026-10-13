// xv6 中文导读
// 文件：kernel/ioapic.c
// 文件作用：kernel/ioapic.c 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// The I/O APIC manages hardware interrupts for an SMP system.
// I/O APIC 管理 SMP 系统的硬件中断。
// http://www.intel.com/design/chipsets/datashts/29056601.pdf
// See also picirq.c.
// 也可以参考 picirq.c。

#include "types.h"
#include "defs.h"
#include "traps.h"

#define IOAPIC  0xFEC00000   // Default physical address of IO APIC；IO APIC 默认物理地址

#define REG_ID     0x00  // Register index: ID；寄存器索引：ID
#define REG_VER    0x01  // Register index: version；寄存器索引：版本
#define REG_TABLE  0x10  // Redirection table base；重定向表起始位置

// The redirection table starts at REG_TABLE and uses
// 重定向表从 REG_TABLE 开始，每个中断使用
// two registers to configure each interrupt.
// 两个寄存器进行配置。
// The first (low) register in a pair contains configuration bits.
// 每对寄存器中的低寄存器包含配置位。
// The second (high) register contains a bitmask telling which
// 高寄存器包含位掩码，表示哪些
// CPUs can serve that interrupt.
// CPU 可以处理该中断。
#define INT_DISABLED   0x00010000  // Interrupt disabled；禁用中断
#define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)；电平触发，而非边沿触发
#define INT_ACTIVELOW  0x00002000  // Active low (vs high)；低电平有效，而非高电平有效
#define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)；目标是 CPU ID，而非 APIC ID

volatile struct ioapic *ioapic;

// IO APIC MMIO structure: write reg, then read or write data.
// IO APIC 的 MMIO 结构：先写寄存器索引，再读写数据。
struct ioapic {
  uint reg;
  uint pad[3];
  uint data;
};

static uint
ioapicread(int reg)
{
  ioapic->reg = reg;
  return ioapic->data;
}

static void
ioapicwrite(int reg, uint data)
{
  ioapic->reg = reg;
  ioapic->data = data;
}

void
ioapicinit(void)
{
  int i, id, maxintr;

  ioapic = (volatile struct ioapic*)IOAPIC;
  maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
  id = ioapicread(REG_ID) >> 24;
  if(id != ioapicid)
    cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");

  // Mark all interrupts edge-triggered, active high, disabled,
  // 将所有中断标记为边沿触发、高电平有效、禁用，
  // and not routed to any CPUs.
  // 并且不路由到任何 CPU。
  for(i = 0; i <= maxintr; i++){
    ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
    ioapicwrite(REG_TABLE+2*i+1, 0);
  }
}

void
ioapicenable(int irq, int cpunum)
{
  // Mark interrupt edge-triggered, active high,
  // 将中断标记为边沿触发、高电平有效、
  // enabled, and routed to the given cpunum,
  // 已启用，并路由到指定的 cpunum，
  // which happens to be that cpu's APIC ID.
  // 该编号正好是该 CPU 的 APIC ID。
  ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
  ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
