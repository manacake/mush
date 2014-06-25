/* mymalloc.c */

#include <stddef.h>
#include <stdint.h>

#include "mymalloc.h"
#include "buddy.h"

void * mymalloc( intptr_t size ) {
	void * p;	/* the allocated block */
	int i = 0;	/* log base 2 of block size */
	int twotothei = 1;
	/* allow for a prefix on the allocated block */
	size = size + sizeof( void * );
	/* compute log base 2 of block size */
	while (twotothei < size) {
		i = i + 1;
		twotothei = twotothei << 1;
	}

	/* try to allocate */
	p = buddyalloc( i );
	if (p == NULL) {
		enlargeheap( i );
		p = buddyalloc( i );
	}
	if (p != NULL) { /* record block size and step forward one */
		*(int *)p = i;
		p = (void *)(((intptr_t)p) + sizeof( void * ));
	}
	return p;
}

void myfree( void * p ) {
	int i;
	/* back up one and recover the record of the block size */
	p = (void *)(((intptr_t)p) - sizeof( void * ));
	i = *(int *)p;
	buddyfree( p, i );
}