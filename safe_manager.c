#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
int main()
{
    const char* pathName="/home/test/file_safe/test.c";
    int in,out,flag;
    char buffer[1024];
    in=open("/home/test/Desktop/open.c",O_RDONLY,S_IRUSR);
    if(-1==in)
    {
        printf(" 打开文件b.c失败 !\n");
        return -1;
    }
    out=creat(pathName,S_IWUSR);
    if(-1==in)
    {
        printf("创建文件 %s 失败!\n",pathName);
        return -1;
    }
    while((flag=read(in,buffer,1024))>0)
    {
        write(out,buffer,flag);
    }
    close(in);
    close(out);
    printf("复制文件b.c到%s 完毕!\n",pathName);
    return 0;
}

