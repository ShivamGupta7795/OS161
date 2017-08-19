#include <types.h>
#include <syscall.h>
#include <vnode.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <file_syscalls.h>
#include <copyinout.h>
#include <spl.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <kern/errno.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <addrspace.h>
#include <mips/trapframe.h>
#include <kern/wait.h>
#include <limits.h>

int fork_prevent=0;
struct proc_table *p_table;
int
sys_write(int fd, const void *buf, size_t buflen, int *err1)
{

	int result;
	int result1;
	struct iovec iov;
	struct uio ku;
	//struct addrspace *as;
	struct proc *proc = curproc;
	struct vnode *vn;
	void *dest;
	dest=kmalloc(buflen);

	if(fd<0 || fd>=OPEN_MAX|| proc->file_table[fd]==NULL){
		kfree(dest);
		*err1 = EBADF;
		return -1;
	}

	//int in = proc->file_table[fd]->perm&000011;
	if(!(proc->file_table[fd]->perm & O_ACCMODE)){
		kfree(dest);
		*err1 = EBADF;
		return -1;
	}

	if(buf == NULL || buflen<=0){
		kfree(dest);
		*err1 = EFAULT;
		return -1;
	}
	/*if(fd==1)
	{
		
		const char *con="con:";
		char *file_name;
		struct vnode *v0;
		file_name=kstrdup(con);
		//proc->file_table[1]=kmalloc(sizeof(struct file_handle));
		vfs_open(file_name, O_WRONLY, 0, &v0);
		proc->file_table[1]->v=v0;
		proc->file_table[1]->offset=0;
		proc->file_table[1]->perm=O_WRONLY;
		proc->file_table[1]->ref_count=1;
		
		
		//as = proc_getas();
		vn=proc->file_table[fd]->v;
		
	//kprintf("%d", buflen);
		result1=copyin(buf,dest,buflen);
		if(result1!=0)
		{
			*err1=result1;
			return -1;
		}
	
		lock_acquire(proc->file_table[fd]->sys_lock);
		uio_kinit(&iov, &ku, dest, buflen, proc->file_table[fd]->offset, UIO_WRITE);
		result = VOP_WRITE(vn, &ku);
		proc->file_table[fd]->offset=ku.uio_offset;
		if (result) {
       	                 kprintf(" Write error: ");
     	                  // vfs_close(vn);
                       // vfs_remove(name);
       	                 lock_release(proc->file_table[fd]->sys_lock);
       	                 return -1;
         	       }
        vfs_close(vn);
        lock_release(proc->file_table[fd]->sys_lock);
    	return buflen;
	}
	else if(fd==2)
	{
		
		const char *con="con:";
		char *file_name;
		struct vnode *v0;
		file_name=kstrdup(con);
		//proc->file_table[2]=kmalloc(sizeof(struct file_handle));
		vfs_open(file_name, O_WRONLY, 0, &v0);
		proc->file_table[2]->v=v0;
		proc->file_table[2]->offset=0;
		proc->file_table[2]->perm=O_WRONLY;
		proc->file_table[2]->ref_count=1;
		//as = proc_getas();
		vn=proc->file_table[fd]->v;
	
	//kprintf("%d", buflen);
		result1=copyin(buf,dest,buflen);
		if(result1!=0)
		{
			//lock_release(proc->file_table[fd]->sys_lock);
			*err1=result1;
			return -1;
		}
		lock_acquire(proc->file_table[fd]->sys_lock);
	
		uio_kinit(&iov, &ku, dest, buflen, proc->file_table[fd]->offset, UIO_WRITE);
		result = VOP_WRITE(vn, &ku);
		proc->file_table[fd]->offset=ku.uio_offset;
		if (result) {
       	                 kprintf(" Write error: ");
     	                  // vfs_close(vn);
                       // vfs_remove(name);
       	                 lock_release(proc->file_table[fd]->sys_lock);
       	                 return -1;
         	       }
        vfs_close(vn);
        lock_release(proc->file_table[fd]->sys_lock);
    	return buflen;
	}
	else
	{*/


	vn=proc->file_table[fd]->v;
	
	//kprintf("%d", buflen);
	result1=copyin(buf,dest,buflen);
	if(result1!=0)
	{
		kfree(dest);
		*err1=result1;
		//lock_release(proc->file_table[fd]->sys_lock);
		return -1;
	}

	
	lock_acquire(proc->file_table[fd]->sys_lock);
	uio_kinit(&iov, &ku, dest, buflen, proc->file_table[fd]->offset, UIO_WRITE);
	result = VOP_WRITE(vn, &ku);
	proc->file_table[fd]->offset=ku.uio_offset;

	if (result) {
		kprintf(" Write error: ");
                       // vfs_close(vn);
                       // vfs_remove(name);
                lock_release(proc->file_table[fd]->sys_lock);
		kfree(dest);
		return -1;
	}
	kfree(dest);
	lock_release(proc->file_table[fd]->sys_lock);
	return buflen-ku.uio_resid;
}
int
sys_open(const char *filename, int flags,int *err1)
{
	//struct addrspace *as;
	char *file_name;
	struct proc *proc = curproc;
	struct vnode *vn;
	int result;
	int proc_fd=-1;
	size_t got;
	if(filename==NULL){
		*err1 = EFAULT;
		return -1;
	}
	void*buf=kmalloc(sizeof(char)*PATH_MAX);


	
	result=copyinstr((userptr_t)filename,buf,sizeof(char)*PATH_MAX,&got);
	if(result)
	{
		kfree(buf);
		*err1=result;
		return -1;
	}
	//as = proc_getas();
	file_name=kstrdup(buf);
	//kprintf("%d", buflen);
	int i=0;
	result=vfs_open(file_name, flags, 0, &vn);
	for(i=3; proc->file_table[i]!=NULL; i++);
	//kprintf("inside open %d\n",i);
	proc->file_table[i]=kmalloc(sizeof(struct file_handle));
	proc_fd=i;
	proc->file_table[i]->v=vn;
	proc->file_table[i]->offset=0;
	proc->file_table[i]->perm=flags;
	proc->file_table[i]->ref_count=1;
	proc->file_table[i]->sys_lock=lock_create(file_name);
	
	kfree(file_name);
	if (result) {
		*err1 = EINVAL;
                       // vfs_close(vn);
                       // vfs_remove(name);
		kfree(buf);
		return -1;
	}
	kfree(buf);
	return proc_fd;

}
int 
sys_read(int fd, void *buf, size_t buflen,int *err1){

	int result;
//int result1;
	struct iovec iov;
	struct uio ui;
	//struct uio ku;
	struct proc *proc = curproc;
	struct vnode *vn;
	

	void *dest = kmalloc(buflen);

	if(fd<0 || fd>=OPEN_MAX|| proc->file_table[fd]==NULL){
		kfree(dest);
		*err1 = EBADF;
		return -1;
	}

	int in = proc->file_table[fd]->perm&000011;
	if( in == 1){
		kfree(dest);
		*err1 = EBADF;
		return -1;
	}

	if(buf == NULL || buflen<=0){
		kfree(dest);
		*err1 = EFAULT;
		return -1;
	}

	vn = proc->file_table[fd]->v;





/*if(result1){
	kprintf("Error in read syscall: ");
	return result1;
}*/
	lock_acquire(proc->file_table[fd]->sys_lock);
	uio_kinit(&iov, &ui, dest, buflen, proc->file_table[fd]->offset, UIO_READ);
	result=VOP_READ(vn, &ui);
	if(result){
		kfree(dest);
		*err1 = EFAULT;
		lock_release(proc->file_table[fd]->sys_lock);
		return -1;
	}
//kprintf("aaaaa %s",*dest);
	result=copyout(dest, buf, buflen);
	if(result){
		kfree(dest);
		*err1 = EFAULT;
		lock_release(proc->file_table[fd]->sys_lock);
		return -1;
	}
	proc->file_table[fd]->offset=ui.uio_offset;
/*if(result){
	kprintf("Error in syscall read: ");
	return result;
}*/
	kfree(dest);
	lock_release(proc->file_table[fd]->sys_lock);
	return buflen-ui.uio_resid;

}
int
sys_close(int fd,int *err1)
{
	struct proc *proc = curproc;
	if(fd<0 || fd>=OPEN_MAX||proc->file_table[fd]==NULL){
		*err1 = EBADF;
		return -1;
	}
	
	lock_acquire(proc->file_table[fd]->sys_lock);
	//kprintf("inside close %d\n",fd);
	if(proc->file_table[fd]->ref_count==1)
	{
		vfs_close(proc->file_table[fd]->v);
	//	kfree(proc->file_table[fd]->v);
		proc->file_table[fd]->v=NULL;
		proc->file_table[fd]->offset=0;
		proc->file_table[fd]->ref_count=0;
		lock_release(proc->file_table[fd]->sys_lock);
		lock_destroy(proc->file_table[fd]->sys_lock);
		kfree(proc->file_table[fd]);
		proc->file_table[fd]=NULL;

		//kprintf("close completed\n");
		
	}
	else
	{
		proc->file_table[fd]->ref_count--;
		lock_release(proc->file_table[fd]->sys_lock);
		proc->file_table[fd]=NULL;
		//kprintf("close done");
		//kfree(proc->file_table[fd]);
	}
	*err1=0;
	//lock_release(proc->file_table[fd]->sys_lock);
	
	return 0;
}
off_t 
sys_lseek(int fd, off_t pos, int whence, int *err1)
{

	struct proc *proc = curproc;

	if(fd<0 || fd>=OPEN_MAX || proc->file_table[fd]==NULL){
		*err1 = -1;
		return (__i64)EBADF;
	}

	
	if(proc->file_table[fd]==NULL)
	{
		*err1=-1;
		return (__i64)EBADF;
	}
	if(proc->file_table[fd]==NULL)
	{
		*err1=EBADF;
		return -1;
	}
	struct vnode *vn = proc->file_table[fd]->v;	
	off_t result;
	struct stat statbuf;
	*err1 =0;

	//off_t err = (__i64)-1;
	int wh;
	
	lock_acquire(proc->file_table[fd]->sys_lock);
	*err1 = copyin((userptr_t)whence, &wh, sizeof(int));
	if(*err1){
		*err1 = -1;
		lock_release(proc->file_table[fd]->sys_lock);
		return (__i64)EINVAL;
	}

	if(proc->file_table[fd]->v==NULL)
	{
		lock_release(proc->file_table[fd]->sys_lock);
		return (__i64)EBADF;
	}
	if(!VOP_ISSEEKABLE(vn)){
		*err1 = -1;
		kprintf("error in VOP_ISSEEKABLE");
		result = (__i64)ESPIPE;
		lock_release(proc->file_table[fd]->sys_lock);
		return result;

	}

	off_t offset = proc->file_table[fd]->offset;

	if(wh<0 || wh>2){
		*err1 = -1;
		lock_release(proc->file_table[fd]->sys_lock);
		return (__i64)EINVAL;
	}

	if(wh==SEEK_SET){
		if(pos<0){
			*err1 = -1;
			kprintf("error in seek set");
			result = (__i64)EINVAL;
			lock_release(proc->file_table[fd]->sys_lock);
			return result;
		}
		
		offset = pos;
		proc->file_table[fd]->offset=offset;
	}
	if(wh==SEEK_CUR){
		offset = proc->file_table[fd]->offset;
		if(offset+pos<0){
			kprintf("error in seek cur");
			*err1 = -1;
			result = (__i64)EINVAL;
			lock_release(proc->file_table[fd]->sys_lock);
			return result;
		}
		
		offset = offset + pos;
		proc->file_table[fd]->offset=offset;

	}

	if(wh==SEEK_END){
		result = (__i64)VOP_STAT(vn, &statbuf);
		//kprintf("entered in seek end");
		if(result){
			kprintf("error in SEEK_END:");
			*err1 = -1;
			lock_release(proc->file_table[fd]->sys_lock);
			return result;
		}
		
		offset = (statbuf.st_size) + pos;
		proc->file_table[fd]->offset=offset;

	}
	lock_release(proc->file_table[fd]->sys_lock);
	return offset;

}

