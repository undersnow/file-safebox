#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/utsname.h>
#include <asm/pgtable.h>
#include <linux/kallsyms.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/uaccess.h> 
#include <linux/rtc.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/fs_struct.h>
#include <linux/limits.h> 

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hook syscalls");

#define SAFEPATH "/root/safeBox"
#define SAFENAME "safeBox"
#define COMMANDNAME "safe_manager"
#define MAX_LENGTH 256

typedef asmlinkage long (*orig_read_t)(struct pt_regs *regs);
typedef asmlinkage long (*orig_write_t)(struct pt_regs *regs);
typedef asmlinkage long (*orig_openat_t)(struct pt_regs *regs);
typedef asmlinkage long (*orig_rename_t)(struct pt_regs *regs);
unsigned long sys_call_table_addr = 0;
orig_read_t old_read = NULL;
orig_write_t old_write = NULL;
orig_openat_t old_openat = NULL;
orig_rename_t old_rename = NULL;

unsigned int level;
pte_t *pte;


void get_path_from_dentry(char *path, struct dentry *dentry) {
	char *buf = (char*)kmalloc(MAX_LENGTH,GFP_KERNEL);
	if (strcmp(dentry->d_name.name,"/") == 0) {
		strcpy(path,dentry->d_name.name);
		return;
	}

	strcpy(path,dentry->d_name.name);
	while ((dentry = dentry->d_parent) != NULL) {
		strcpy(buf,dentry->d_name.name);
		if (strcmp(buf,"/") != 0) {
			strcat(buf,"/");
		}
		strcat(buf,path);
		strcpy(path,buf);
		if (strcmp(dentry->d_name.name,"/") == 0) {
			break;
		}
	}
}

asmlinkage long hooked_read(struct pt_regs *regs) {
    struct files_struct *files = current->files;
	struct fdtable* fdt = files->fdt;
	char *path = NULL;
	char *buff = (char*)kmalloc(PATH_MAX,GFP_KERNEL);
	struct file* file = fdt->fd[regs->di];
	
    task_lock(current);
	if(file != NULL && file->f_path.dentry != NULL) {
		//get_path_from_dentry(path,file->f_path.dentry);
		path = dentry_path_raw(file->f_path.dentry,buff,PATH_MAX);
		if(strncmp(path,SAFEPATH,strlen(SAFEPATH)) == 0 || strstr(path,SAFENAME)) {
			if (strncmp(current->comm,COMMANDNAME,strlen(COMMANDNAME)-1) != 0) {
				printk("Find read operation to safeBox: %s The path is %s The command name is %s", SAFEPATH, path, current->comm);
				kfree(buff);
				task_unlock(current);
				return -1;
			}
		}
	}

	kfree(buff);
	task_unlock(current);
    return old_read(regs);
}

asmlinkage long hooked_write(struct pt_regs *regs) {
    struct files_struct *files = current->files;
	struct fdtable* fdt = files->fdt;
	char *path = NULL;
	char *buff = (char*)kmalloc(PATH_MAX,GFP_KERNEL);
	struct file* file = fdt->fd[regs->di];

    task_lock(current);
	if(file != NULL && file->f_path.dentry != NULL) {	
		//get_path_from_dentry(path,file->f_path.dentry);
		path = dentry_path_raw(file->f_path.dentry,buff,PATH_MAX);
		if(strncmp(path,SAFEPATH,strlen(SAFEPATH)) == 0 || strstr(path,SAFENAME)) {
			if (strncmp(current->comm,COMMANDNAME,strlen(COMMANDNAME)-1) != 0) {
				printk("Find write operation to safeBox: %s The path is %s The command name is %s", SAFEPATH, path, current->comm);
				kfree(buff);
				task_unlock(current);
				return -1;
			}
		}
	}

	kfree(buff);
    task_unlock(current);
    return old_write(regs);
}

