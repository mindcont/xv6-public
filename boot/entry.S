// xv6 中文导读
// 文件：boot/entry.S
// 文件作用：建立临时页表、打开分页、设置内核栈并进入内核 main()
// 所属阶段：内核入口阶段
//
# The xv6 kernel starts executing in this file. This file is linked with
# xv6 内核从此文件开始执行；该文件与
# the kernel C code, so it can refer to kernel symbols such as main().
# 内核 C 代码链接，因此可以引用 main() 等内核符号。
# The boot block (bootasm.S and bootmain.c) jumps to entry below.
# 启动块（bootasm.S 和 bootmain.c）会跳转到下面的 entry。
        
# Multiboot header, for multiboot boot loaders like GNU Grub.
# Multiboot 头，供 GNU Grub 等 Multiboot 启动加载器使用。
# http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
#
# Using GRUB 2, you can boot xv6 from a file stored in a
# Linux file system by copying kernel or kernelmemfs to /boot
# and then adding this menu entry:
#
# menuentry "xv6" {
# 	insmod ext2
# 	set root='(hd0,msdos1)'
# 	set kernel='/boot/kernel'
# 	echo "Loading ${kernel}..."
# 	multiboot ${kernel} ${kernel}
# 	boot
# }

#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"

# Multiboot header.  Data to direct multiboot loader.
.p2align 2
.text
.globl multiboot_header
multiboot_header:
  #define magic 0x1badb002
  #define flags 0
  .long magic
  .long flags
  .long (-magic-flags)

# By convention, the _start symbol specifies the ELF entry point.
# 按约定，_start 符号指定 ELF 入口点。
# Since we haven't set up virtual memory yet, our entry point is
# 由于尚未建立虚拟内存，当前入口点是
# the physical address of 'entry'.
# `entry` 的物理地址。
.globl _start
_start = V2P_WO(entry)

# Entering xv6 on boot processor, with paging off.
# 引导处理器进入 xv6，此时分页尚未开启。
.globl entry
entry:
  # Turn on page size extension for 4Mbyte pages；开启 4 MB 页面所需的页大小扩展
  movl    %cr4, %eax
  orl     $(CR4_PSE), %eax
  movl    %eax, %cr4
  # Set page directory；设置页目录
  movl    $(V2P_WO(entrypgdir)), %eax
  movl    %eax, %cr3
  # Turn on paging；开启分页
  movl    %cr0, %eax
  orl     $(CR0_PG|CR0_WP), %eax
  movl    %eax, %cr0

  # Set up the stack pointer；设置栈指针
  movl $(stack + KSTACKSIZE), %esp

  # Jump to main(), and switch to executing at
  # 跳转到 main()，并切换到
  # high addresses. The indirect call is needed because
  # 高地址执行。必须使用间接调用，因为
  # the assembler produces a PC-relative instruction
  # 汇编器会为直接跳转生成 PC 相对指令，
  # for a direct jump.
  # 而这里需要跳转到高地址。
  mov $main, %eax
  jmp *%eax

.comm stack, KSTACKSIZE