pid_t sys_fork(struct trapframe *ptf, int *err1){
//	kprintf("--->entered fork\n");
	fork_prevent++;
/*	if(fork_prevent==128)
	{
		for(int i=3;i<PID_MAX;i++)
		{
			proc_destroy(p_table[i].p);
		}
		*err1=ENPROC;
		return -1;
	}*/
/*	if(fork_prevent>5)
	{
//		kprintf("print");
		*err1=ENPROC;
		return -1;
	}*/
	int i=0;
	for(i=3;i<__PID_MAX;i++){
		if(p_table[i].pid==-1){
			break;
		}
	}
	if(i==__PID_MAX)
	{
	//	kprintf(".");
		*err1 = ENOMEM;
		return -1;
	}
	struct proc *cproc = child_create("child_proc");
	if(cproc==NULL)
	{
//		kprintf("fork\n");
		*err1=ENOMEM;
		return -1;
	}
//cproc->ppid=curproc->pid;
	//kprintf("inside maluu    %d",cproc->pid);
	

	/*if(cproc == NULL){
		//kprintf("inside maluu");
		*err1 = ENOMEM;
		return -1;
	}*/

	/*if(cproc->pid == -2){
		//kprintf("too many processes are here");
		*err1 = ENPROC;
		return -1;
	}*/

	struct trapframe *ctf = kmalloc(sizeof(struct trapframe));
	if(ctf==NULL)
	{
//		proc_destroy(cproc);
		kprintf("fork ctf\n");
		*err1=ENOMEM;
		return -1;
	}
//struct addrspace *caddr;
//pid_t result_pid=-1;
	int result;
	struct proc *proc = curproc;


//assign unique pid to child process


//copy the address space of parent process to child process address space
//paddr = proc_getas();
	//kprintf("inside maluu");
	result = as_copy(proc->p_addrspace,&cproc->p_addrspace);
	if(result != 0){
		kfree(ctf);
//		proc_destroy(cproc);
//		kprintf("entered\n");
		*err1=ENOMEM;
		return -1;
	}

//copy the parent ptrapframe into child trapframe
//ctf = *ptf;
	memcpy(ctf, ptf, sizeof(*ptf));
	/*if(result!=0)
	{
		kfree(ctf);
		proc_destroy(cproc);
		kprintf("memcpy\n");
		*err1=ENOMEM;
		return -1;
	}*/
//	kprintf("SIZE: %d--->\n",sizeof(curproc->file_table[i]));
//code for sharing the file handle
	for(int i=0;i<OPEN_MAX;i++)
	{
		if(curproc->file_table[i]!=NULL)
		{
		//	kprintf("%d--->",curproc->file_table[i]->ref_count);
			//kprintf("inside m: %d\n",i);
			curproc->file_table[i]->ref_count++;
			cproc->file_table[i] = proc->file_table[i];
		}
		
	}

//call this at the end of fork
	result  = thread_fork("child_proc",cproc,(void*)enter_forked_process,(void*)ctf,(long unsigned)cproc->p_addrspace);
	if(result!=0){
		kfree(ctf);
//		proc_destroy(cproc);
		*err1=result;
		return -1;
	}


	return cproc->pid;
}

