// xv6 中文导读
// 文件：boot/bootasm.S
// 文件作用：启动扇区的早期 x86 汇编入口，切换到保护模式并调用 bootmain()
// 所属阶段：设备加电后的 Bootloader 阶段
//
#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# Start the first CPU: switch to 32-bit protected mode, jump into C.
# 启动第一个 CPU：切换到 32 位保护模式并跳转到 C 代码。
# The BIOS loads this code from the first sector of the hard disk into
# BIOS 将这段代码从硬盘第一个扇区加载到
# memory at physical address 0x7c00 and starts executing in real mode
# 物理地址 0x7c00，并在实模式下开始执行，
# with %cs=0 %ip=7c00.
# 此时 %cs=0、%ip=0x7c00。

.code16                       # Assemble for 16-bit mode
.globl start
start:
  cli                         # BIOS enabled interrupts; disable

  # Zero data segment registers DS, ES, and SS.
  # 将数据段寄存器 DS、ES 和 SS 清零。
  xorw    %ax,%ax             # Set %ax to zero；将 %ax 清零
  movw    %ax,%ds             # -> Data Segment；数据段
  movw    %ax,%es             # -> Extra Segment；附加段
  movw    %ax,%ss             # -> Stack Segment；栈段

  # Physical address line A20 is tied to zero so that the first PCs 
  # 早期 PC 将物理地址线 A20 固定为 0，
  # with 2 MB would run software that assumed 1 MB.  Undo that.
  # 使拥有 2 MB 内存的机器仍能运行假设只有 1 MB 的软件；这里撤销该限制。
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # Switch from real to protected mode.  Use a bootstrap GDT that makes
  # 从实模式切换到保护模式。使用启动 GDT，使
  # virtual addresses map directly to physical addresses so that the
  # 虚拟地址直接映射到物理地址，从而
  # effective memory map doesn't change during the transition.
  # 在切换期间有效内存布局保持不变。
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE, %eax
  movl    %eax, %cr0

//PAGEBREAK!
  # Complete the transition to 32-bit protected mode by using a long jmp
  # 通过长跳转完成到 32 位保护模式的切换，
  # to reload %cs and %eip.  The segment descriptors are set up with no
  # 重新加载 %cs 和 %eip。段描述符不做地址转换，
  # translation, so that the mapping is still the identity mapping.
  # 因此映射仍然是恒等映射。
  ljmp    $(SEG_KCODE<<3), $start32

  .code32  # Tell assembler to generate 32-bit code now；通知汇编器生成 32 位代码
start32:
  # Set up the protected-mode data segment registers；设置保护模式数据段寄存器
  movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    $0, %ax                 # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  # Set up the stack pointer and call into C；设置栈指针并调用 C 代码
  movl    $start, %esp
  call    bootmain

  # If bootmain returns (it shouldn't), trigger a Bochs
  # 如果 bootmain 返回（正常不应返回），则触发 Bochs
  # breakpoint if running under Bochs, then loop.
  # 断点，然后进入死循环。
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
  outw    %ax, %dx
spin:
  jmp     spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment
gdt:
  SEG_NULLASM                             # null seg
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
  SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
  .long   gdt                             # address gdt

