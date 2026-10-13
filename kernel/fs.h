// xv6 中文导读
// 文件：kernel/fs.h
// 文件作用：kernel/fs.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// On-disk file system format.
// 磁盘上的文件系统格式。
// Both the kernel and user programs use this header file.
// 内核和用户程序都会使用这个头文件。


#define ROOTINO 1  // root i-number；根目录 inode 编号
#define BSIZE 512  // block size；块大小

// Disk layout:
// 磁盘布局：
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// mkfs 计算超级块并创建初始文件系统。超级块
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)；文件系统镜像大小（块数）
  uint nblocks;      // Number of data blocks；数据块数量
  uint ninodes;      // Number of inodes；inode 数量
  uint nlog;         // Number of log blocks；日志块数量
  uint logstart;     // Block number of first log block；第一个日志块编号
  uint inodestart;   // Block number of first inode block；第一个 inode 块编号
  uint bmapstart;    // Block number of first free map block；第一个空闲位图块编号
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
// 磁盘上的 inode 结构。
struct dinode {
  short type;           // File type；文件类型
  short major;          // Major device number (T_DEV only)；主设备号（仅 T_DEV）
  short minor;          // Minor device number (T_DEV only)；次设备号（仅 T_DEV）
  short nlink;          // Number of links to inode in file system；文件系统中指向 inode 的链接数
  uint size;            // Size of file (bytes)；文件大小（字节）
  uint addrs[NDIRECT+1];   // Data block addresses；数据块地址
};

// Inodes per block.
// 每个块中的 inode 数量。
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
// 包含 inode i 的块。
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
// 每个块中的位图位数。
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
// 包含块 b 对应位的空闲位图块。
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
// 目录是一个文件，内容是连续的 dirent 结构。
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

