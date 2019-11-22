# file-safebox
file safe based on overloading system call

make

insmod hook.ko

path：/root/safeBox

已成功hook read，write，openat，rename，unlinkat等系统调用，并成功实现保险箱管理程序的识别

其他程序无法读取保险箱文件夹内文件内容，并且无法从文件外向保险箱传输文件

保险箱内无法创建新文件，也无法删除文件以及查看文件

无法使用ls，mv，cp，rm等命令



管理程序用法：

gcc safe_manager.c  -o  safe_manager

```shell
./safe_manager in  filename     :cp file to /root/safeBox/
./safe_manager out filename     :cp file in /root/safeBox/ to current folder
./safe_manager ls               :check the file list in /root/safeBox/
```


不足：

使用ls命令时，虽无法查看，但ls后接的字符串如果刚好是保险箱内文件的文件名的话，会打印相应文件名，可以被暴力搜索