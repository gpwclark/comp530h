#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "urrsched.h" /* used by both kernel module and user program */


typedef struct __urrsched_ps_t urrsched_ps_t;
struct __urrsched_ps_t {
    struct list_head mylist ;
    int pid;
    int weight;
    ktime_t start;
    ktime_t end;
};
LIST_HEAD(ps_info_list);
///////
struct task_struct *call_task = NULL;
char *respbuf;
struct sched_class *user_rr_sched_class;
int file_value;
struct dentry *dir, *file;
//
struct sched_param newParams = {.sched_priority = 1}; 
unsigned int firstCall = 1;
/* This function emulates the handling of a system call by
 * accessing the call string from the user program, executing
 * the requested function and preparing a response.
 */
static urrsched_ps_t *get_ps_info(pid_t pid){
    urrsched_ps_t *position = NULL;
    list_for_each_entry(position, &ps_info_list, mylist){
        if(position->pid == pid){
            return position;
        }
    }
    return NULL;
}

unsigned int (* get_rr_interval_orig) (struct rq *, struct task_struct *);
static void (* task_tick_orig) (struct rq *, struct task_struct *, int);

static void urr_task_tick(struct rq *rq, struct task_struct *p, int queued){
    urrsched_ps_t *mySchedInfo = get_ps_info(p->pid);
    if(mySchedInfo == NULL)
        return;
    mySchedInfo->end = mySchedInfo->start;

    p->rt.time_slice = mySchedInfo->weight * TENMS;//Reset timeslice to weighted
    task_tick_orig(rq, p, queued);
    p->rt.time_slice = mySchedInfo->weight * TENMS;//Reset timeslice to weighted

    mySchedInfo->start = ktime_get();//get a new time
    printk(KERN_DEBUG "urrsched: urr_task_tick PID %i with weight %i DIFFtime %llu\n", p->pid, mySchedInfo->weight,  (u64) ktime_to_ns(ktime_sub(end,start)) );
    return;
}

unsigned int urr_get_rr_interval(struct rq *rq, struct task_struct *task){
    printk(KERN_DEBUG "urrsched: urr_get_rr_interval for PID %i \n", task->pid);
    //int rval = get_rr_interval_orig(rq, task);
    urrsched_ps_t *mySchedInfo = get_ps_info(task->pid);
    int rval = mySchedInfo->weight * TENMS;//Reset timeslice to weighted
    return rval;
}

