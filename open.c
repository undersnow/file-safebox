#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdio.h>
int main()
{
	int fd;
	fd=open("/home/test/file_safe/abc",O_CREAT,0777);
	printf("fd=%d\n",fd);

	return 0;

}
