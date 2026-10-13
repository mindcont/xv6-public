// xv6 中文导读
// 文件：kernel/asm.h
// 文件作用：kernel/asm.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
//
// assembler macros to create x86 segments
// 用于创建 x86 段描述符的汇编宏。
//

#define SEG_NULLASM                                             \
        .word 0, 0;                                             \
        .byte 0, 0, 0, 0

// The 0xC0 means the limit is in 4096-byte units
// 0xC0 表示段界限以 4096 字节为单位，
// and (for executable segments) 32-bit mode.
// 对可执行段表示 32 位模式。
#define SEG_ASM(type,base,lim)                                  \
        .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
        .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
                (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define STA_X     0x8       // Executable segment；可执行段
#define STA_W     0x2       // Writeable (non-executable segments)；可写（不可执行段）
#define STA_R     0x2       // Readable (executable segments)；可读（可执行段）
