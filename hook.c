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
MODULE_DESCRIPTION("hook sys_read");

#define SAFEPATH "/root/safeBox"
#define COMMANDNAME "safe_manager"
#define MAX_LENGTH 256

typedef asmlinkage long (*orig_read_t)(struct pt_regs *regs);
typedef asmlinkage long (*orig_write_t)(struct pt_regs *regs);

unsigned long sys_call_table_addr = 0;
orig_read_t old_read = NULL;
orig_write_t old_write = NULL;

unsigned int level;
pte_t *pte;

asmlinkage long hooked_read(struct pt_regs *regs) {
    struct files_struct *files = current->files;
	char *buff = (char*)kmalloc(2048,GFP_KERNEL);
    task_lock(current);
	struct fdtable* fdt = files->fdt;
	char* path = NULL;
	struct file* file = fdt->fd[regs->di];
	if(file != NULL && file->f_path.dentry != NULL) {	
		path=dentry_path_raw(file->f_path.dentry,buff,2048);
		if(strstr(path,"safeBox") && strncmp(current->comm,COMMANDNAME,strlen(COMMANDNAME)) != 0) {
			printk("Find read operation to safeBox: %s. The path is %s. The command name is %s.", SAFEPATH, path, current->comm);
			task_unlock(current);
			kfree(buff);
			return -1;
		}
	}
	task_unlock(current);
    kfree(buff);
    return old_read(regs);
}

asmlinkage long hooked_write(struct pt_regs *regs) {
    struct files_struct *files = current->files;
	char *buff = (char*)kmalloc(2048,GFP_KERNEL);
    task_lock(current);
	struct fdtable* fdt = files->fdt;
	char* path = NULL;
	struct file* file = fdt->fd[regs->di];
	if(file != NULL && file->f_path.dentry != NULL) {	
		path = dentry_path_raw(file->f_path.dentry,buff,2048);
		if(strstr(path,"safeBox") && strncmp(current->comm,COMMANDNAME,strlen(COMMANDNAME)) != 0) {
			printk("Find write operation to safeBox: %s. The path is %s. The command name is %s.", SAFEPATH, path, current->comm);
			task_unlock(current);
			kfree(buff);
			return -1;
		}
	}
    task_unlock(current);
    kfree(buff);
    return old_write(regs);
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
	
    pte = lookup_address((unsigned long) sys_call_table_addr, &level);
 

    set_pte_atomic(pte, pte_mkwrite(*pte));
 
    printk("+ unprotected kernel memory page containing sys_call_table\n");
 
    ((unsigned long * ) (sys_call_table_addr))[__NR_read]= (unsigned long) hooked_read;
    ((unsigned long * ) (sys_call_table_addr))[__NR_write]= (unsigned long) hooked_write;

    printk("+ sys_read hooked!\n");
    printk("+ sys_write hooked!\n");

    set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
 
    return 0;
}

 
static void hooked_exit(void) {
    if(old_read != NULL) {
    	set_pte_atomic(pte, pte_mkwrite(*pte));
    	
        // restore sys_call_table to original state
        ((unsigned long * ) (sys_call_table_addr))[__NR_read] = (unsigned long) old_read;
        ((unsigned long * ) (sys_call_table_addr))[__NR_write] = (unsigned long) old_write;

        set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
    }
    
    printk("+ Unloading hook_read module\n");
}

module_init(hooked_init);
module_exit(hooked_exit);