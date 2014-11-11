#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include "vmlogger.h" /* used by both kernel module and user program */

/* The following two variables are global state shared between
 * the "call" and "return" functions.  They need to be protected
 * from re-entry caused by kernel preemption.
 */

/* The call_task variable is used to ensure that the result is
 * returned only to the process that made the call.  Only one
 * result can be pending for return at a time (any call entry 
 * while the variable is non-NULL is rejected).
 */

struct task_struct *call_task = NULL;
char respbuf[MAX_RESP];

int file_value;
struct dentry *dir, *file;

struct vm_operations_struct *my_vm_ops = NULL;
int (* old_fault)(struct vm_area_struct *vma, struct vm_fault *vmf); // function pointer to a fault handler -- to use in wrapper function

int my_fault(struct vm_area_struct *vma, struct vm_fault *vmf){//custom fault handler function
    int rval = 0;
    if(old_fault != NULL){
        rval = old_fault(vma, vmf);
    }
    return rval;

}
/* This function emulates the handling of a system call by
 * accessing the call string from the user program, executing
 * the requested function and preparing a response.
 */

static ssize_t vmlogger_call(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	int rc;
	char callbuf[MAX_CALL];


	/* the user's write() call should not include a count that exceeds
	 * the size of the module's buffer for the call string.
	 */

	if(count >= MAX_CALL)
		return -EINVAL;

	/* The preempt_disable() and preempt_enable() functions are used in the
	 * kernel for preventing preemption.  They are used here to protect
	 * global state.
	 */

	preempt_disable();

	if (call_task != NULL) {
		preempt_enable(); 
		return -EAGAIN;
	}

	strcpy(respbuf,""); /* initialize buffer with null string */

	/* current is global for the kernel and contains a pointer to the
	 * running process
	 */
	call_task = current;
	
	/* Use the kernel function to copy from user space to kernel space.
	*/

	rc = copy_from_user(callbuf, buf, count);
	callbuf[MAX_CALL - 1] = '\0'; /* make sure it is a valid string */

	if (strcmp(callbuf, "vmlogger") != 0) {
		sprintf(respbuf, "Failed invalid call");
		printk(KERN_DEBUG "vmlogger: call %s will return %s\n", callbuf, respbuf);
		preempt_enable();
		return count;  /* write() calls return the number of bytes written */
	}

	sprintf(respbuf, "0");
    //Save some of our mm info
    my_vm_ops = kmalloc(sizeof(call_task->mm->mmap->vm_ops), GFP_ATOMIC);
    if(my_vm_ops == NULL){
		sprintf(respbuf, "Failed, ENOSPC");
		printk(KERN_DEBUG "vmlogger: Exit on my_vm_ops == %p\n", my_vm_ops);
		preempt_enable(); 
		return -ENOSPC;
    }
    memcpy(my_vm_ops ,&call_task->mm->mmap->vm_ops, sizeof(call_task->mm->mmap->vm_ops));
	printk(KERN_DEBUG "vmlogger: memcpy to my_vm_ops at %p", &my_vm_ops);
    old_fault = call_task->mm->mmap->vm_ops->fault; //make pointer to orig function so we can call it later
    if(old_fault != NULL){
        my_vm_ops->fault = my_fault; //set custom struct pointer (for the fault function) to our custom function)
    }
    if(my_vm_ops != NULL){
        call_task->mm->mmap->vm_ops = my_vm_ops;
    }
	/* Use kernel functions for access to pid for a process 
	*/

	//sprintf(respbuf, "	   \n", );
	/* Here the response has been generated and is ready for the user
	 * program to access it by a read() call.
	 */


    sprintf(respbuf, "Success");
	printk(KERN_DEBUG "vmlogger: call %s will return %s", callbuf, respbuf);
	preempt_enable();

	*ppos = 0;  /* reset the offset to zero */
	return count;  /* write() calls return the number of bytes written */
}

/* This function emulates the return from a system call by returning
 * the response to the user.
 */

static ssize_t vmlogger_return(struct file *file, char __user *userbuf,
		size_t count, loff_t *ppos)
{
	int rc; 

	preempt_disable();

	if (current != call_task) {
		preempt_enable();
		return 0;
	}

	rc = strlen(respbuf) + 1; /* length includes string termination */

	/* return at most the user specified length with a string 
	 * termination as the last byte.  Use the kernel function to copy
	 * from kernel space to user space.
	 */

	if (count < rc) {
		respbuf[count - 1] = '\0';
		rc = copy_to_user(userbuf, respbuf, count);
	}
	else 
		rc = copy_to_user(userbuf, respbuf, rc);


	strcpy(respbuf, "");//clear respbuff, it is array so do this
	call_task = NULL;


    if(my_vm_ops != NULL){
        kfree(my_vm_ops);
    }
    my_vm_ops = NULL;

	preempt_enable();

	*ppos = 0;  /* reset the offset to zero */
	return rc;  /* read() calls return the number of bytes read */
} 

static const struct file_operations my_fops = {
	.read = vmlogger_return,
	.write = vmlogger_call,
};

/* This function is called when the module is loaded into the kernel
 * with insmod.  It creates the directory and file in the debugfs
 * file system that will be used for communication between programs
 * in user space and the kernel module.
 */

static int __init vmlogger_module_init(void)
{

	/* create a directory to hold the file */

	dir = debugfs_create_dir(dir_name, NULL);
	if (dir == NULL) {
		printk(KERN_DEBUG "vmlogger: error creating %s directory\n", dir_name);
		return -ENODEV;
	}

	/* create the in-memory file used for communication;
	 * make the permission read+write by "world"
	 */


	file = debugfs_create_file(file_name, 0666, dir, &file_value, &my_fops);
	if (file == NULL) {
		printk(KERN_DEBUG "vmlogger: error creating %s file\n", file_name);
		return -ENODEV;
	}

	printk(KERN_DEBUG "vmlogger: created new debugfs directory and file\n");

	return 0;
}

/* This function is called when the module is removed from the kernel
 * with rmmod.  It cleans up by deleting the directory and file and
 * freeing any memory still allocated.
 */

static void __exit vmlogger_module_exit(void)
{
	debugfs_remove(file);
	debugfs_remove(dir);
    if(my_vm_ops != NULL){
        kfree(my_vm_ops);
    }
}

/* Declarations required in building a module */

module_init(vmlogger_module_init);
module_exit(vmlogger_module_exit);
MODULE_LICENSE("GPL");
