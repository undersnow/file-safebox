#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>			
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SAFEPATH "/root/safeBox"


void getFileName(char * dirPath)
{
	DIR *dir = opendir(dirPath);
	if (dir == NULL)
	{
		printf("%s\n",strerror(errno));
		return;
	}       
	chdir(dirPath);
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL)
	{
		if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
		{
			continue;
		}
		struct stat st;
		stat(ent->d_name,&st);
		if(S_ISDIR(st.st_mode))
		{
			getFileName(ent->d_name);
		}
		else
		{
			printf("%s\n",ent->d_name);
		}
	}       
	closedir(dir);
	chdir("..");
}

int main( int argc, char *argv[] )
{

	if( argc == 1)
	{
		printf("please use parameter!  use -h for help\n");
		return 0;
	}
	else if(argc==2) 
	{
		if (strncmp(argv[1],"ls",2)==0)
		{
		//list of files in safe

			getFileName(SAFEPATH);

			return 0;
		}
		else if(strncmp(argv[1],"-h",2)==0)
		{
			printf("./safe_manager in  filename     :cp file to /root/safeBox/\n");
			printf("./safe_manager out filename     :cp file in /root/safeBox/ to current folder\n");
			printf("./safe_manager ls               :check the file list in /root/safeBox/\n");
		}
		else 
		{
			printf("wrong list!  %s\n",argv[1]);
			return 0;
		}
	}
	else if (argc==3)
	{

		if(strncmp(argv[1],"in",2)!=0&&strncmp(argv[1],"out",3)!=0)
		{
			printf("wrong !  %s\n",argv[1]);
			return 0;
		}

		char *path1=argv[2];
		char path2[80]=SAFEPATH;
		char *p=strrchr(path1,'/');

		if(p==NULL)
		{
			strcat(path2,"/");
			strcat(path2,path1);
		}
		else
		{
			strcat(path2,p);
		}
		//printf("5wr %s ",aa);

		int in=-1;
		int out=-1;
		int flag;
		char buffer[1024];

		if(strncmp(argv[1],"out",3) == 0)
		{
			in = open(path2,S_IWUSR);
			if(in == -1)
			{ 
				printf("open file %s failed!\n", path2); 
				return -1;
			}
			out = creat(path1,S_IWUSR|S_IRUSR);
			if (out == -1) 
			{ 
				printf("create file %s failed!\n", path1); 
				return -1;
			}
		}
		else if(strncmp(argv[1],"in",2) == 0)
		{
			in = open(path1,S_IWUSR);
			if(in == -1)
			{
				printf("open file %s failed!\n", path1);
				return -1;
			} 
			out = creat(path2,S_IWUSR|S_IRUSR);
			if (out == -1)
			{
				printf("create file %s failed!\n", path2);
				return -1;
			}
		}

		// copy file content
		while((flag=read(in,buffer,1024))>0)
		{
			write(out,buffer,flag);
		}

		close(in);
		close(out);
		
		printf("copy file successfully!\n");
	}
	else
	{
		printf("wrong!\n");
	}

	return 0;
}

