# xv6 中文学习计划

## 1. 学习目标

通过阅读和运行当前仓库中的 x86 版本 xv6，逐步理解一个 Unix-like 操作系统的核心机制：

- 用户程序如何启动和运行；
- Shell 如何创建进程、执行程序和实现管道；
- 用户态如何通过系统调用进入内核态；
- 内核如何管理进程、CPU、内存和文件；
- 文件系统如何把目录、inode、文件描述符和磁盘块连接起来；
- 中断、锁和上下文切换如何共同支撑并发执行。

这不是 Windows 应用程序开发项目。用户程序会被编译成 xv6 能运行的 x86 ELF 程序，并写入 xv6 的文件系统镜像中，由 xv6 自己加载执行。

## 2. 当前仓库结构说明

当前仓库是传统 x86 版 xv6，源码全部平铺在根目录，没有现代项目常见的 `kernel/`、`user/`、`fs/` 子目录。阅读时应按照功能分组理解文件，而不是按照目录层级理解。

主要功能分组如下：

| 分组 | 代表文件 | 主要内容 |
|---|---|---|
| 启动与链接 | `bootasm.S`、`bootmain.c`、`entry.S`、`kernel.ld` | CPU 启动、加载内核、进入内核 |
| 内核初始化 | `main.c`、`defs.h`、`types.h`、`param.h` | 初始化各个内核子系统 |
| 进程与调度 | `proc.c`、`proc.h`、`swtch.S` | 进程创建、调度、睡眠、退出 |
| 中断与设备 | `trap.c`、`uart.c`、`console.c`、`ide.c` | 中断、键盘、串口、磁盘 |
| 内存管理 | `kalloc.c`、`vm.c`、`mmu.h` | 物理页、页表、用户地址空间 |
| 系统调用 | `syscall.c`、`sysproc.c`、`sysfile.c`、`usys.S` | 用户态进入内核态的接口 |
| 文件系统 | `bio.c`、`log.c`、`fs.c`、`file.c` | 块缓存、日志、inode、文件描述符 |
| 用户程序 | `init.c`、`sh.c`、`ls.c`、`cat.c` 等 | xv6 中运行的类 Unix 程序 |
| 构建与调试 | `Makefile`、`.gdbinit.tmpl`、`gdbutil` | 编译、启动和调试 |

## 3. 学习原则

### 3.1 先建立调用链，再读实现细节

每次阅读都从一个用户可见行为开始，例如：

```text
ls
→ sh.c
→ fork()
→ exec("ls")
→ usys.S
→ syscall.c
→ sysfile.c
→ file.c / fs.c
```

不要一开始试图逐行读完整个 `proc.c`、`vm.c` 或 `fs.c`。

### 3.2 每次只追踪一个问题

建议围绕以下问题阅读：

- 一个用户程序是如何被加载的？
- `fork()` 到底复制了什么？
- `exec()` 如何替换进程地址空间？
- `read()` 如何从磁盘或键盘返回数据？
- Shell 的管道如何连接两个进程？
- 进程从 `RUNNABLE` 到 `RUNNING` 的过程是什么？
- 一个文件名如何最终对应到磁盘块？

### 3.3 同时看数据结构和状态变化

阅读函数时，重点记录：

1. 输入参数和返回值；
2. 修改了哪些全局数据结构；
3. 使用了哪把锁；
4. 是否可能睡眠或被中断；
5. 调用完成后，进程、内存或文件系统状态发生了什么变化。

## 4. 分阶段学习计划

## 阶段一：建立运行环境和整体认识

### 目标

知道 xv6 如何编译、生成镜像并启动，区分内核镜像和文件系统镜像。

### 阅读文件

```text
README
Makefile
main.c
```

### 需要理解

- `xv6.img`：内核所在的磁盘镜像；
- `fs.img`：用户文件系统镜像；
- `mkfs.c`：生成文件系统镜像；
- `make qemu`：使用 QEMU 启动 xv6；
- `main.c`：内核初始化的总入口。

