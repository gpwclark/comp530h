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
#include "usersync.h" /* used by both kernel module and user program */

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

wait_queue_head_t q[MAX_QUEUES];//array of wait queues for all the events
int qindex = 0; //index of next empty spot for a wait queue head

/* This function emulates the handling of a system call by
 * accessing the call string from the user program, executing
 * the requested function and preparing a response.
 */

static ssize_t usersync_call(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	int rc;
	char callbuf[MAX_CALL];
	char * calltemp;
	DEFINE_WAIT(wait);
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

	//parse syscall line
	calltemp = strchr(callbuf,' ');
	calltemp[0] = '\0';
	calltemp++;// we want the pointer after the space
	//calltemp is the string of params
	printk(KERN_DEBUG "usersync: call %s calltemp %s", callbuf, calltemp);
		
	//handle the different calls
	if (strcmp(callbuf, "event_create") ) {
		//only param is name
		//make the wait queue for the event
		init_waitqueue_head(&(q[qindex]));
		printk(KERN_DEBUG "usersync: call %s returns %d", callbuf, qindex);		
		sprintf(respbuf, "%d\n", qindex);
	}
	else if (strcmp(callbuf, "event_id") ) {
	}
	else if (strcmp(callbuf, "event_wait") ) {
	}
	else if (strcmp(callbuf, "event_signal") ) {
	}
	else if (strcmp(callbuf, "event_destroy") ) {
	}
	else{
		strcpy(respbuf, "Failed: invalid operation\n");
		printk(KERN_DEBUG "usersync: call %s will return %s\n", callbuf, respbuf);
		preempt_enable();
		return count;  /* write() calls return the number of bytes written */
	}

	/* Here the response has been generated and is ready for the user
	 * program to access it by a read() call.
	 */


	//printk(KERN_DEBUG "usersync: call %s will return %s", callbuf, respbuf);
	preempt_enable();

	*ppos = 0;  /* reset the offset to zero */
	return count;  /* write() calls return the number of bytes written */
}

/* This function emulates the return from a system call by returning
 * the response to the user.
 */

static ssize_t usersync_return(struct file *file, char __user *userbuf,
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
	.read = usersync_return,
	.write = usersync_call,
};

/* This function is called when the module is loaded into the kernel
 * with insmod.  It creates the directory and file in the debugfs
 * file system that will be used for communication between programs
 * in user space and the kernel module.
 */

static int __init usersync_module_init(void)
{

	/* create a directory to hold the file */

	dir = debugfs_create_dir(dir_name, NULL);
	if (dir == NULL) {
		printk(KERN_DEBUG "usersync: error creating %s directory\n", dir_name);
		return -ENODEV;
	}

	/* create the in-memory file used for communication;
	 * make the permission read+write by "world"
	 */


	file = debugfs_create_file(file_name, 0666, dir, &file_value, &my_fops);
	if (file == NULL) {
		printk(KERN_DEBUG "usersync: error creating %s file\n", file_name);
		return -ENODEV;
	}

	printk(KERN_DEBUG "usersync: created new debugfs directory and file\n");

	return 0;
}

/* This function is called when the module is removed from the kernel
 * with rmmod.  It cleans up by deleting the directory and file and
 * freeing any memory still allocated.
 */

static void __exit usersync_module_exit(void)
{
	debugfs_remove(file);
	debugfs_remove(dir);
	if (respbuf != NULL)
		kfree(respbuf);
}

/* Declarations required in building a module */

module_init(usersync_module_init);
module_exit(usersync_module_exit);
MODULE_LICENSE("GPL");