asmlinkage long hooked_openat(struct pt_regs *regs) {
	char *name = (char*)kmalloc(MAX_LENGTH,GFP_KERNEL);
	char *path = (char*)kmalloc(MAX_LENGTH,GFP_KERNEL);
	strncpy_from_user(name,(char*)regs->si,MAX_LENGTH);

	task_lock(current);

	//absolute path
	if (strncmp(name,"/",1) == 0) {
		strcpy(path,name);
	}
	//relative path
	else if ((int)regs->di == AT_FDCWD) {
		struct dentry *parent_dentry = current->fs->pwd.dentry;
		get_path_from_dentry(path,parent_dentry);
		//strcpy(path,parent_dentry->d_name.name);
		if (strcmp(path,"/") != 0) {
			strcat(path,"/");
		}
		strcat(path,name);
	}
	else {
		goto end;
	}
	
	if (strncmp(path,SAFEPATH,strlen(SAFEPATH)) == 0 || strstr(path,SAFENAME)) {
		if (strncmp(current->comm,COMMANDNAME,strlen(COMMANDNAME)-1) != 0) {
			printk("Find openat operation to safeBox: %s The path is %s", SAFEPATH, path);
			kfree(name);
			kfree(path);
			task_unlock(current);
			return -1;
		}
	}

end:
	kfree(name);
	kfree(path);
	task_unlock(current);
	return old_openat(regs);
}

asmlinkage long hooked_rename(struct pt_regs *regs) {
	char *dst = (char*)kmalloc(MAX_LENGTH,GFP_KERNEL);
	char *src = (char*)kmalloc(MAX_LENGTH,GFP_KERNEL);
	strncpy_from_user(dst,(char*)regs->si,MAX_LENGTH);
        strncpy_from_user(src,(char*)regs->di,MAX_LENGTH);
        if(strstr(dst,SAFENAME)||strstr(src,SAFENAME))
              {return -1;}	
	return old_rename(regs);
}

static int obtain_sys_call_table_addr(unsigned long * sys_call_table_addr) {
	int ret = 1;
	unsigned long temp_sys_call_table_addr;
 
	temp_sys_call_table_addr = kallsyms_lookup_name("sys_call_table");
	
	if (0 == sys_call_table_addr) {
		ret = -1;
		goto cleanup;
	}
	
	printk("Found sys_call_table: %p", (void *) temp_sys_call_table_addr);
	*sys_call_table_addr = temp_sys_call_table_addr;
		
cleanup:
	return ret;
}
 
static int hooked_init(void) {
	int ret = -1;
    printk("+ Loading hook_mkdir module\n");
	ret = obtain_sys_call_table_addr(&sys_call_table_addr);
    if(ret != 1){
		printk("- unable to locate sys_call_table\n");
		return 0;
	}

    printk("+ found sys_call_table at %08lx!\n", sys_call_table_addr);
 
    old_read = ((orig_read_t *)(sys_call_table_addr))[__NR_read];
    old_write = ((orig_write_t *)(sys_call_table_addr))[__NR_write]; 
    old_openat = ((orig_openat_t *)(sys_call_table_addr))[__NR_openat]; 
    old_rename = ((orig_rename_t *)(sys_call_table_addr))[__NR_rename]; 
	
    pte = lookup_address((unsigned long)sys_call_table_addr, &level);
 
    set_pte_atomic(pte, pte_mkwrite(*pte));
 
    printk("+ unprotected kernel memory page containing sys_call_table\n");
 
    ((unsigned long * ) (sys_call_table_addr))[__NR_read]= (unsigned long) hooked_read;
    ((unsigned long * ) (sys_call_table_addr))[__NR_write]= (unsigned long) hooked_write;
    ((unsigned long * ) (sys_call_table_addr))[__NR_openat]= (unsigned long) hooked_openat;
    ((unsigned long * ) (sys_call_table_addr))[__NR_rename]= (unsigned long) hooked_rename;
    printk("+ sys_read hooked!\n");
    printk("+ sys_write hooked!\n");
    printk("+ sys_openat hooked!\n");
    printk("+ sys_rename hooked!\n");
	
    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
 
    return 0;
}

 
static void hooked_exit(void) {
	pte = lookup_address((unsigned long)sys_call_table_addr, &level);
	
    set_pte_atomic(pte, pte_mkwrite(*pte));
    	
    // restore sys_call_table to original state
    ((unsigned long * ) (sys_call_table_addr))[__NR_read] = (unsigned long) old_read;
    ((unsigned long * ) (sys_call_table_addr))[__NR_write] = (unsigned long) old_write;
    ((unsigned long * ) (sys_call_table_addr))[__NR_openat] = (unsigned long) old_openat;
    ((unsigned long * ) (sys_call_table_addr))[__NR_rename] = (unsigned long) old_rename;
    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
    
    printk("+ Unloading hook.ko\n");
}

module_init(hooked_init);
module_exit(hooked_exit);