// xv6 中文导读
// 文件：kernel/stat.h
// 文件作用：kernel/stat.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device

struct stat {
  short type;  // Type of file；文件类型
  int dev;     // File system's disk device；文件系统磁盘设备
  uint ino;    // Inode number；inode 编号
  short nlink; // Number of links to file；指向文件的链接数
  uint size;   // Size of file in bytes；文件大小（字节）
};
