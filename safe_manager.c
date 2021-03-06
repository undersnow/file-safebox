#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>			
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SAFEPATH "/home/test/safeBox"

char password[80], cmd1[80], cmd2[80];
char cwd[1024];

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

int copyFile(char *from, char *to) {
	int in = -1, out = -1, flag;
	char buffer[1024];

	chdir(cwd);

	printf("copy from %s to %s\n", from, to);

	in = open(from,S_IWUSR);
	if(in == -1)
	{
		printf("open file %s failed!\n", from);
		return -1;
	} 

	out = creat(to,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	if (out == -1)
	{
		printf("create file %s failed!\n", to);
		return -1;
	}

	// copy file content
	while((flag=read(in,buffer,1024))>0)
	{
		write(out,buffer,flag);
	}

	close(in);
	close(out);

	return 0;
}

int is_dir_exist(const char*dir_path){
    if(dir_path==NULL){
        return -1;
    }
    if(opendir(dir_path)==NULL){
        return -1;
    }
    return 0;
}

unsigned int APHash(char*str)
{
    unsigned int hash=0 ;
    int i ;
       
    for(i=0;*str;i++)
    {
        if((i&1)==0)
        {
            hash^=((hash<<7)^(*str++)^(hash>>3));
        }
        else
        {
            hash^=(~((hash<<11)^(*str++)^(hash>>5)));
        }
    }
       
    return(hash % 249997);
}

int main( int argc, char *argv[] )
{
	
	char *from, *to, *name;
        int hashnum=0;
       int judge=is_dir_exist(SAFEPATH);
       if(judge==0){
        printf("find safeBox at /home/test/safeBox\n");
       }else if(judge==-1){
        printf("creating safeBox at /home/test/safeBox\n");
	    int result=mkdir(SAFEPATH,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
            if(result==0)
            {printf("successfully create safeBox\n");
}
            else
            {
      printf("create safeBox failure!!!\n");
               return -1;
             } 
       }else
	{
		printf("wrong safeBox route!!!\n");
	    return -1;	
	}
	getcwd(cwd, 1024);

	printf("Please enter the password: ");
	scanf("%s", password);
        hashnum=APHash(password);
	if (hashnum!= 60178) {
		printf("Wrong password!\n");
		return -1;
	}
	printf("\n\tWelcome to safe manager!\n");

	while(1) {
		fflush(stdin);

		printf(">>> ");
		scanf("%s", cmd1);

		if (strcmp(cmd1,"help") == 0 || strcmp(cmd1,"h") == 0) {
			printf("in  filename    	:cp file into safe box\n");
			printf("out filename    	:cp file from safe box to current folder\n");
			printf("del filename    	:cp file from safe box to current folder\n");
			printf("ls              	:check the file list in the safe box\n");
		}

		else if (strcmp(cmd1,"in") == 0) {
			scanf("%s", cmd2);
			from = cmd2;
			to = (char *)malloc(sizeof(char)*80);
			strcpy(to, SAFEPATH);
			name = strrchr(from,'/');
			if(name == NULL)
			{
				strcat(to,"/");
				strcat(to,from);
			}
			else
				strcat(to,name);

			//printf("copy from %s to %s\n", from, to);

			if (copyFile(from,to) == -1)
				printf("copy file failed!\n");
			else
				printf("copy file successfully!\n");

			free(to);
			to = NULL;
		}

		else if (strcmp(cmd1,"out") == 0) {
			scanf("%s", cmd2);
			to = cmd2;
			from = (char *)malloc(sizeof(char)*80);
			strcpy(from, SAFEPATH);
			name = strrchr(to,'/');
			if(name == NULL)
			{
				strcat(from,"/");
				strcat(from,to);
			}
			else
				strcat(from,name);

			//printf("copy from %s to %s\n", from, to);

			if (copyFile(from,to) == -1)
				printf("copy file failed!\n");
			else
				printf("copy file successfully!\n");

			free(from);
			from = NULL;
		}

		else if (strcmp(cmd1,"del") == 0) {
			scanf("%s", cmd2);
			name = cmd2;
			from = (char *)malloc(sizeof(char)*80);
			strcpy(from,SAFEPATH);
			strcat(from,"/");
			strcat(from,name);

			printf("delete file %s.\n", from);

			if(remove(from) == 0)
				printf("delete successfully!\n");
			else
				printf("delete failed!\n");
		}

		else if (strcmp(cmd1,"ls") == 0) {
			getFileName(SAFEPATH);
		}

		else if (strcmp(cmd1,"exit") == 0) 
			break;

		else
			goto wrong;


		continue;


wrong:

		printf("Invalid syntax. Use 'h' or 'help' for help\n");
	}


	return 0;
}