### 实践任务

- 在 WSL2 Ubuntu 中编译并启动 xv6：

  ```bash
  cd /mnt/d/xv6-public
  make TOOLPREFIX= qemu-nox
  ```

  编译产物位于 `build/`，成功启动后应看到：

  ```text
  init: starting sh
  $
  ```

- 如果已经完成编译，只想再次启动，可以直接执行：

  ```bash
  make TOOLPREFIX= qemu-nox
  ```

- 启动带 QEMU 图形窗口的版本：

  ```bash
  make TOOLPREFIX= qemu
  ```

- 启动 GDB 调试版本：

  ```bash
  make TOOLPREFIX= qemu-nox-gdb
  ```

- 清理编译产物：

  ```bash
  make TOOLPREFIX= clean
  ```

- 在 Shell 中运行 `ls`、`cat README`、`echo hello`；
- 阅读 `Makefile` 中 `kernel`、`fs.img`、`xv6.img` 和 `qemu` 的规则；
- 记录内核初始化函数的调用顺序。

### 自检问题

- 用户程序被放进了哪个镜像？
- 内核和文件系统是否是同一个镜像？
- xv6 启动后第一个用户进程是什么？

## 阶段二：从 `init` 和 Shell 开始

### 目标

理解 xv6 用户态程序的组织方式，以及 Unix Shell 的基本工作模式。

### 阅读文件

```text
init.c
sh.c
echo.c
ls.c
cat.c
user.h
ulib.c
printf.c
umalloc.c
```

### 重点调用

```text
init
→ fork
→ exec("sh")

sh
→ fork
→ exec
→ wait
```

### 实践任务

依次观察以下命令的行为：

```sh
echo hello
ls
cat README
ls | grep xv6
cat README > out
```

尝试回答：

- Shell 为什么需要 `fork()`？
- 为什么执行新程序要使用 `exec()`？
- Shell 如何等待子进程结束？
- 管道和重定向分别修改了什么？

## 阶段三：系统调用路径

### 目标

理解用户态函数如何进入内核，并被分派到具体的内核实现。

### 阅读文件

```text
user.h
usys.S
syscall.h
syscall.c
sysproc.c
sysfile.c
trapasm.S
trap.c
```

### 重点追踪

```text
fork
exec
exit
wait
open
read
write
close
pipe
```

### 通用调用链

```text
用户程序
→ user.h 中的声明
→ usys.S 中的系统调用入口
→ CPU 陷阱/中断入口
→ trap.c
→ syscall.c
→ sysproc.c 或 sysfile.c
```

### 实践任务

选择 `write()` 和 `fork()` 各画一张调用链图，并标出：

- 用户态代码；
- 汇编入口；
- 陷阱处理；
- 系统调用分派；
- 具体内核实现。

## 阶段四：进程、调度和上下文切换

### 目标

理解进程控制块、进程状态、调度器和上下文切换。

### 阅读文件

```text
proc.h
proc.c
swtch.S
sysproc.c
forktest.c
zombie.c
```

### 重点函数

```text
allocproc()
userinit()
fork()
scheduler()
sched()
yield()
sleep()
wakeup1()
exit()
wait()
```

### 进程状态

```text
UNUSED → EMBRYO → RUNNABLE → RUNNING
                         ↑        ↓
                       SLEEPING ←─┘

RUNNING → ZOMBIE
```

### 实践任务

- 阅读 `forktest.c`，观察进程数量如何增长；
- 阅读 `zombie.c`，理解父进程不调用 `wait()` 时的状态；
- 在 `scheduler()`、`fork()`、`exit()` 附近设置断点；
- 记录一个进程从创建到退出的状态变化。

## 阶段五：虚拟内存和地址空间

### 目标

理解 xv6 如何给每个进程建立独立的虚拟地址空间。

### 阅读文件

