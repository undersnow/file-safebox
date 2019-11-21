# file-safebox
file safe based on overloading system call

make

insmod hook.ko

path：/root/safeBox

已成功hook read,write,openat系统调用，并成功实现保险箱管理程序的识别

其他程序无法读取保险箱文件夹内文件内容，并且无法从文件外向保险箱传输文件
保险箱内无法创建新文件
无法通过ls命令查看保险箱内的文件列表



管理程序用法：

```shell
./safe_manager in  filename     :cp file to /root/safeBox/
./safe_manager out filename     :cp file in /root/safeBox/ to current folder
./safe_manager ls               :check the file list in /root/safeBox/
```


不足：

其他程序可以删除保险箱内的文件

可以通过ls+文件名直接查看文件属性
