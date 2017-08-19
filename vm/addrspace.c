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
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <mips/tlb.h>
#include <spl.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		kprintf("as_create\n");
		return NULL;
	}

	/*
	 * Initialize as needed.
	 */
	as->head_region=NULL;
	as->heap=0;
	as->heap_size=0;
	as->head_page=NULL;
	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */
	/* copy addrspace struct from old to new */
	struct region_info *old_info=old->head_region;
	
	while(old_info!=NULL)
	{
		
		if(newas->head_region==NULL)
		{
			newas->head_region=kmalloc(sizeof(struct region_info));
			if(newas->head_region==NULL)
			{
				return ENOMEM;
			}
			newas->head_region->start=old_info->start;
			newas->head_region->size=old_info->size;
			newas->head_region->next=NULL;
			//*newas->head_region=*old_info;
			
		}
		else
		{
			struct region_info *newas_info=newas->head_region;
			while(newas_info->next!=NULL)
			{
				newas_info=newas_info->next;
			}
			newas_info->next=kmalloc(sizeof(struct region_info));
			if(newas_info->next==NULL)
			{
				return ENOMEM;
			}
			newas_info->next->start=old_info->start;
			newas_info->next->size=old_info->size;
			newas_info->next->next=NULL;
			//*newas_info->next=*old_info;
		}
		old_info=old_info->next;
	}
	
	newas->heap=old->heap;
	newas->heap_size=old->heap_size;	
	
	struct page_table_entry *old_page=old->head_page;
	while(old_page!=NULL)
	{
		if(newas->head_page==NULL)
		{
			newas->head_page=kmalloc(sizeof(struct page_table_entry));
			if(newas->head_page==NULL)
			{
				return ENOMEM;
			}
			newas->head_page->va=old_page->va;
			paddr_t paddr;
			paddr=getppages(1,false);
			if(paddr==0)
			{
				return ENOMEM;
			}
			newas->head_page->pa=paddr;
//			newas->head_page->state=old_page->state;
			newas->head_page->next=NULL;
			
/*			if(old_page->state==false)
			{
				
				void *buf=kmalloc(PAGE_SIZE);
				int result=blockread(old_page->index,&buf);	//arguments??
				memcpy((void*)PADDR_TO_KVADDR(newas->head_page->pa),buf,PAGE_SIZE);	//buf??
				old_page->state=true;
			memmove((void*)PADDR_TO_KVADDR(newas->head_page->pa),
				(const void *)PADDR_TO_KVADDR(old_page->pa),
				PAGE_SIZE);
			}*/
			newas->head_page->state=old_page->state;
				
		}
		else
		{
			struct page_table_entry *newas_page=newas->head_page;
			while(newas_page->next!=NULL)
			{
				newas_page=newas_page->next;
			}
			newas_page->next=kmalloc(sizeof(struct page_table_entry));
			if(newas_page->next==NULL)
			{
				return ENOMEM;	
			}
			newas_page->next->va=old_page->va;
			paddr_t paddr;
			paddr=getppages(1,false);
			if(paddr==0)
			{
				return ENOMEM;
			}
			newas_page->next->pa=paddr;
			newas_page->next->state=old_page->state;
			newas_page->next->next=NULL;

			memmove((void*)PADDR_TO_KVADDR(newas_page->next->pa),
				(const void *)PADDR_TO_KVADDR(old_page->pa),
				PAGE_SIZE);
		}
		old_page=old_page->next;
	}	
		

	
	
//	(void)old;

	*ret = newas;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */
	struct region_info *current = as->head_region;                         
        struct region_info *temp = current;                                    
        
        while(current!=NULL){
                temp = current->next;                                          
                kfree(current);                                                
                current = temp;                                                
        }
//	kfree(as->head_region);                                                                      
	as->head_region=NULL;        
        struct page_table_entry *pte = as->head_page;                          
        struct page_table_entry *tmp= pte;                                     
        
        while(pte!=NULL){
		vaddr_t vaddr=PADDR_TO_KVADDR(pte->pa);
		free_kpages(vaddr);
		//free_kpages(vaddr);
                tmp = pte->next;                                               
                kfree(pte);                                                    
                pte=tmp;                                                       
        }
//	kfree(as->head_page);                                                                      
 	as->head_page=NULL;       
        kfree(as); 
}

void
as_activate(void)
{
	int i;
	int spl;
        struct addrspace *as;

        as = proc_getas();
        if (as == NULL) {
                return;
        }

        /* Disable interrupts on this CPU while frobbing the TLB. */
        spl = splhigh();

        for (i=0; i<NUM_TLB; i++) {
                tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        }

        splx(spl);

}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	(void)readable;
	(void)writeable;
	(void)executable;
	//size_t npages;

//      dumbvm_can_sleep();
        
        /* Align the region. First, the base... */
       // memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
        vaddr &= PAGE_FRAME;
        
        /* ...and now the length. */
       // memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;
        
        //npages = memsize / PAGE_SIZE;		// why no ceil?
        
        /* We don't use these - all pages are read-write */
	if(as->head_region==NULL)
	{
		as->head_region=kmalloc(sizeof(struct region_info));
        	if(as->head_region==NULL)
        	{
                	return ENOMEM;
        	}
        	as->head_region->start=vaddr;
//        	as->head_region->size=npages;
		as->head_region->size=memsize;
        //	as->head_region->readable=readable;
        //	as->head_region->writeable=writeable;
        //	as->head_region->executable=executable;
        	as->head_region->next=NULL;
	}	
	else
	{	
		struct region_info *current=as->head_region;
		while(current->next!=NULL)
		{
			current=current->next;
		}
		current->next=kmalloc(sizeof(struct region_info));
		if(current->next==NULL)
		{
			return ENOMEM;
		}
        	current->next->start=vaddr;
        //	current->next->size=npages;
		current->next->size=memsize;				//Ask what to save. memsize of npages 
        //	current->next->readable=readable;
        //	current->next->writeable=writeable;
        //	current->next->executable=executable;
        	current->next->next=NULL;
	}
	
	if(vaddr+memsize>as->heap)
	{
		as->heap=vaddr+memsize+1;
	}
	
       /* (void)readable;
        (void)writeable;
        (void)executable;*/
	return 0;
        
       /* if (as->as_vbase1 == 0) {
                as->as_vbase1 = vaddr;
                as->as_npages1 = npages;
                return 0;
        }
        
        if (as->as_vbase2 == 0) {
                as->as_vbase2 = vaddr;
                as->as_npages2 = npages;
                return 0;
        }*/
        
        
}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

//	(void)as;
	if(as->heap % PAGE_SIZE)
	{
		int amount = PAGE_SIZE - (as->heap % PAGE_SIZE);
		as->heap=as->heap+amount;
	}
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}
