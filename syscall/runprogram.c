/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than runprogram() does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname)
{
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
	struct proc *newproc=curproc;
	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}


	/* open console files*/
	struct vnode *v0;
	struct vnode *v1;
	struct vnode *v2;
	const char *con="con:";
	char *file_name1;
	char *file_name2;
	char *file_name3;
	file_name1=kstrdup(con);
	file_name2=kstrdup(con);
	file_name3=kstrdup(con);
	//open stdout
	
	vfs_open(file_name1, O_WRONLY, 0664, &v1);
	newproc->file_table[1]=kmalloc(sizeof(struct file_handle));
	newproc->file_table[1]->v=v1;
	newproc->file_table[1]->offset=0;
	newproc->file_table[1]->perm=O_WRONLY;
	newproc->file_table[1]->ref_count=1;
	newproc->file_table[1]->sys_lock = lock_create(progname);
	//open stdin
	vfs_open(file_name2, O_RDONLY, 0664, &v0);
	newproc->file_table[0]=kmalloc(sizeof(struct file_handle));
	newproc->file_table[0]->v=v0;
	newproc->file_table[0]->offset=0;
	newproc->file_table[0]->perm=O_RDONLY;
	newproc->file_table[0]->ref_count=1;
	newproc->file_table[0]->sys_lock = lock_create(progname);
	//open stderr
	vfs_open(file_name3, O_WRONLY, 0664, &v2);
	newproc->file_table[2]=kmalloc(sizeof(struct file_handle));
	newproc->file_table[2]->v=v2;
	newproc->file_table[2]->offset=0;
	newproc->file_table[2]->perm=O_WRONLY;
	newproc->file_table[2]->ref_count=1;
	newproc->file_table[2]->sys_lock = lock_create(progname);


	kfree(file_name1);
	kfree(file_name2);
	kfree(file_name3);


	/* We should be a new process. */
	KASSERT(proc_getas() == NULL);

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

	/* Warp to user mode. */
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}

