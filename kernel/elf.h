// xv6 中文导读
// 文件：kernel/elf.h
// 文件作用：kernel/elf.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
// Format of an ELF executable file
// ELF 可执行文件格式。

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian；小端表示的 ELF 魔数

// File header；文件头
struct elfhdr {
  uint magic;  // must equal ELF_MAGIC；必须等于 ELF_MAGIC
  uchar elf[12];
  ushort type;
  ushort machine;
  uint version;
  uint entry;
  uint phoff;
  uint shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

// Program section header；程序段头
struct proghdr {
  uint type;
  uint off;
  uint vaddr;
  uint paddr;
  uint filesz;
  uint memsz;
  uint flags;
  uint align;
};

// Values for Proghdr type；程序段类型取值
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags；程序段标志位
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4