```text
kalloc.c
vm.c
mmu.h
memlayout.h
proc.c
```

### 重点函数

```text
kalloc()
kfree()
setupkvm()
inituvm()
allocuvm()
deallocuvm()
copyuvm()
switchuvm()
switchkvm()
walkpgdir()
loaduvm()
```

### 重点问题

- 物理页如何分配和释放？
- 内核地址空间如何映射？
- 用户程序的代码、数据和栈如何放置？
- `fork()` 如何复制父进程地址空间？
- 上下文切换时为什么要切换页表？

### 实践任务

画出一个用户进程的地址空间示意图，标出：

```text
代码段
数据段
堆
用户栈
内核映射
```

## 阶段六：文件描述符、管道和文件对象

### 目标

理解 Unix 中“文件描述符”这一核心抽象。

### 阅读文件

```text
file.h
file.c
pipe.c
sysfile.c
```

### 重点函数

```text
filealloc()
filedup()
fileclose()
fileread()
filewrite()
pipealloc()
pipe_read()
pipe_write()
```

### 实践任务

解释以下命令的内部过程：

```sh
cat README | grep xv6
```

重点说明：

- 父进程创建了什么管道；
- 两个子进程分别继承了哪些文件描述符；
- 为什么要关闭不使用的管道端；
- `dup()` 如何实现标准输入或标准输出重定向。

## 阶段七：文件系统

### 目标

理解文件名、目录、inode、文件描述符和磁盘块之间的关系。

### 阅读文件

```text
fs.h
bio.c
buf.h
log.c
fs.c
file.c
sysfile.c
ide.c
mkfs.c
```

### 推荐顺序

```text
fs.h
→ bio.c
→ log.c
→ fs.c
→ file.c
→ sysfile.c
→ ide.c
→ mkfs.c
```

### 重点概念

- 磁盘块；
- 块缓存；
- inode；
- 目录项；
- 路径解析；
- 日志和事务；
- 文件描述符；
- 链接和引用计数。

### 实践任务

追踪下面的调用：

```text
open("a", O_CREATE | O_WRONLY)
→ sys_open()
→ namei() / nameiparent()
→ ialloc()
→ iunlockput()
```

然后回答：

- 文件名如何解析成 inode？
- 目录本身为什么也可以看成一种文件？
- 文件内容最终如何读写磁盘块？
- 为什么文件系统需要日志？

## 阶段八：启动、中断和设备

### 目标

理解 xv6 如何从 CPU 上电进入 C 语言内核，以及硬件事件如何进入内核。

### 阅读文件

```text
bootasm.S
bootmain.c
entry.S
initcode.S
vectors.pl
trapasm.S
trap.c
uart.c
console.c
kbd.c
ide.c
lapic.c
ioapic.c
```

### 阅读顺序

```text
bootasm.S
→ bootmain.c
→ entry.S
→ main.c
→ vectors.pl
→ trapasm.S
→ trap.c
```

### 实践任务

分别追踪：

- 内核启动过程；
- 键盘输入到 Shell 的过程；
- 串口输出到 QEMU 终端的过程；
- 磁盘块读取到文件系统的过程。

## 阶段九：锁和并发

### 目标

理解多 CPU 环境下为什么需要锁，以及不同锁的适用场景。

### 阅读文件

```text
spinlock.h
spinlock.c
sleeplock.h
sleeplock.c
Notes
```

### 重点问题

- 哪些数据结构是多个 CPU 共享的？
- 哪把锁保护进程表？
- 哪把锁保护物理内存分配器？
- 自旋锁和睡眠锁有什么区别？
- 持有锁时能否睡眠？
- 锁的获取顺序如何避免死锁？

### 实践任务

选择以下任意三个对象，记录其锁、访问者和临界区：

```text
进程表
物理内存分配器
块缓存
inode
管道
控制台
```

## 5. 推荐的核心阅读路线

如果希望用一条最短路线建立整体理解，按下面顺序阅读：

