// xv6 中文导读
// 文件：kernel/date.h
// 文件作用：kernel/date.h 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
struct rtcdate {
  uint second;
  uint minute;
  uint hour;
  uint day;
  uint month;
  uint year;
};
