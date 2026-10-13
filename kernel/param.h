// xv6 中文导读
// 文件：kernel/param.h
// 文件作用：kernel/param.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
#define NPROC        64  // maximum number of processes；最大进程数
#define KSTACKSIZE 4096  // size of per-process kernel stack；每个进程内核栈大小
#define NCPU          8  // maximum number of CPUs；最大 CPU 数量
#define NOFILE       16  // open files per process；每个进程打开文件数
#define NFILE       100  // open files per system；系统文件对象总数
#define NINODE       50  // maximum number of active i-nodes；活动 inode 最大数量
#define NDEV         10  // maximum major device number；最大主设备号
#define ROOTDEV       1  // device number of file system root disk；根文件系统磁盘设备号
#define MAXARG       32  // max exec arguments；exec 最大参数数
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks

