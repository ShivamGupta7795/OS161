#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <vnode.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <stat.h>
#include <bitmap.h>
#include <uio.h>
#define STACKPAGES	1024

struct swapdisk *sd;
void
vm_bootstrap(void)
{
	/* Do nothing. */
//	kprintf("SIZE file_handle: %d---->\n",sizeof(struct file_handle));
//	kprintf("SIZE addrspace: %d---->\n",sizeof(struct addrspace));
//	kprintf("SIZE region info: %d---->\n",sizeof(struct region_info));
//	kprintf("SIZE proc: %d---->\n",sizeof(struct proc));

	struct vnode *v;
	int result;
	const char *file_name="lhd0raw:";
	char *file_name1=kstrdup(file_name);
	struct stat statbuf;
	int size;
//	struct bitmap *bm;
//	struct device *dev;
/*	result=vfs_setbootfs(file_name1);
	if(result)
	{
		kprintf("setbootfs\n%d",result);
	}*/
	result=vfs_open(file_name1,O_RDWR,0,&v);
	if(result)
	{
		kprintf("swapon\n%d",result);
	}
	struct swapdisk *sd=kmalloc(sizeof(struct swapdisk));

	sd->v=v;
	VOP_STAT(v,&statbuf);
	if(result)
	{
		kprintf("VOP STAT ERROR");
//		return result;
	}
	size=statbuf.st_size;
	int bit_size=size/PAGE_SIZE;
	if(size<PAGE_SIZE)
	{
		bit_size=1;
	}
	
	
//	kprintf("%d\n",bit_size);
	
	//Create a bit map
	
	sd->bm=bitmap_create(bit_size);
}

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("dumbvm tried to do tlb shootdown?!\n");
}

vaddr_t
alloc_kpages(unsigned npages)
{
	paddr_t pa;
	pa = getppages(npages,true);
	if (pa==0) {
	//	kprintf("alloc_kpages\n");
		return 0;
	}
	return PADDR_TO_KVADDR(pa);
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase, vtop, stackbase, stacktop;
	paddr_t paddr;
	uint32_t ehi, elo;
	struct addrspace *as;
	int spl;

	(void)faulttype;
	faultaddress&=PAGE_FRAME;	

	if (curproc == NULL) {
		/*
		 * No process. This is probably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	as = proc_getas();
	if (as == NULL) {
		/*
		 * No address space set up. This is probably also a
		 * kernel fault early in boot.
		 */
		return EFAULT;
	}
	/* check if the address is valid */
	struct region_info *current=as->head_region;
	int invalid=1;
	stackbase=USERSTACK - STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;
	while(current!=NULL)
	{
		vbase=current->start;
		//vtop=vbase+current->size * PAGE_SIZE;
		vtop=vbase+current->size;
		if(faultaddress >= vbase && faultaddress < vtop)		//equal to or not
		{
			invalid=0;
			break;
		}
		invalid=1;
		current=current->next;
	}
	if(faultaddress >= stackbase && faultaddress < stacktop)
	{
		invalid=0;
	}
	

	/* checking heap code here */
	vaddr_t heap_base,heap_top;
	heap_base=as->heap;
	heap_top=as->heap + as->heap_size;
	if(faultaddress >= heap_base && faultaddress < heap_top)
	{
		invalid=0;
	}


	if(invalid==1)
	{
		return EFAULT;
	}

	/* check if in page table */
	int in_page=0;
	
	struct page_table_entry *current1=as->head_page;
	while(current1!=NULL)
	{
		if(faultaddress == current1->va)
		{
			//check in disk or mem

			if(current1->state==false)
			{
				paddr=getppages(1,false);
				vaddr_t vaddr=PADDR_TO_KVADDR(paddr);
				int result=blockread(current1->index,(void*)vaddr);	//arguments??
				if(result)
				{
					return -1;	//Handle this properly
				}
				current1->state=true;
				in_page=1;
				break;
			}
			else
			{
				paddr=current1->pa;
				in_page=1;
				break;
			}
		}
		in_page=0;
		current1=current1->next;
	}
	spl=splhigh();
	if(in_page==1)
	{
		
		ehi=faultaddress;
		elo=paddr | TLBLO_DIRTY | TLBLO_VALID;
		tlb_random(ehi,elo);
		splx(spl);
		return 0;
	}
	else
	{
		paddr_t paddr;
		paddr=getppages(1,false);
		
		if(paddr==0)
		{
			kprintf("vm_fault\n");
			return ENOMEM;
		}
		if(as->head_page==NULL)
		{
			as->head_page=kmalloc(sizeof(struct page_table_entry));
			if(as->head_page==NULL)
			{
				kprintf("vm_fault\n");
				return ENOMEM;
			}
			as->head_page->va=faultaddress;
			as->head_page->pa=paddr;
			as->head_page->state=true;
			as->head_page->next=NULL;
		}
		else if(as->head_page!=NULL)
		{
			struct page_table_entry *current2=as->head_page;
			while(current2->next!=NULL)
			{
				current2=current2->next;
			}
			current2->next=kmalloc(sizeof(struct page_table_entry));
			if(current2->next==NULL)
			{
				kprintf("vm_fault\n");
				return ENOMEM;
			}
			current2->next->va=faultaddress;
			current2->next->pa=paddr;
			current2->next->state=true;
			current2->next->next=NULL;
		}
			
		ehi=faultaddress;
		elo=paddr | TLBLO_DIRTY | TLBLO_VALID;
		tlb_random(ehi,elo);
		splx(spl);
		return 0;
	}		
			
}
int blockread(int index, void *buf)
{
	int offset=index * PAGE_SIZE;
	struct vnode *v=sd->v;	
	struct iovec iov;
	struct uio ui;
	int result;

	uio_kinit(&iov, &ui, buf, PAGE_SIZE, offset, UIO_READ);
	result=VOP_READ(v, &ui);
	if(result){			
		kfree(buf);			//take this part out
		return -1;
	}
//kprintf("aaaaa %s",*dest);
	bitmap_unmark(sd->bm,index);
	return 0;
}
int blockwrite(void *buf)
{
	unsigned int index;
	int result;
	result=bitmap_alloc(bm,&index);
	if(result)
	{
		return -1;
	}
	int offset=index*PAGE_SIZE;
	struct vnode *v=sd->v;	
	struct iovec iov;
	struct uio ui;

	// uio_kinit(&iov, &ui, buf, PAGE_SIZE, offset, UIO_WRITE);
	ui.uio_iov = &iov;
	ui.uio_offset  = 0;
	ui.uio_resid = PAGE_SIZE;
	ui.uio_iovcnt= 1;
	ui.uio_segflg = UIO_SYSSPACE;
	ui.uio_rw = UIO_WRITE;
	ui.uio_space = NULL; 

	result=VOP_WRITE(v, &ui);
	if(result){			
		kfree(buf);			//take this part out
		return -1;
	}
	return index;
}
