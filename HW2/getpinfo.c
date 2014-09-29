#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/mm_types.h>
#include <linux/rwsem.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include "getpinfo.h" /* used by both kernel module and user program */

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
char *respbuf;

int file_value;
struct dentry *dir, *file;

/* This function emulates the handling of a system call by
 * accessing the call string from the user program, executing
 * the requested function and preparing a response.
 */

static ssize_t getpinfo_call(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	int rc;
	char callbuf[MAX_CALL];
	char resp_line[MAX_LINE];

	pid_t cur_pid = 0;
	pid_t parent = 0;
	long state = 0;
	long flags = 0x0;
	int priority = 0;
	char* f_name;
	char * d_name;
	struct task_struct *task_pointer = NULL;
	struct list_head *list = NULL;

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

	respbuf = kmalloc(MAX_RESP, GFP_ATOMIC);
	if (respbuf == NULL) {
		preempt_enable(); 
		return -ENOSPC;
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

	if (strcmp(callbuf, "getpinfo") != 0) {
		strcpy(respbuf, "Failed: invalid operation\n");
		printk(KERN_DEBUG "getpinfo: call %s will return %s\n", callbuf, respbuf);
		preempt_enable();
		return count;  /* write() calls return the number of bytes written */
	}

	sprintf(respbuf, "Success:\n");

	/* Use kernel functions for access to pid for a process 
	*/
	//allocate memory for some string buffers
	f_name = kmalloc( (sizeof(char) * MAX_FN) , GFP_ATOMIC); 
	d_name = kmalloc( (sizeof(char) * MAX_DL) , GFP_ATOMIC);

	rcu_read_lock();
	list_for_each(list, &call_task->sibling){
		task_pointer = list_entry(list, struct task_struct, sibling);
		cur_pid = pid_vnr(get_task_pid(task_pointer,PIDTYPE_PID));
		parent = task_pointer->parent->pid;
		state = task_pointer->state;
		flags= task_pointer->flags;
		priority= task_pointer->prio;
		get_task_comm(f_name, task_pointer);
		d_name = d_path(&(task_pointer->fs->pwd), d_name, MAX_DL);

		sprintf(resp_line, "     Current PID %d\n", cur_pid);
		strcat(respbuf, resp_line);

		sprintf(resp_line, "	  parent %d\n", parent);	
		strcat(respbuf, resp_line);

		sprintf(resp_line, "	  state %ld\n", state);	
		strcat(respbuf, resp_line);

		sprintf(resp_line, "	  flags %08lx\n", flags);
		strcat(respbuf, resp_line);

		sprintf(resp_line, "	  priority %d\n", priority);
		strcat(respbuf, resp_line);

		sprintf(resp_line, "	  filename %s\n", f_name);
		strcat(respbuf, resp_line);

		sprintf(resp_line, "	  cwd %s\n", d_name);
		strcat(respbuf, resp_line);	

		down_read(&(task_pointer->mm->mmap_sem));
		sprintf(resp_line, "	  number of vma's %d\n", task_pointer->mm->map_count);
		up_read(&(task_pointer->mm->mmap_sem));
		strcat(respbuf, resp_line);

		down_read(&(task_pointer->mm->mmap_sem));
		sprintf(resp_line, "	  total VM %ld\n", task_pointer->mm->total_vm);
		up_read(&(task_pointer->mm->mmap_sem));
		strcat(respbuf, resp_line);

		down_read(&(task_pointer->mm->mmap_sem));
		sprintf(resp_line, "	  shared VM %ld\n", task_pointer->mm->shared_vm);
		up_read(&(task_pointer->mm->mmap_sem));
		strcat(respbuf, resp_line);

		down_read(&(task_pointer->mm->mmap_sem));
		sprintf(resp_line, "	  executable VM %ld\n", task_pointer->mm->exec_vm);
		up_read(&(task_pointer->mm->mmap_sem));
		strcat(respbuf, resp_line);

		down_read(&(task_pointer->mm->mmap_sem));
		sprintf(resp_line, "	  stack VM %ld\n", task_pointer->mm->stack_vm);
		up_read(&(task_pointer->mm->mmap_sem));
		strcat(respbuf, resp_line);
	}
	rcu_read_unlock();
	kfree(f_name);
	kfree(d_name);
	/* Here the response has been generated and is ready for the user
	 * program to access it by a read() call.
	 */


	printk(KERN_DEBUG "getpinfo: call %s will return %s", callbuf, respbuf);
	preempt_enable();

	*ppos = 0;  /* reset the offset to zero */
	return count;  /* write() calls return the number of bytes written */
}

/* This function emulates the return from a system call by returning
 * the response to the user.
 */

static ssize_t getpinfo_return(struct file *file, char __user *userbuf,
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

	kfree(respbuf);

	respbuf = NULL;
	call_task = NULL;

	preempt_enable();

	*ppos = 0;  /* reset the offset to zero */
	return rc;  /* read() calls return the number of bytes read */
} 

static const struct file_operations my_fops = {
	.read = getpinfo_return,
	.write = getpinfo_call,
};

/* This function is called when the module is loaded into the kernel
 * with insmod.  It creates the directory and file in the debugfs
 * file system that will be used for communication between programs
 * in user space and the kernel module.
 */

static int __init getpinfo_module_init(void)
{

	/* create a directory to hold the file */

	dir = debugfs_create_dir(dir_name, NULL);
	if (dir == NULL) {
		printk(KERN_DEBUG "getpinfo: error creating %s directory\n", dir_name);
		return -ENODEV;
	}

	/* create the in-memory file used for communication;
	 * make the permission read+write by "world"
	 */


	file = debugfs_create_file(file_name, 0666, dir, &file_value, &my_fops);
	if (file == NULL) {
		printk(KERN_DEBUG "getpinfo: error creating %s file\n", file_name);
		return -ENODEV;
	}

	printk(KERN_DEBUG "getpinfo: created new debugfs directory and file\n");

	return 0;
}

/* This function is called when the module is removed from the kernel
 * with rmmod.  It cleans up by deleting the directory and file and
 * freeing any memory still allocated.
 */

static void __exit getpinfo_module_exit(void)
{
	debugfs_remove(file);
	debugfs_remove(dir);
	if (respbuf != NULL)
		kfree(respbuf);
}

/* Declarations required in building a module */

module_init(getpinfo_module_init);
module_exit(getpinfo_module_exit);
MODULE_LICENSE("GPL");
