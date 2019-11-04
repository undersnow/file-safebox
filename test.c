#include<linux/init.h>
#include<linux/module.h>
#include<linux/version.h>
#include <linux/kallsyms.h>
#include<linux/moduleparam.h>
#include<linux/unistd.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include<linux/string.h>
#include<linux/fs.h>
#include<linux/fdtable.h>
#include<linux/uaccess.h> 
#include<linux/rtc.h>
#include<linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/fs_struct.h>
#include <linux/limits.h>

#define AUDITPATH "/home/test/file_safe"  //保险箱文件
#define MAX_LENGTH 256

MODULE_LICENSE("GPL");
unsigned long** hhh;
asmlinkage long(* origin_openat)(struct pt_regs *regs);
void get_fullname(const char *pathname,char *fullname);
int judge_path(struct pt_regs * regs, char * pathname);

asmlinkage long new_openat(struct pt_regs *regs)//新的openat
{
    
	char buffer[PATH_MAX];
	long nbytes;
	nbytes=strncpy_from_user(buffer,(char*)regs->bx,PATH_MAX);
    
	if(judge_path(regs,buffer))  //判断是否保险箱safe_file 
	{
		if(strncmp(current->comm ,"safe_manager",10)!=0)    //判断是否管理程序
		{
			printk(KERN_ALERT"this is a open\n");
			return -1;
			//nbytes=strncpy_from_user(buffer,(char*)regs->bx,PATH_MAX);

			//printk("%s",*((char __user * __user *)(regs->cx)+1));
		} 
	}
	
	return origin_openat(regs);

}

void get_fullname(const char *pathname,char *fullname)
{
	struct dentry *parent_dentry = current->fs->pwd.dentry;
        char buf[MAX_LENGTH];     
	if (*(parent_dentry->d_name.name)=='/'){
	    strcpy(fullname,pathname);
	    return;
	}	
	for(;;){
	    if (strcmp(parent_dentry->d_name.name,"/")==0)
		buf[0]='\0';
	    else
	        strcpy(buf,parent_dentry->d_name.name);
            strcat(buf,"/");
            strcat(buf,fullname);
            strcpy(fullname,buf);
            if ((parent_dentry == NULL) || (*(parent_dentry->d_name.name)=='/'))
                 break;
            parent_dentry = parent_dentry->d_parent;
	}
	strcat(fullname,pathname);
	return;
}

int judge_path(struct pt_regs * regs, char * pathname)
{
    char fullname[PATH_MAX];
    char auditpath[PATH_MAX];
    memset(fullname, 0, PATH_MAX);
    memset(auditpath, 0, PATH_MAX);
    get_fullname(pathname,fullname);
//    printk("Info: fullname is  %s \n",fullname);
    strcpy(auditpath,AUDITPATH);
    if (strncmp(fullname,auditpath,12) == 0) 
	    {
		printk("right");
		return 1;
		}
	else
		return 0;
}
//内存写保护
void disable_write_protection(void)
{
  unsigned long cr0 = read_cr0();
  clear_bit(16, &cr0);
  write_cr0(cr0);
  printk("disable cr0%lx\n",(unsigned long)cr0);
}

void enable_write_protection(void)
{
  unsigned long cr0 = read_cr0();
  set_bit(16, &cr0);
  write_cr0(cr0);
  printk("enable cr0%lx\n",(unsigned long)cr0);

}
static int __init hello_init(void)
{
    hhh=kallsyms_lookup_name("sys_call_table");

   disable_write_protection();
   origin_openat=(void *)hhh[__NR_openat];
   printk("before %lx\n",(unsigned long)(hhh[__NR_openat]));
   hhh[__NR_openat]=(unsigned long*)new_openat;
   printk("openat %lx\n",(unsigned long)origin_openat);
   printk("new openat %lx\n",(unsigned long)new_openat);
   printk("after %lx\n",(unsigned long)(hhh[__NR_openat])); 
   enable_write_protection();
   {
   printk("%d\n",__NR_openat);
   }
   printk("dasfd %lx\n",(unsigned long)hhh);
 return 0;
}

static void __exit hello_exit(void)
{
    disable_write_protection();
    hhh[__NR_openat] = (unsigned long*)origin_openat;
	enable_write_protection(); 
	printk(KERN_ALERT "Goodbye \n");
}

module_init(hello_init);
module_exit(hello_exit);