```text
README
→ Makefile
→ main.c
→ init.c
→ sh.c
→ user.h
→ usys.S
→ syscall.c
→ sysproc.c
→ proc.c
→ swtch.S
→ vm.c
→ sysfile.c
→ file.c
→ fs.c
→ bio.c
→ log.c
→ ide.c
```

这条路线遵循：

```text
系统如何启动
→ 用户如何执行命令
→ 系统调用如何进入内核
→ 进程如何运行
→ 内存如何隔离
→ 文件如何存储
```

## 6. 建议的实践项目

完成基础阅读后，可以按难度进行小改动：

### 入门级

- 增加一个用户程序，例如 `hello.c`；
- 修改 `echo`，支持简单选项；
- 为 Shell 增加一个简单内建命令；
- 阅读并解释 `usertests.c` 中的测试。

### 进阶级

- 增加一个简单系统调用；
- 修改进程调度策略；
- 增加进程信息显示功能；
- 修改 `sleep` 或增加更清晰的进程状态输出；
- 为文件系统增加一个简单的统计接口。

### 挑战级

- 实现写时复制 `fork()`；
- 增加共享内存机制；
- 改进 Shell 的命令解析；
- 增加新的调度算法；
- 阅读并尝试移植到 xv6-riscv 的设计。

## 7. 每次学习的记录模板

建议每次阅读一个主题时记录以下内容：

```markdown
# 主题：

## 本次问题

## 阅读文件

## 关键结构体

## 关键函数

## 调用链

## 状态变化

## 锁和并发

## 我的疑问

## 实验结果
```

## 8. 最终学习成果检查

当能够用自己的话回答下面问题时，说明已经掌握了 xv6 的主干：

- xv6 如何从启动代码进入内核？
- 第一个用户进程如何启动 Shell？
- `fork()`、`exec()`、`exit()`、`wait()` 分别做什么？
- 用户态系统调用如何进入内核？
- 调度器如何选择下一个进程？
- 上下文切换保存和恢复了什么？
- 每个进程的地址空间如何建立？
- 文件描述符、文件对象和 inode 有什么关系？
- 管道如何连接两个进程？
- 文件内容如何最终读写到磁盘？
- xv6 中的自旋锁和睡眠锁分别解决什么问题？

## 9. 学习时的总体建议

先把 xv6 当成一个可以观察的实验系统，而不是一大堆 C 文件。每次从一个具体命令开始，沿着函数调用进入内核，再回到用户可见结果。推荐优先掌握以下四条主线：

```text
Shell → 进程 → 系统调用
Shell → 文件描述符 → 文件系统
进程 → 地址空间 → 页表
硬件事件 → 中断 → 内核处理函数
```

掌握这四条主线后，再深入启动汇编、锁、日志和多 CPU 细节，会更容易形成完整的操作系统模型。

## 10. 配套视频和在线资料

### 10.1 MIT 官方课程资料

