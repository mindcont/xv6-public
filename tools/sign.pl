#!/usr/bin/perl
# xv6 中文导读
# 文件：tools/sign.pl
# 文件作用：为启动扇区填充至 512 字节并写入 0x55AA 签名
# 所属阶段：启动镜像构建阶段
#

open(SIG, $ARGV[0]) || die "open $ARGV[0]: $!";

$n = sysread(SIG, $buf, 1000);

if($n > 510){
  print STDERR "boot block too large: $n bytes (max 510)\n";
  exit 1;
}

print STDERR "boot block is $n bytes (max 510)\n";

$buf .= "\0" x (510-$n);
$buf .= "\x55\xAA";

open(SIG, ">$ARGV[0]") || die "open >$ARGV[0]: $!";
print SIG $buf;
close SIG;

