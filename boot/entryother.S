// xv6 中文导读
// 文件：boot/entryother.S
// 文件作用：为其他 CPU 提供启动入口，建立执行环境后进入 mpmain()
// 所属阶段：多处理器启动阶段
//
#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
	
# Each non-boot CPU ("AP") is started up in response to a STARTUP
# 每个非引导 CPU（“AP”）响应 STARTUP
# IPI from the boot CPU.  Section B.4.2 of the Multi-Processor
# IPI，由引导 CPU 发送。多处理器规范 B.4.2 节
# Specification says that the AP will start in real mode with CS:IP
# 规定 AP 将以实模式启动，CS:IP
# set to XY00:0000, where XY is an 8-bit value sent with the
# 设置为 XY00:0000，其中 XY 是随
# STARTUP. Thus this code must start at a 4096-byte boundary.
# STARTUP 发送的 8 位值。因此这段代码必须从 4096 字节边界开始。
#
# Because this code sets DS to zero, it must sit
# 由于这段代码将 DS 设为 0，所以它必须位于
# at an address in the low 2^16 bytes.
# 低 2^16 字节地址范围内。
#
# Startothers (in main.c) sends the STARTUPs one at a time.
# main.c 中的 startothers() 一次向一个 CPU 发送 STARTUP。
# It copies this code (start) at 0x7000.  It puts the address of
# 它把这段代码（start）复制到 0x7000，并把
# a newly allocated per-core stack in start-4,the address of the
# 每个 CPU 新分配的栈地址写入 start-4，把
# place to jump to (mpenter) in start-8, and the physical address
# 跳转目标 mpenter 写入 start-8，并把
# of entrypgdir in start-12.
# entrypgdir 的物理地址写入 start-12。
#
# This code combines elements of bootasm.S and entry.S.
# 这段代码结合了 bootasm.S 和 entry.S 的功能。

.code16           
.globl start
start:
  cli            

  # Zero data segment registers DS, ES, and SS.
  # 将数据段寄存器 DS、ES 和 SS 清零。
  xorw    %ax,%ax
  movw    %ax,%ds
  movw    %ax,%es
  movw    %ax,%ss

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

  # Complete the transition to 32-bit protected mode by using a long jmp
  # 使用长跳转完成到 32 位保护模式的切换，
  # to reload %cs and %eip.  The segment descriptors are set up with no
  # 重新加载 %cs 和 %eip。段描述符不做地址转换，
  # translation, so that the mapping is still the identity mapping.
  # 因此映射仍然是恒等映射。
  ljmpl    $(SEG_KCODE<<3), $(start32)

//PAGEBREAK!
.code32  # Tell assembler to generate 32-bit code now.
start32:
  # Set up the protected-mode data segment registers；设置保护模式数据段寄存器
  movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    $0, %ax                 # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  # Turn on page size extension for 4Mbyte pages；开启 4 MB 页面扩展
  movl    %cr4, %eax
  orl     $(CR4_PSE), %eax
  movl    %eax, %cr4
  # Use entrypgdir as our initial page table；使用 entrypgdir 作为初始页表
  movl    (start-12), %eax
  movl    %eax, %cr3
  # Turn on paging；开启分页
  movl    %cr0, %eax
  orl     $(CR0_PE|CR0_PG|CR0_WP), %eax
  movl    %eax, %cr0

  # Switch to the stack allocated by startothers()；切换到 startothers() 分配的栈
  movl    (start-4), %esp
  # Call mpenter()；调用 mpenter()
  call	 *(start-8)

  movw    $0x8a00, %ax
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax
  outw    %ax, %dx
spin:
  jmp     spin

.p2align 2
gdt:
  SEG_NULLASM
  SEG_ASM(STA_X|STA_R, 0, 0xffffffff)
  SEG_ASM(STA_W, 0, 0xffffffff)


gdtdesc:
  .word   (gdtdesc - gdt - 1)
  .long   gdt