| 资源 | 链接 | 说明 |
|---|---|---|
| MIT 6.1810 2026 课程主页 | [课程主页](https://pdos.csail.mit.edu/6.1810/2026/) | 当前课程主页，包含课程说明和课程入口 |
| MIT 6.1810 2026 课程日程 | [课程日程](https://pdos.csail.mit.edu/6.1810/2026/schedule.html) | 包含讲次、阅读材料、实验和部分历史视频链接 |
| MIT 6.1810 xv6 官方教材 | [xv6 book](https://pdos.csail.mit.edu/6.1810/2026/xv6/book-riscv-rev5.pdf) | 当前 RISC-V 版本教材，建议配合源码阅读 |
| MIT 6.S081 2020 课程日程 | [2020 schedule](https://pdos.csail.mit.edu/6.828/2020/schedule.html) | 旧课程页面，包含公开 Lecture 视频入口 |
| MIT 6.828 Fall 2014 视频列表 | [YouTube 播放列表](https://www.youtube.com/playlist?list=PLfciLKR3SgqNJKKIKUliWoNBBH1VHL3AP) | 更早期的公开操作系统课程视频 |

截至本计划整理时，MIT 2026 页面没有确认发布完整的 2026 年课堂录播。课程页面明确说明，部分未来日期的 notes/videos 链接是从 2025 年课程复制的材料，且 2026 年课堂要求现场参加。因此，视频学习应以往年公开视频为主，再与当前课程页面和教材对照。

### 10.2 推荐的视频课程

#### MIT 6.S081 Fall 2020 中文字幕合集

- [Bilibili：MIT 6.S081 2020 操作系统（中英文字幕）](https://www.bilibili.com/video/BV19k4y1C7kA)
- [Bilibili：MIT 6.S081 操作系统工程（中文字幕）](https://www.bilibili.com/video/BV1Dy4y1m7ZE/)

合集覆盖的主题包括：

```text
Introduction
OS Organization and System Calls
Page Tables
RISC-V Calling Convention and Stack Frames
System Call Entry/Exit
Page Faults
Interrupts
Multiprocessors and Locks
Thread Switching
Sleep and Wakeup
File Systems
Crash Recovery
Virtual Memory
Networking
RCU
```

这些视频大多基于 **xv6-riscv**，不是当前仓库的 x86 版本。下面这些内容可以直接用于当前仓库的概念学习：

- Unix 用户程序和 Shell；
- 文件描述符和系统调用；
- 进程、调度和上下文切换；
- 页表和虚拟内存的基本思想；
- 锁、睡眠和唤醒；
- 文件系统、日志和崩溃恢复。

下面这些内容需要结合当前 x86 源码重新对照：

- `bootasm.S`、`entry.S` 等启动汇编；
- x86 的中断描述符、APIC 和 PIC；
- x86 页表格式；
- x86 上下文切换汇编；
- x86 设备和多 CPU 初始化。

### 10.3 视频与当前仓库的对应方法

观看视频时，不直接照抄视频中的文件路径，而是按主题映射：

| 视频主题 | 当前 x86 仓库对应文件 |
|---|---|
| 用户程序和 Shell | `user/init.c`、`user/sh.c`、`user/cat.c`、`user/ls.c` |
| 系统调用 | `user/usys.S`、`user/user.h`、`kernel/syscall.c`、`kernel/sysproc.c`、`kernel/sysfile.c` |
| 进程 | `kernel/proc.c`、`kernel/proc.h`、`kernel/swtch.S` |
| 页表 | `kernel/vm.c`、`kernel/mmu.h`、`kernel/memlayout.h` |
| 中断 | `kernel/trap.c`、`kernel/trapasm.S`、`kernel/traps.h` |
| 锁 | `kernel/spinlock.c`、`kernel/sleeplock.c`、`kernel/Notes` 或 `docs/Notes` |
| 文件系统 | `kernel/fs.c`、`kernel/file.c`、`kernel/bio.c`、`kernel/log.c` |
| 磁盘驱动 | `kernel/ide.c`、`kernel/memide.c` |
| 启动过程 | `boot/bootasm.S`、`boot/bootmain.c`、`boot/entry.S`、`boot/initcode.S` |

### 10.4 与助手共同学习的使用方式

后续每次学习可以按下面格式进行：

```text
视频：第几讲、主题
源码：准备阅读的文件
问题：希望解释的概念或调用链
实验：希望运行、修改或调试的内容
```

例如：

```text
视频：Page Tables
源码：kernel/vm.c、kernel/mmu.h、kernel/memlayout.h
问题：fork() 如何复制地址空间？
实验：画出一个 xv6 进程的页表结构
```

后续分析时，优先以当前仓库的实际源码为准；视频用于建立概念和阅读顺序，若视频中的 RISC-V 汇编、寄存器或文件路径与本仓库不一致，应明确区分架构差异。
