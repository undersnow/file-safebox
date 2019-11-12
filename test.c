#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/utsname.h>
#include <asm/pgtable.h>
#include <linux/kallsyms.h>
#include <linux/proc_fs.h>
#include<linux/init.h>
#include<linux/version.h>
#include<linux/moduleparam.h>
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

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hook sys_read");

unsigned long sys_call_table_addr = 0;
typedef asmlinkage long (*orig_read)(struct pt_regs *regs);
orig_read old_read = NULL;
typedef asmlinkage long (*orig_write)(struct pt_regs *regs);
orig_write old_write = NULL;

asmlinkage long hooked_read(struct pt_regs *regs) {
    struct files_struct *files=current->files;
    int i=0;
	char *buff=(char*)kmalloc(2048,GFP_KERNEL);
	if(NULL!=files)
	{
        task_lock(current);
		struct fdtable* fdt=files->fdt;				
		i=0;
		for(;i<NR_OPEN_DEFAULT;i++)//默认打开表是NR_OPEN_DEFAULT
		{
			char* path=NULL;
			struct file* file=fdt->fd[i];
			if(file!=NULL&&file->f_path.dentry!=NULL)
			{	
				path=dentry_path_raw(file->f_path.dentry,buff, 2048);                           
				if(strstr(path,"file_safe"))
                {   
			        if (strncmp(current->comm,"safe_manager",10)!=0)
					{	
						task_unlock(current);
						return -1;
					}
				}
			}
		}
    task_unlock(current);
	}
    kfree(buff);
    return old_read(regs);
}

asmlinkage long hooked_write(struct pt_regs *regs) {
    struct files_struct *files=current->files;
    int i=0;
	char *buff=(char*)kmalloc(2048,GFP_KERNEL);

	if(NULL!=files)
	{
        task_lock(current);
		struct fdtable* fdt=files->fdt;				
		i=0;
		for(;i<NR_OPEN_DEFAULT;i++)//默认打开表是NR_OPEN_DEFAULT
		{
			char* path=NULL;
			struct file* file=fdt->fd[i];
			if(file!=NULL&&file->f_path.dentry!=NULL)
			{	
				path=dentry_path_raw(file->f_path.dentry,buff, 2048);           // file->d_inode->i_nlink=0;               
				if(strstr(path,"file_safe"))
                {   
			        
			        if(strncmp(current->comm,"safe_manager",10)!=0)
					{	
						task_unlock(current);
						//file->f_path.dentry->d_name.name=".hhh";
						return -1;
					}
				}
			}
		}
    task_unlock(current);
	}
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

unsigned int level;
pte_t *pte;
 

static int hooked_init(void) {
    printk("+ Loading hook_mkdir module\n");
	int ret = -1;
	ret = obtain_sys_call_table_addr(&sys_call_table_addr);
    if(ret != 1){
	printk("- unable to locate sys_call_table\n");
	return 0;
	}

    printk("+ found sys_call_table at %08lx!\n", sys_call_table_addr);
 
    old_read = ((unsigned long * ) (sys_call_table_addr))[__NR_read];
    old_write = ((unsigned long * ) (sys_call_table_addr))[__NR_write]; 
	
    pte = lookup_address((unsigned long) sys_call_table_addr, &level);
 

    set_pte_atomic(pte, pte_mkwrite(*pte));
 
    printk("+ unprotected kernel memory page containing sys_call_table\n");
 
    ((unsigned long * ) (sys_call_table_addr))[__NR_read]= (unsigned long) hooked_read;
    ((unsigned long * ) (sys_call_table_addr))[__NR_write]= (unsigned long) hooked_write;

    printk("+ sys_read hooked!\n");
 
    return 0;
}

 
static void hooked_exit(void) {
    if(old_read != NULL) {
        // restore sys_call_table to original state
        ((unsigned long * ) (sys_call_table_addr))[__NR_read] = (unsigned long) old_read;
        ((unsigned long * ) (sys_call_table_addr))[__NR_write] = (unsigned long) old_write;

        set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
    }
    
    printk("+ Unloading hook_read module\n");
}

module_init(hooked_init);
module_exit(hooked_exit);