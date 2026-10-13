// xv6 中文导读
// 文件：kernel/swtch.S
// 文件作用：kernel/swtch.S 的声明、数据结构或实现代码
// 所属阶段：内核实现阶段
//
# Context switch；上下文切换
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# 将当前寄存器保存到栈上，形成
# a struct context, and save its address in *old.
# 一个 struct context，并把其地址保存到 *old。
# context 结构保存内核线程需要恢复的寄存器，并把其地址保存到 *old。
# Switch stacks to new and pop previously-saved registers.
# 切换到 new 指向的栈，并弹出之前保存的寄存器。

.globl swtch
swtch:
  movl 4(%esp), %eax
  movl 8(%esp), %edx

  # Save old callee-saved registers；保存旧上下文中被调用者保存的寄存器
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch stacks；切换栈
  movl %esp, (%eax)
  movl %edx, %esp

  # Load new callee-saved registers；加载新上下文中被调用者保存的寄存器
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  ret
