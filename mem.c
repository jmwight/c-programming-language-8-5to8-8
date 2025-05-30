//#include <unistd.h> /* for sbrk */
#include <stddef.h>
#include <errno.h>
#include "mem.h"
#define NALLOC	1024

typedef long long Align; /* most restrictive type except for SSE vector which I
			 * am not worried about */

union header		/* block header: */
{
	struct
	{
		union header *ptr; /* next block if on free list */
		unsigned size; /* size of this block */
	} s;
	Align x; /* force alignment of blocks for cpu optimization */
};

typedef union header Header;

static Header *morecore(unsigned);

static Header base; /* empty list of get started */
static Header *freep = NULL; /* start of free list */

/* malloc: general-purpose storage allocator */
void *malloc(unsigned nbytes)
{
	/* error checking, we are only allowing max 1 Mebibyte */
	if(nbytes <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	Header *p, *prevp;
	//static Header *morecore(unsigned);
	unsigned nunits;

	nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
	if((prevp = freep) == NULL) /* no free list yet */
	{
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}
	for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
	{
		if(p->s.size >= nunits) /* big enough */
		{
			if(p->s.size == nunits) /* exactly */
				prevp->s.ptr = p->s.ptr;
			else
			{
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freep = prevp;
			return (void *)(p+1);
		}
		if(p == freep) /* wrapped around free list */
			if((p = morecore(nunits)) == NULL)
			{
				errno = ENOMEM; /* stdlib error no memory */
				return NULL; /* none left */
			}
	}
}

/* calloc: returns pointer to an array of n elements of nbytes each element */
void *calloc(unsigned n, unsigned nbytes)
{
	return malloc(n * nbytes);
}

/* morecore: ask system for more memory */
static Header *morecore(unsigned nu)
{
	void *cp, *sbrk(int); /* this is in <unistd.h>, could use that instead */
	Header *up;

	if(nu < NALLOC)
		nu = NALLOC;
	cp = sbrk(nu * sizeof(Header));
	if(cp == (void *) - 1) /* no space at all */
		return NULL;
	up = (Header *) cp;
	up->s.size = nu;
	free((void *)(up + 1));
	return freep;
}

/* free: put block ap in free list */
void free(void *ap)
{
	Header *bp, *p;

	bp = (Header *)ap - 1; /* points to block header */

	/* error checking for valid size */
	if(bp->s.size <= 0)
	{
		errno = EINVAL; /* XXX: Is this the right one? Not sure */
		return;
	}

	for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
	{
		if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
			break; /* freed block at start or end of arena */
	}
	if(bp + bp->s.size == p->s.ptr) /* join to upper nbr */
	{
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	}
	else
		bp->s.ptr = p->s.ptr;
	if(p + p->s.size == bp) /* join to lower nbr */
	{
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	}
	else
		p->s.ptr = bp;
	freep = p;
}

/* bfree: frees memory in a static or extern array (not obtained from malloc)
 * and adds to free list */
void bfree(void *ap, unsigned n)
{
	Header *bp = (Header *)ap;
	bp->s.size = n;
	free((void *)(ap + 1));
}