static ssize_t urrsched_call(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{

    printk(KERN_DEBUG "urrsched: HZ == %i \n", HZ);
	int rc;
	char callbuf[MAX_CALL];
    int callbuf_param1 = -1;
    char * calltemp = NULL;
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

	if (strncmp(callbuf, URRSCHED_CALL, sizeof(URRSCHED_CALL) - 1 ) != 0) {//If we have the wrong call
		sprintf(respbuf, "%i", -EINVAL);//invalid args
		printk(KERN_DEBUG "urrsched: call %s will return %s because of wrong call\n", callbuf, respbuf);
		preempt_enable();
		return count;  /* write() calls return the number of bytes written */
	}
    else{
        //we have a good call
        calltemp = strchr(callbuf,' ');
        calltemp[0] = '\0';
        calltemp++;// we want the pointer after the space
        int convstr = kstrtoint( calltemp, 0, &callbuf_param1 );
        if (convstr != 0 ){
            printk(KERN_DEBUG "urrsched: call %s will return %s the parameter %i was not acceptable\n", callbuf, respbuf, callbuf_param1);
            preempt_enable(); 
            return -ENOSPC;
        }
    }
    //*****Do the scheduling dance****
    int setSched = sched_setscheduler(call_task, SCHED_RR, &newParams); //Set the scheduling policy to SCHED_RR and to the lowest prio
	if (setSched != 0) {//If we have a bad egg
		sprintf(respbuf, "%i", setSched);//invalid args
		printk(KERN_DEBUG "urrsched: call %s will return %s because sched_setscheduler returned error\n", callbuf, respbuf);
		preempt_enable();
		return count;  /* write() calls return the number of bytes written */
	}
    //firstcall
    if(firstCall){//make first call copy
        user_rr_sched_class = kmalloc(sizeof(*(call_task->sched_class) ), GFP_ATOMIC);
        if (user_rr_sched_class == NULL) {
            preempt_enable(); 
            return -ENOSPC;
        }
        printk(KERN_DEBUG "urrsched: urrsched begin firstCall logic PID %i \n", call_task->pid);
        task_tick_orig = call_task->sched_class->task_tick;
        get_rr_interval_orig = call_task->sched_class->get_rr_interval;
        memcpy(user_rr_sched_class, call_task->sched_class, sizeof( *(call_task->sched_class))+1 );        
        user_rr_sched_class->task_tick = urr_task_tick;
        user_rr_sched_class->get_rr_interval = urr_get_rr_interval;
        //printk(KERN_DEBUG "urrsched: for PID %i task_tick_orig: %p get_rr_interval_orig: %p\n", call_task->pid, task_tick_orig, get_rr_interval_orig);
        firstCall = 0;
    }
    //list to keep track of each ps's weight info
    urrsched_ps_t *call_task_info = kmalloc(sizeof(urrsched_ps_t), GFP_ATOMIC);
    INIT_LIST_HEAD(&(call_task_info->mylist));
    call_task_info->pid = call_task->pid;
    call_task_info->weight = callbuf_param1;
    call_task_info->start = ktime_get();
    list_add (&(call_task_info->mylist), &ps_info_list);
    ///Here we set the call task to use our new sched class
    call_task->sched_class = user_rr_sched_class;
    //Response and such
	sprintf(respbuf, "%i", URRSCHED_SCHED_UWRR_SUCCESS);//Success 
	/* Here the response has been generated and is ready for the user
	 * program to access it by a read() call.
	 */
	printk(KERN_DEBUG "urrsched: call %s will return %s", callbuf, respbuf);
	preempt_enable();

	*ppos = 0;  /* reset the offset to zero */
	return count;  /* write() calls return the number of bytes written */
}

/* This function emulates the return from a system call by returning
 * the response to the user.
 */

static ssize_t urrsched_return(struct file *file, char __user *userbuf,
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
	.read = urrsched_return,
	.write = urrsched_call,
};

/* This function is called when the module is loaded into the kernel
 * with insmod.  It creates the directory and file in the debugfs
 * file system that will be used for communication between programs
 * in user space and the kernel module.
 */

static int __init urrsched_module_init(void)
{

	/* create a directory to hold the file */

	dir = debugfs_create_dir(dir_name, NULL);
	if (dir == NULL) {
		printk(KERN_DEBUG "urrsched: error creating %s directory\n", dir_name);
		return -ENODEV;
	}

	/* create the in-memory file used for communication;
	 * make the permission read+write by "world"
	 */


	file = debugfs_create_file(file_name, 0666, dir, &file_value, &my_fops);
	if (file == NULL) {
		printk(KERN_DEBUG "urrsched: error creating %s file\n", file_name);
		return -ENODEV;
	}

	printk(KERN_DEBUG "urrsched: created new debugfs directory and file\n");


	return 0;
}

/* This function is called when the module is removed from the kernel
 * with rmmod.  It cleans up by deleting the directory and file and
 * freeing any memory still allocated.
 */

static void __exit urrsched_module_exit(void)
{
	debugfs_remove(file);
	debugfs_remove(dir);
	if (respbuf != NULL)
		kfree(respbuf);
	if (user_rr_sched_class != NULL)
		kfree(user_rr_sched_class);
    urrsched_ps_t *position = NULL;
    list_for_each_entry(position, &ps_info_list, mylist){
        kfree(position);
    }
}

/* Declarations required in building a module */

module_init(urrsched_module_init);
module_exit(urrsched_module_exit);
MODULE_LICENSE("GPL");
