# xv6 学习笔记 1：从 DOS 到 xv6，再到 QEMU

## 1. 当前 xv6 是一个完整的操作系统吗？

当前仓库是传统 x86 版 xv6。它不是 Windows，也不是面向日常使用的完整操作系统，而是一个可以启动并运行用户程序的教学型 Unix 内核。

从操作系统的基本结构看，它已经包含了一个最小系统所需要的主要部分：

| 层次 | 当前 xv6 的内容 |
|---|---|
| 启动层 | BIOS/QEMU 启动、Bootloader、进入保护模式 |
| 内核入口 | `boot/entry.S`、`kernel/main.c` |
| 中断与系统调用 | IDT、陷阱处理、系统调用分派 |
| 进程管理 | `fork`、`exec`、`wait`、`exit` |
| 调度系统 | 进程状态、调度器、上下文切换 |
| 内存管理 | 页表、虚拟内存、物理页分配、用户地址空间 |
| 文件系统 | inode、目录、文件描述符、日志、块缓存 |
| 设备驱动 | IDE 磁盘、键盘、串口、控制台 |
| 用户态 | Shell、文件命令、测试程序和系统调用库 |
| 镜像工具 | `tools/mkfs.c`，生成文件系统镜像 |

启动后的基本链路是：

```text
加电
  ↓
Bootloader
  ↓
加载内核
  ↓
建立分页和中断
  ↓
启动第一个用户进程
  ↓
运行 init
  ↓
启动 Shell
  ↓
执行用户程序
```

因此，可以说 xv6 在“教学型 Unix 系统”的范围内是相对完整的，但它没有现代操作系统的完整功能，例如网络协议栈、USB 和显卡驱动、图形界面、动态链接器、多用户权限、完善的换页机制和广泛的硬件兼容性。

## 2. 当前仓库的代码规模

按照当前仓库中被 Git 跟踪的 C、头文件和汇编源文件统计：

| 目录 | 文件数 | 文件总行数 |
|---|---:|---:|
| `boot/` | 5 | 345 |
| `kernel/` | 49 | 5,452 |
| `user/` | 20 | 2,921 |
| `tools/` | 1 | 254 |
| 合计 | 75 | 8,972 |

这里的行数包含空行、注释和当前项目增加的中文学习注释，不包含 `docs/`、`references/`、`Makefile` 和 `build/` 编译产物。

代码规模只能作为参考。xv6 的特点是把进程、虚拟内存、系统调用、中断、调度器和文件系统集中在较少的代码中，因此非常适合顺着调用路径学习操作系统。

## 3. xv6 和 DOS 的关系

两者在使用体验上有相似之处：

- 都提供命令行 Shell；
- 都可以执行用户程序；
- 都有文件和目录；
- 用户通过命令行与系统交互。

但是内部结构差异很大：

| 对比 | DOS | xv6 |
|---|---|---|
| 定位 | 磁盘操作系统、单用户环境 | 教学型 Unix 内核 |
| 进程 | 基本没有现代意义上的进程隔离 | 有进程、调度、`fork/exec/wait` |
| 内存 | 主要是实模式和简单内存管理 | 有分页、虚拟地址空间和页表 |
| 权限 | 几乎没有用户态/内核态隔离 | 用户态和内核态分离 |
| 系统调用 | 主要通过 BIOS/DOS 中断 | 通过陷阱进入内核 |
| 文件系统 | FAT 等文件系统 | inode、目录、日志、块缓存 |
| Shell | `COMMAND.COM` | xv6 `sh` |

更准确的类比是：

> xv6 的外部使用体验有点像 DOS 的命令行环境，但内部结构更接近一个极简 Unix/Linux 内核。

## 4. DOS 和 xv6 使用的语言

### 4.1 DOS：汇编语言为主

MS-DOS 2.x 的核心代码主要使用 8086 汇编语言。早期系统内存和磁盘空间有限，而且需要直接控制 CPU、BIOS 和硬件，因此汇编语言非常合适。

典型的 DOS 汇编调用形式是：

```asm
MOV AX, 4C00H
INT 21H
```

其中：

- `MOV` 是汇编指令；
- `AX` 是 8086 寄存器；
- `INT 21H` 是调用 DOS 服务的中断入口。

`COMMAND.COM` 也主要由汇编实现。

### 4.2 xv6：C 语言为主，汇编负责底层入口

当前 x86 版 xv6 的主要内核逻辑使用 C 语言实现，例如：

```text
kernel/proc.c       进程和调度
kernel/vm.c         虚拟内存
kernel/fs.c         文件系统
kernel/syscall.c    系统调用分派
user/sh.c           用户态 Shell
```