pid_t sys_getpid(){
	struct proc *proc  = curproc;
	return proc->pid;
}
pid_t sys_waitpid(pid_t pid, int *status, int options, int *err1){
	int result;
	struct proc *cproc = p_table[pid].p;
//	kprintf("enter waitpid\n");
	//kprintf("%d ananannannd",p_table[pid].pid);
	if(cproc==NULL){
		//kprintf("invalid child");
		*err1 = ESRCH;
		return -1;
	}

	

	if(pid<0 || pid>__PID_MAX){
		//kprintf("bad pid");
		*err1 = ESRCH;
		return -1;
	}

	if(curproc->pid != cproc->ppid){
		//kprintf("invalid parent");
		*err1 = ECHILD;
		return -1;
	}

	if(options!=0){
		//kprintf("options NULL");
		*err1 = EINVAL;
		return -1;
	}
	
	if(!p_table[pid].p->exit){
		P(p_table[pid].p->sem);	
	}

	if(status==NULL)
	{
		
		proc_destroy(cproc);
		p_table[pid].p=NULL;
		p_table[pid].pid=-1;
//		as_destroy(cproc->p_addrspace);
		//kprintf("status NULL");
		*err1 = 0;
		return pid;
	}	
	
	
	
	result=copyout((const void*)&(p_table[pid].p->exitcode),(userptr_t)status,sizeof(int));
	

	if(result)
	{
		//kprintf("options NULLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL");
		*err1= EFAULT;
		return -1;
	}
	//status = (int*)proc->exitcode;
	//if(WIFEXITED(*status)){
	//	return proc->pid;
	//}
	
/*	for (int i=0;i<__OPEN_MAX;i++)
	{
		if(cproc->file_table[i]!=NULL)
		{
			int err;
			sys_close(i,&err);
		}
	}*/
	proc_destroy(cproc);
	p_table[pid].p=NULL;
	p_table[pid].pid=-1;
	//as_destroy(cproc->p_addrspace);
	//p_table[pid].p=NULL;
	//p_table[pid].pid=-1;//free the space from the space table
	*err1=0;

	return pid;
}
void sys__exit(int exitcode){
//	kprintf("--->exit\n");
	/*struct proc *proc = curproc;
	
	kprintf("entered maluu");
	thread_exit();*/
	//implement as_deactivate in addrspace.c
	struct proc *proc = curproc;
	//exit(exitcode);
	//kprintf("entered exit\n");
	//kprintf("exit kardiya");
	//kprintf("before >>>>>>>%d",exitcode);
	/* close all the files */
	for (int i=0;i<__OPEN_MAX;i++)
		{
			if(curproc->file_table[i]!=NULL)
			{
				int err;
				sys_close(i,&err);
			}
		}

	proc->exitcode = _MKWAIT_EXIT(exitcode);
	//kprintf("after>>>>>>>>>%d",proc->exitcode);
	proc->exit=true;
	V(proc->sem);
	//kprintf("inside     maluu");
	//kprintf("semphore incremented\n");
	thread_exit();
	//kprintf("thread exited\n");
	//implement as_deactivate in addrspace.c
}
int
sys_execv(const char *program, char **args,int *err1)
{
//	kprintf("enter printf\n");
	if(program==NULL)
	{
		*err1=EFAULT;
		return -1;
	}
	if(args==NULL)
	{
		*err1=EFAULT;
		return -1;
	}
	int result;
	
	//kprintf("exec\n");
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	//int result; 
	
	size_t got;
	/* Open the file. */
	//kprintf("%s maluu \n",args[1]);
	char **kargs=kmalloc(sizeof(char)*65500);
	result=copyin((userptr_t)args,kargs,sizeof(args));
	if(result)
	{
		*err1=EFAULT;
		return -1;
	}
	char *check=kmalloc(sizeof(char*));
	result=copyin((userptr_t)program,check,sizeof(program));
	if(result)
	{
		*err1=EFAULT;
		return -1;
	}
	kfree(check);
	//kprintf("%s",file_name);
	result = vfs_open((char*)program, O_RDONLY, 0, &v);
	if (result) {
		*err1=EINVAL;
		return -1;
	}
	
	
	/*for (int i=0;args[i]!=NULL;i++)
	{
		check=kmalloc(sizeof(char*));
		result=copyin((userptr_t)args[i],check,sizeof(char*));
		
		if(result)
		{
			//kprintf("%s \n",args[i-10]);
			//kprintf("%d \n",l);
			*err1=EFAULT;
			return -1;
		}
		kfree(check);
	}*/
	char check1;
	int i=0;
	//kprintf("%s",args[1]);
	for (i=0;args[i]!=NULL;i++)
	{
		result=copyin((userptr_t)args[i],&check1,sizeof(args[i]));
		if(result)
		{
			*err1=EFAULT;
			return -1;
		}
		int l;
		l=strlen(args[i]);
		//kprintf("%d \n",l);
		kargs[i]=kmalloc(sizeof(char)*(l+1));
		result=copyinstr((userptr_t)args[i],kargs[i],sizeof(char)*(l+1),&got);
		
		if(result)
		{
			kfree(kargs);
			//kprintf("%s \n",args[i-10]);
			//kprintf("%d \n",l);
			*err1=EFAULT;
			return -1;
		}
		//kprintf("%s \n",kargs[i]);
	}
	//kargs[i]=kmalloc(sizeof(char*));
	//kargs[i]=NULL;
	//kprintf("%s %d\n",kargs[i],i);
	//KASSERT(proc_getas() == NULL);
	as_destroy(curproc->p_addrspace);
	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}
	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();
	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}
	/* Done with the file now. */
	vfs_close(v);
	
	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
	//kprintf("%d maluu", stackptr);
	
	int j=0;
	//kprintf("%s anannannand \n",kargs[1]);
	//kprintf("%s \n",kargs[0]);
	//kprintf("%s \n",kargs[1]);
	//char**kaddr=kmalloc(sizeof(char)*);
	for(j=0;kargs[j]!=NULL;j++)
	{
		int size;
		//kprintf("%s    \n and j=%d \n",kargs[0],j);
		size=strlen(kargs[j]);
		size=size+1;
		if (size%4 != 0) {
			size=size+4-(size%4);
		}
		stackptr = stackptr-size;
		//if(j>2000)
		//{
			//kprintf("%s %d\n",kargs[i],i);
		//}
		
		result = copyout(kargs[j],(userptr_t)stackptr,size);
		kfree(kargs[j]);
		if (result) {
			kfree(kargs);
			*err1=result;
			return -1;
		}
		//kaddr[j]=kmalloc(sizeof(char*));
		kargs[j]=(char *)stackptr;
	}
	stackptr=stackptr-(4*sizeof(char));
	copyout(kargs[j],(userptr_t)stackptr,4*sizeof(char));
	int q=i-1;
	for(q=q;q>=0;q--)
	{
		stackptr=stackptr-sizeof((char*)stackptr);
		result=copyout(&kargs[q],(userptr_t)stackptr,sizeof((char*)stackptr));
		if(result)
		{
			kfree(kargs);
			*err1=result;
			return -1;
		}
	}
	kfree(kargs);
	//proc_setas(as);
	*err1=0;
	
	//kprintf("%p",(userptr_t)stackptr);
	enter_new_process(i /*argc*/, (userptr_t)stackptr /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);
	


}
int
sys_dup2(int oldfd, int newfd,int *err1)
{
	if((oldfd<0 || oldfd >=OPEN_MAX) || (newfd<0 || newfd >=OPEN_MAX))
	{

		//kprintf("dup2\n %d\n",newfd);
		*err1=EBADF;
		return -1;
	}
	if(curproc->file_table[oldfd]==NULL)
	{
		*err1=EBADF;
		return -1;
	}
	if(oldfd==newfd)
	{
		return newfd;
	}
	lock_acquire(curproc->file_table[oldfd]->sys_lock);
	if(curproc->file_table[newfd]==NULL)
	{
		//kprintf("enter dup21 >>>>>>>>>>>>\n");
		curproc->file_table[newfd]=curproc->file_table[oldfd];
	}
	else
	{
			struct proc *proc = curproc;
			if(proc->file_table[newfd]->ref_count==1)
			{
				vfs_close(proc->file_table[newfd]->v);
				proc->file_table[newfd]->v=NULL;
				proc->file_table[newfd]->offset=0;
				proc->file_table[newfd]->ref_count=0;
				lock_destroy(proc->file_table[newfd]->sys_lock);
				kfree(proc->file_table[newfd]);
				proc->file_table[newfd]=NULL;
				
		//lock_destroy(proc->file_table[fd]->sys_lock);
			}
			else
			{
				proc->file_table[newfd]->ref_count--;
				proc->file_table[newfd]=NULL;
				
			}
		
		curproc->file_table[oldfd]->ref_count++;
		//curproc->file_table[newfd]=kmalloc(sizeof(struct file_handle));
		curproc->file_table[newfd]=curproc->file_table[oldfd];

	}
	//kprintf("enter dup22\n");
	lock_release(curproc->file_table[oldfd]->sys_lock);
	return newfd;
}

