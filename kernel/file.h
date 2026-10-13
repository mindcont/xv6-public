// xv6 中文导读
// 文件：kernel/file.h
// 文件作用：kernel/file.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  int ref; // reference count；引用计数
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  uint off;
};


// in-memory copy of an inode
// inode 的内存副本。
struct inode {
  uint dev;           // Device number；设备号
  uint inum;          // Inode number；inode 编号
  int ref;            // Reference count；引用计数
  struct sleeplock lock; // protects everything below here；保护下面的所有字段
  int valid;          // inode has been read from disk?；inode 是否已从磁盘读入

  short type;         // copy of disk inode；磁盘 inode 的副本
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};

// table mapping major device number to
// device functions
// 将主设备号映射到设备操作函数的表。
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
