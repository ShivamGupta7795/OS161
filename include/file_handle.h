#include <spinlock.h> 
#include <synch.h>
#include <kern/limits.h>
struct file_handle{
        struct vnode *v;
        struct lock *sys_lock;//sync
        off_t offset;
        int perm;
        int ref_count;
};

struct proc_table{
	struct proc *p;
	pid_t pid;
};

extern struct proc_table *p_table;
extern int fork_prevent;