int
sys_sbrk(intptr_t amount,int *err1)
{
//	spinlock_acquire(&curproc->p_lock);
	vaddr_t stackbase=USERSTACK-(1024 * PAGE_SIZE);
	stackbase&=PAGE_FRAME;
	int check;
//	kprintf("%d\n",curproc->p_addrspace->heap);
	if(amount < PAGE_SIZE && amount > 0)
	{
		*err1=EINVAL;
		return -1;
	}
	if(amount>=0)
	{	
		check=((amount/PAGE_SIZE)%1==0)?0:1;
//		kprintf("%d\n",check);
		if(check==1)
		{
			*err1=EINVAL;
			return -1;
		}
	}
	else if(amount<0)
	{
		check=(((-1*amount)/PAGE_SIZE)%1==0)?0:1;
		if(check==1)
		{
			*err1=EINVAL;
			return -1;
		}
	}
	if((curproc->p_addrspace->heap_size+amount)<0)
	{
		*err1=EINVAL;
		return -1;
	}
	if(curproc->p_addrspace->heap + curproc->p_addrspace->heap_size + amount >=stackbase)
	{
		*err1=ENOMEM;
		return -1;
	}
	struct page_table_entry *pte1=curproc->p_addrspace->head_page;
		
	while(pte1!=NULL)
	{
		if(pte1->va>=curproc->p_addrspace->heap && pte1->va<stackbase)
		{
//			kprintf("Added: %d------>",pte1->va);
		}
		pte1=pte1->next;
	}
//		kprintf("\n");
	//if((curproc->p_addrspace->heap + amount)<curproc->p_addrspace->heap)
	//{
	//	*err1=EINVAL;
	//	return -1;
	//}
//	kprintf("%d\n",curproc->p_addrspace->heap);

/* print all the pages in the heap */
	






	
	/* increment/decrement the heap size */
	if(amount<0)
	{	
/*		int npages=(-1*amount)/PAGE_SIZE;
		
		for(int i=1;i<=npages;i++)
		{
			vaddr_t check_vaddr=(curproc->p_addrspace->heap
		+curproc->p_addrspace->heap_size)-(i*PAGE_SIZE);
			struct page_table_entry *pte=curproc->p_addrspace->head_page, *pte_prev=NULL;*/

	
	/*		while(pte!=NULL && pte->va!=check_vaddr)
			{
				pte_prev=pte;
				pte=pte->next;
			}
			if(pte!=NULL)
			{
				vaddr_t vaddr=PADDR_TO_KVADDR(pte->pa);
				free_kpages(vaddr);
				pte_prev->next=pte->next;
				kfree(pte);
			}*/
		vaddr_t old_heap=curproc->p_addrspace->heap+curproc->p_addrspace->heap_size;

		vaddr_t new_heap=old_heap+amount;
		int flag = 0;

		struct page_table_entry *pte=curproc->p_addrspace->head_page, *pte_prev=NULL;
		pte_prev=pte;
		




		
		while(pte!=NULL)
		{
			if(pte->va>new_heap && pte->va<stackbase)
			{
//				kprintf("Deleted:%d---->",pte->va);
				vaddr_t vaddr=PADDR_TO_KVADDR(pte->pa);
				free_kpages(vaddr);
				pte_prev->next=pte->next;
				flag=1;	
				kfree(pte);
			}
			if(flag==1)
			{
				pte=pte_prev->next;
			}
			else
			{
				pte_prev=pte;
				pte=pte->next;
			}
			flag=0;
		}	

	}
					
	int old_break=curproc->p_addrspace->heap+curproc->p_addrspace->heap_size;
	curproc->p_addrspace->heap_size=curproc->p_addrspace->heap_size+amount;
//	spinlock_release(&curproc->p_lock);
	return old_break;
	
}	
