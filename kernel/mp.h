// xv6 中文导读
// 文件：kernel/mp.h
// 文件作用：kernel/mp.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// See MultiProcessor Specification Version 1.[14]
// 参考《多处理器规范》1.1/1.4 版本。

struct mp {             // floating pointer；浮动指针结构
  uchar signature[4];           // "_MP_"
  void *physaddr;               // phys addr of MP config table；MP 配置表物理地址
  uchar length;                 // 1；长度
  uchar specrev;                // [14]；规范版本
  uchar checksum;               // all bytes must add up to 0；所有字节相加必须为 0
  uchar type;                   // MP system config type；MP 系统配置类型
  uchar imcrp;
  uchar reserved[3];
};

struct mpconf {         // configuration table header；配置表头
  uchar signature[4];           // "PCMP"
  ushort length;                // total table length；表总长度
  uchar version;                // [14]；版本
  uchar checksum;               // all bytes must add up to 0；所有字节相加必须为 0
  uchar product[20];            // product id；产品标识
  uint *oemtable;               // OEM table pointer；OEM 表指针
  ushort oemlength;             // OEM table length；OEM 表长度
  ushort entry;                 // entry count；表项数量
  uint *lapicaddr;              // address of local APIC；Local APIC 地址
  ushort xlength;               // extended table length；扩展表长度
  uchar xchecksum;              // extended table checksum；扩展表校验和
  uchar reserved;
};

struct mpproc {         // processor table entry；处理器表项
  uchar type;                   // entry type (0)；表项类型（0）
  uchar apicid;                 // local APIC id；Local APIC ID
  uchar version;                // local APIC verison；Local APIC 版本
  uchar flags;                  // CPU flags；CPU 标志
    #define MPBOOT 0x02           // This proc is the bootstrap processor；该处理器是引导处理器
  uchar signature[4];           // CPU signature；CPU 签名
  uint feature;                 // feature flags from CPUID instruction；CPUID 返回的特性标志
  uchar reserved[8];
};

struct mpioapic {       // I/O APIC table entry；I/O APIC 表项
  uchar type;                   // entry type (2)；表项类型（2）
  uchar apicno;                 // I/O APIC id；I/O APIC ID
  uchar version;                // I/O APIC version；I/O APIC 版本
  uchar flags;                  // I/O APIC flags；I/O APIC 标志
  uint *addr;                  // I/O APIC address；I/O APIC 地址
};

// Table entry types；表项类型
#define MPPROC    0x00  // One per processor；每个处理器一个
#define MPBUS     0x01  // One per bus；每条总线一个
#define MPIOAPIC  0x02  // One per I/O APIC；每个 I/O APIC 一个
#define MPIOINTR  0x03  // One per bus interrupt source；每个总线中断源一个
#define MPLINTR   0x04  // One per system interrupt source；每个系统中断源一个

//PAGEBREAK!
// Blank page.
