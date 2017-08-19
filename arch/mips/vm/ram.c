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

#include <types.h>
#include <lib.h>
#include <vm.h>
#include <mainbus.h>
#include <spinlock.h>
#include <current.h>
#include <addrspace.h>
#include <mips/tlb.h>
#include <current.h>
#include <proc.h>
vaddr_t firstfree;   /* first free virtual address; set by start.S */
static struct core * coremap;
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
static paddr_t firstpaddr;  /* address of first free physical page */
static paddr_t lastpaddr;
static int last_mem;   		/* one past end of last free physical page */
/*
 * Called very early in system boot to figure out how much physical
 * RAM is available.
 */
void
ram_bootstrap(void)
{
	size_t ramsize;

	/* Get size of RAM. */
	ramsize = mainbus_ramsize();

	/*
	 * This is the same as the last physical address, as long as
	 * we have less than 512 megabytes of memory. If we had more,
	 * we wouldn't be able to access it all through kseg0 and
	 * everything would get a lot more complicated. This is not a
	 * case we are going to worry about.
	 */

	if (ramsize > 512*1024*1024) {
		ramsize = 512*1024*1024;
	}

	lastpaddr = ramsize;

	/*
	 * Get first free virtual address from where start.S saved it.
	 * Convert to physical address.
	 */
	firstpaddr = firstfree - MIPS_KSEG0;

	kprintf("%uk physical memory available\n",
		(lastpaddr-firstpaddr)/1024);

	/* coremap logic */
//	kprintf("Ramsize=%d",ramsize);
	/* get the number of pages needed to store in coremap */
//	int npages=((ramsize/PAGE_SIZE)%1==0)?(ramsize/PAGE_SIZE):(ramsize/PAGE_SIZE)+1;
	int npages=ramsize/PAGE_SIZE;
//	kprintf("npages=%d\n",npages);
	/* calculate how much mem to initialize array */

	int mem_needed=npages*sizeof(struct core);
	
//	kprintf("mem_needed=%d\n",mem_needed);
	/* convert it into page */
	int pages_needed=1;
	if(mem_needed>=PAGE_SIZE)
	{
		pages_needed=((mem_needed/PAGE_SIZE)%1==0)?(mem_needed/PAGE_SIZE):(mem_needed/PAGE_SIZE)+1;
	}
//	int pages_needed=mem_needed/PAGE_SIZE;
//	kprintf("pages_needed=%d\n",pages_needed);
	/* initialize struct */
	coremap=(struct core*)firstfree; 
	int i=0;
	int j=0;
	/* get the number of pages needed before the firstpaddr */
//	kprintf("firstpaddr=%d",firstpaddr);
//	int bef_fpaddr=firstpaddr/PAGE_SIZE;
	int bef_fpaddr=((firstpaddr/PAGE_SIZE)%1==0)?(firstpaddr/PAGE_SIZE):(firstpaddr/PAGE_SIZE)+1;
//	kprintf("before fpaddr: %d\n",bef_fpaddr);
	/* mark them in the coremap */
	for(i=0;i<bef_fpaddr;i++)
	{
		coremap[i].size=bef_fpaddr;
		coremap[i].available=false;
		coremap[i].kernel=true;
		coremap[i].core_pte=NULL;
		KASSERT(coremap[i].size!=0);
	}
	j=i;
	/* mark the pages occupied by coremap */
	for(i=j;i<(j+pages_needed);i++)
	{
		coremap[i].size=pages_needed;
		coremap[i].available=false;
		coremap[i].kernel=true;
		coremap[i].core_pte=NULL;
		KASSERT(coremap[i].size!=0);
	}
	j=i;
//	kprintf("entered");
	for(i=j;i<j+(npages-(bef_fpaddr+pages_needed));i++)
	{
		coremap[i].size=0;
		coremap[i].available=true;
		coremap[i].kernel=false;
		coremap[i].core_pte=NULL;
	}
	last_mem=i;
	for(i=0;i<last_mem;i++)
	{
		if(coremap[i].available==false)
		{
			KASSERT(coremap[i].size!=0);
		}
	}
//	kprintf("total pages: %d",i);
//	kprintf("LAST ENTRY: %d",coremap[0].available);
	/* increment the firstpaddr by mem_needed */
}
paddr_t
getppages(unsigned long npages,bool kernel)
{
        paddr_t addr;
//	kprintf("%d",(int)npages);
        spinlock_acquire(&stealmem_lock);
	// kprintf("last mem: %d\n",last_mem);
	int index=-1;
	int count=0;

	for(int i=0;i<last_mem;i++)
	{
		if(coremap[i].available==true)
		{
			count++;
			if(count==(int)npages)
			{
				index=i-npages+1;
				break;
			}
		}
		else
		{
			count=0;
		}
	}
	if(index==-1)
	{
		//as_activate();
		//kprintf("getppages\n");
		//swap out
		while(1)
		{
			int index_rem=random() % (last_mem-1);
			if(coremap[index_rem].kernel==false)
			{
				//update the page table entry and write the data on disk
				vaddr_t vaddr=PADDR_TO_KVADDR(index_rem * PAGE_SIZE);
				struct page_table_entry *current=coremap[index_rem].core_pte;
				int result=blockwrite((void*)vaddr);
			
				if(result==-1)
				{
					kprintf("error");
				}
				while(current!=NULL)
				{
					unsigned int check=index_rem * PAGE_SIZE;
					if(current->pa==check)
					{
						current->pa=-1;
						current->state=false;
						current->index=result;
						break;
					}
					current=current->next;
				}
			
			}
			break;
		}
			
	//	spinlock_release(&stealmem_lock);
	//	return 0;
	}
	for(int i=index;i<(index+(int)npages);i++){
		coremap[i].available=false;
		coremap[i].size=(int)npages;
		coremap[i].kernel=kernel;
		if(kernel==true)
		{
			coremap[i].core_pte=NULL;
		}
		else
		{
			coremap[i].core_pte=curproc->p_addrspace->head_page;
		}
		KASSERT(coremap[i].size!=0);
	}
/*	for(int i=0;i<last_mem;i++)
	{
		if(coremap[i].available==false)
		{
			KASSERT(coremap[i].size!=0);
		}
	}
*/	addr=index * PAGE_SIZE;	
	


	//kprintf("%d\n",random());

        //addr = ram_stealmem(npages);
	bzero((void*)PADDR_TO_KVADDR(addr),npages * PAGE_SIZE);
		
        spinlock_release(&stealmem_lock);
        return addr;
}
void
free_kpages(vaddr_t vaddr)
{
/*	for(int i=0;i<last_mem;i++)
	{
		if(coremap[i].available==false)
		{
			KASSERT(coremap[i].size!=0);
		}
	}
*/        //kprintf("\nentered\n");
        paddr_t paddr = vaddr - MIPS_KSEG0;
//	kprintf("%u\n",paddr);
        int index=((paddr/PAGE_SIZE)%1==0)?(paddr/PAGE_SIZE):(paddr/PAGE_SIZE)+1;
//	kprintf("%d\n",index);
	/*paddr_t paddr;
	struct page_table_entry *page=curproc->p_addrspace->head_page;
	while(page!=NULL)
	{
		if(page->va==vaddr)
		{
			paddr=page->pa;
			break;
		}
		page=page->next;
	}*/
	//int index=vaddr/PAGE_SIZE;
	KASSERT(index>=0 && index<last_mem);
        int size=coremap[index].size;
//	kprintf("%d and %d\n",coremap[index].available,coremap[index].size);
//	KASSERT(coremap[index].size!=0);
	
        for(int i=index;i<(index+size);i++)
        {
		KASSERT(coremap[i].available==false);   
                //kprintf("%d",i);
                coremap[i].available=true;
                coremap[i].size=0;
        }
	as_activate();
}
unsigned
int
coremap_used_bytes()
{
 	int count=0;
	for(int i=0;i<last_mem;i++)
	{
		if(coremap[i].available==false)
		{
			count++;
		}
	}
	unsigned int bytes=count * PAGE_SIZE;
	return bytes;    
}
	
