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
/*
** module macros
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("hook sys_read");
 
/*
** module constructor/destructor
*/
//typedef void (*sys_call_ptr_t)(void);
//sys_call_ptr_t *_sys_call_table = NULL;
unsigned long sys_call_table_addr = 0;
typedef asmlinkage long (*orig_read)(struct pt_regs *regs);
orig_read old_read = NULL;
 
// hooked mkdir function
asmlinkage long hooked_read(struct pt_regs *regs) {
    struct files_struct *files=current->files;
    int i=0;
	char *buf=(char*)kmalloc(2048,GFP_KERNEL);
	
	if(buf==NULL)
		return old_read(regs);
	
	if(NULL!=files)
	{
		struct fdtable* fdt=files->fdt;				
		i=0;
		for(;i<NR_OPEN_DEFAULT;i++)//默认打开表是NR_OPEN_DEFAULT
		{
			char* path=NULL;
			struct file* file=fdt->fd[i];
			if(file!=NULL&&file->f_path.dentry!=NULL)
			{	
				char*result=NULL;
				if(file->f_path.dentry->d_iname!=NULL)
				{
					//result=strstr(file->f_path.dentry->d_iname,"file_safe");//查找制定类型的文件
					path=dentry_path_raw(file->f_path.dentry,buf, 2048);
					if(strstr(path,"file_safe"))//识别出保险箱
						return -1;
				}
			}
		}


	}
	kfree(buf);

    return old_read(regs);
}



static int obtain_sys_call_table_addr(unsigned long * sys_call_table_addr) {
	int ret = 1;
	unsigned long temp_sys_call_table_addr;
 
	temp_sys_call_table_addr = kallsyms_lookup_name("sys_call_table");
	
	/* Return error if the symbol doesn't exist */
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
 
// initialize
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
    pte = lookup_address((unsigned long) sys_call_table_addr, &level);
 
    set_pte_atomic(pte, pte_mkwrite(*pte));//写保护
 
    printk("+ unprotected kernel memory page containing sys_call_table\n");
    ((unsigned long * ) (sys_call_table_addr))[__NR_read]= (unsigned long) hooked_read;
    printk("+ sys_read hooked!\n");
 
    return 0;
}
 
static void hooked_exit(void) {
    if(old_read != NULL) {
      
        ((unsigned long * ) (sys_call_table_addr))[__NR_read] = (unsigned long) old_read;
        set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));//写保护
    }
    
    printk("+ Unloading hook_read module\n");
}
 
module_init(hooked_init);
module_exit(hooked_exit);