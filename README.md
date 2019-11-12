# file-safebox
file safe based on overloading system call

make

insmod test.ko

path：/home/test/file_safe

已成功hook read,write系统调用，并成功实现保险箱管理程序的识别

其他程序无法读取保险箱文件夹内文件内容，并且无法从文件外向保险箱传输文件



不足：

其他程序可以使用ls命令等查看文件名

其他程序可以删除

其他程序向保险箱复制文件时，会留下一个大小为0的文件，可在保险箱管理程序中使用查看命令时删除