/*
 * This function is for allocating physical memory prior to VM
 * initialization.
 *
 * The pages it hands back will not be reported to the VM system when
 * the VM system calls ram_getsize(). If it's desired to free up these
 * pages later on after bootup is complete, some mechanism for adding
 * them to the VM system's page management must be implemented.
 * Alternatively, one can do enough VM initialization early so that
 * this function is never needed.
 *
 * Note: while the error return value of 0 is a legal physical address,
 * it's not a legal *allocatable* physical address, because it's the
 * page with the exception handlers on it.
 *
 * This function should not be called once the VM system is initialized,
 * so it is not synchronized.
 */
paddr_t
ram_stealmem(unsigned long npages)
{
	size_t size;
	paddr_t paddr;

	size = npages * PAGE_SIZE;

	if (firstpaddr + size > lastpaddr) {
		return 0;
	}

	paddr = firstpaddr;
	firstpaddr += size;

	return paddr;
}

/*
 * This function is intended to be called by the VM system when it
 * initializes in order to find out what memory it has available to
 * manage. Physical memory begins at physical address 0 and ends with
 * the address returned by this function. We assume that physical
 * memory is contiguous. This is not universally true, but is true on
 * the MIPS platforms we intend to run on.
 *
 * lastpaddr is constant once set by ram_bootstrap(), so this function
 * need not be synchronized.
 *
 * It is recommended, however, that this function be used only to
 * initialize the VM system, after which the VM system should take
 * charge of knowing what memory exists.
 */
paddr_t
ram_getsize(void)
{
	return lastpaddr;
}

/*
 * This function is intended to be called by the VM system when it
 * initializes in order to find out what memory it has available to
 * manage.
 *
 * It can only be called once, and once called ram_stealmem() will
 * no longer work, as that would invalidate the result it returned
 * and lead to multiple things using the same memory.
 *
 * This function should not be called once the VM system is initialized,
 * so it is not synchronized.
 */
paddr_t
ram_getfirstfree(void)
{
	paddr_t ret;

	ret = firstpaddr;
	firstpaddr = lastpaddr = 0;
	return ret;
}
