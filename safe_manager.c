#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>			
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void getFileName(char * dirPath)
{
        DIR *dir=opendir(dirPath);
        if(dir==NULL)
        {
                printf("%s\n",strerror(errno));
                return;
        }       
        chdir(dirPath);
        struct dirent *ent;
        while((ent=readdir(dir))!=NULL)
        {
                if(strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0)
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

		getFileName("/home/test/file_safe/");
	
		return 0;
	    }
                else if(strncmp(argv[1],"-h",2)==0)
                {
                   printf("./safe_manager in  filename     :cp file to /home/test/file_safe/\n");
                   printf("./safe_manager out filename     :cp file in /home/test/file_safe/ to current folder\n");
                   printf("./safe_manager ls               :check the file list in /home/test/file_safe/\n");
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
		char *fn,*p;
               
		char *pathname=argv[2];
                char aa[80]="/home/test/file_safe/";
             
		int in=-1;
		int out=-1;
		int flag;
                p=strrchr(pathname,'/');
                if(p==NULL)
{
strcat(aa,pathname);
}
else
               {
                strcat(aa,p);
               }                
                printf("5wr %s",aa);
             
		char buffer[1024];
		if(strncmp(argv[1],"out",3)==0)
		{
			in=open(aa,S_IWUSR);
                        if(in==-1){printf(" 打开文件失败 !\n");return -1;}
			out=creat(pathname,S_IWUSR|S_IRUSR);
                        printf("d");
		}
		else if(strncmp(argv[1],"in",2)==0)
		{
                        in=open(pathname,S_IWUSR);
                        if(in==-1){printf(" 打开文件失败 !\n");return -1;} 
			out=creat(aa,S_IWUSR|S_IRUSR);
			
		}
		if(-1==out)
		{
			printf("创建文件 %s 失败!\n",pathname);
			return -1;
		}
		while((flag=read(in,buffer,1024))>0)
		{
			write(out,buffer,flag);
		}
		close(in);
		close(out);
		printf("复制文件到%s 完毕!\n",pathname);
	}
	else
	{
		printf("wrong!");
	}

    return 0;
}

