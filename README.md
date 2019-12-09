# file-safebox
file safe based on overloading system call

保险箱程序路径：/home/test/safeBox

产生日志文件路径：/home/test/safebox_log



内核模块用法：

make

insmod hook.ko

已成功hook openat，rename，unlinkat等系统调用，并成功实现保险箱管理程序的识别

其他程序无法读取保险箱文件夹内文件内容，并且无法从文件外向保险箱传输文件

保险箱内无法创建新文件，也无法删除文件以及查看文件

无法使用ls，mv，cp，rm等命令



管理程序用法：

编译：gcc safe_manager.c  -o  safe_manager

```shell
./safe_manager    （password：password）
 in  filename     :cp file to /root/safeBox/
 out filename     :cp file in /root/safeBox/ to current folder
 del filename     :delete file in /root/safeBox/ 
 ls               :check the file list in /root/safeBox/
 exit             :exit the program
```


不足：

1.使用ls命令时，虽无法查看，但ls后接的字符串如果刚好是保险箱内文件的文件名的话，会打印相应文件名，可以被暴力搜索；

2.保险箱内可以被其他程序存入文件夹，但无法存入文件。