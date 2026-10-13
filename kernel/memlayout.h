// xv6 中文导读
// 文件：kernel/memlayout.h
// 文件作用：kernel/memlayout.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Memory layout
// 内核物理和虚拟内存布局。

#define EXTMEM  0x100000            // Start of extended memory；扩展内存起始地址
#define PHYSTOP 0xE000000           // Top physical memory；物理内存上限
#define DEVSPACE 0xFE000000         // Other devices are at high addresses；其他设备位于高地址

// Key addresses for address space layout (see kmap in vm.c for layout)
// 地址空间布局中的关键地址，具体见 vm.c 的 kmap。
#define KERNBASE 0x80000000         // First kernel virtual address；第一个内核虚拟地址
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked；内核链接地址

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts；等同 V2P，但不做类型转换
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts；等同 P2V，但不做类型转换