少量汇编用于 C 语言难以直接完成的 CPU 底层工作：

```text
boot/bootasm.S      设置保护模式和分页前的环境
boot/entry.S        内核入口
kernel/swtch.S      保存和恢复上下文
kernel/trapasm.S    中断和陷阱入口
kernel/usys.S       用户态系统调用入口
```

可以用下面的方式理解两者：

```text
DOS：
  汇编控制硬件，汇编实现操作系统逻辑

xv6：
  汇编负责 CPU 底层入口，C 语言实现操作系统主要逻辑
```

## 5. DOS 2.1 大约有多少代码？

DOS 2.1 没有一个公认、统一的公开源码行数统计。公开资料主要提供 MS-DOS 2.0 源码，微软仓库包含 MS-DOS 1.25、2.0 和 4.0 的源码；DOS 2.1 的具体 OEM 版本也可能存在差异。

按不同统计范围，可以粗略理解为：

| 统计范围 | 大致规模 |
|---|---:|
| DOS 2.x 核心内核 | 约 1～2 万行汇编 |
| 加上 Shell、FORMAT、COPY、SYS 等工具 | 约 2～4 万行 |
| 再加文档、构建文件和 OEM 适配代码 | 可能超过 4 万行 |

这个范围是历史源码的近似估计，不应当当作 DOS 2.1 的精确官方数字。MS-DOS 2.0 的公开源码包含约 118 个文本文件，其中既有汇编代码，也有文档和构建材料。

参考：

- [Microsoft MS-DOS 源码仓库](https://github.com/microsoft/MS-DOS)
- [MS-DOS 2.0 源码说明](https://github.com/microsoft/MS-DOS/blob/main/v2.0/source/README.txt)

## 6. QEMU 是什么？

QEMU 是一个开源的计算机模拟器和虚拟机。在本项目中，它负责模拟一台可以运行 xv6 的计算机。

```text
Windows
  └── WSL2 Linux
        └── GCC、Make、GDB
              └── QEMU
                    └── xv6
```

QEMU 可以模拟：

- x86 或 RISC-V CPU；
- 内存；
- 磁盘；
- 串口和控制台；
- 中断控制器；
- 键盘、IDE 等设备。

启动 xv6 时，Makefile 先编译 Bootloader、内核和用户程序，生成内核镜像以及文件系统镜像，然后启动 QEMU：

```bash
make TOOLPREFIX= qemu-nox
```

大致过程如下：

```text
Makefile
  ↓
编译 Bootloader、内核和用户程序
  ↓
生成 build/xv6.img 和 build/fs.img
  ↓
启动 QEMU
  ↓
QEMU 模拟 x86 计算机
  ↓
加载 xv6 镜像
  ↓
运行 xv6
```

看到下面的提示时：

```text
xv6...
cpu0: starting 0
init: starting sh
$
```

最后的 `$` 是 xv6 自己的 Shell 提示符，不是 Windows 或 WSL 的 Shell 提示符。

## 7. WSL、GCC、Make、QEMU 和 GDB 的职责

| 工具 | 职责 |
|---|---|
| WSL2 | 提供 Linux 开发环境 |
| GCC | 编译 xv6 的 C 和汇编代码 |
| Make | 按依赖关系组织编译和启动 |
| QEMU | 模拟计算机并运行 xv6 |
| GDB | 调试 xv6 内核，查看寄存器、内存和调用栈 |

学习时应当区分：

```text
WSL 不是 xv6
QEMU 不是 xv6
QEMU 是承载和运行 xv6 的虚拟硬件平台
```

## 8. 建议的理解顺序

建议按下面的顺序继续学习：

1. 先用 QEMU 启动 xv6，确认能看到 xv6 Shell。
2. 阅读 `boot/`，理解 CPU 如何从启动状态进入内核。
3. 阅读 `kernel/main.c`，理解内核初始化顺序。
4. 阅读 `proc.c` 和 `swtch.S`，理解进程、调度和上下文切换。
5. 阅读 `vm.c`，理解页表和用户地址空间。
6. 阅读 `syscall.c`、`trap.c` 和 `usys.S`，理解系统调用路径。
7. 阅读 `fs.c`、`file.c`、`bio.c` 和 `log.c`，理解文件系统。
8. 最后结合 `user/sh.c`，从用户命令反向跟踪到内核实现。

配套图示见：

- [xv6 启动流程瀑布图](D:\xv6-public\docs\xv6启动流程瀑布图.md)
- [xv6 关键寄存器与内存结构图](D:\xv6-public\docs\xv6关键寄存器与内存结构图.md)
- [xv6 学习计划](D:\xv6-public\docs\xv6学习计划.md)
