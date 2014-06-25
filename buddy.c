/*  a buddy system
 */

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include "buddy.h"

/* the maximum block size is 2**(MAX-1) */
#define MAX 30

/* all the freelists are initially empty */
static void * freelists[MAX] = { NULL };

void * buddyalloc( int i ) {
        /* buddy system allocate one block of size 2 to the i */
        void * p;
        if (i >= MAX) return NULL;
        if (freelists[i] != NULL) { /* easy case */
                p = freelists[i];
                freelists[i] = *(void **)p;

        } else { /* freelists[i] == NULL, hard case */
                p = buddyalloc( i + 1 );
                if (p != NULL) { /* split oversized block */
                        void * b = (void *)(((intptr_t) p) ^ (1 << i));
                        *(void **)b = NULL;
                        freelists[i] = b;
                }
        }
        return p;
}

void buddyfree( void * p, int i ) {
        /* buddy system free the block p of size 2 to the i */
        void * b; /* candidate for buddy of p */
        void ** pb = &freelists[i]; /* pointer b */
        /* search for buddy in free list */
        for (;;) { /* loop exit by break*/
                b = *pb;
                if (b == NULL) { /* search reached end of list */
                        /* put p at end of list */
                        *(void **)p = NULL;
                        *pb = p;
                        /* quit loop */
                        break;
                }
                if (((intptr_t)p) == (((intptr_t)b) ^ (1 << i))) {
                        /* unlink b from its free list */
                        *pb = *(void **)b;
                        /* compute address of merged blocks */
                        p = (void *)(((intptr_t)p) & ((intptr_t)b));
                        /* free result of merger */
                        buddyfree( p, i + 1 );
                        break;
                }
                /* walk down list */
                pb = (void **)b;
        }
}

void enlargeheap( int i ) {
        /* enlarge the heap to guarantee a free block of size 2**i */
        intptr_t twotothei;
        void * p;     /* start of new block of memory (or remainder of it ) */
        void * q;     /* end of new block of memory */
        void * nextp; /* proposed next value of p */
        /* guarantee an aligned block of size 2**i by getting twice that */
        i = i + 1;
        twotothei = 1 << i;
        /* guarantee minimum size */
        while (twotothei < sizeof( void * )) {
                i = i + 1;
                twotothei = twotothei << 1;
        }
        /* get a block of memory from p to just before q */
        p = sbrk( twotothei );
        q = sbrk( 0 );
        /* it's possible that allocation will fail */
        if ((p == q) || (((intptr_t)p) == -1) || p == NULL ) return;
        /* at this point, the original value of i is irrelevant */
        /* First chop block into successively larger free blocks */
        i = 0;
        twotothei = 1;
        for (;;) { /* exit by break */
                /* find the largest block compatible with the address */
                while ((((intptr_t)p) & (twotothei)) == 0) {
                        i = i + 1;
                        twotothei = twotothei << 1;
                }
                /* now, p is xxxx10000 and twotothei is 000010000 */
                nextp = (void *)(((intptr_t)p) + twotothei);
                if (nextp >= q) break;
                /* chop off a chip and free it if big enough */
                if (twotothei >= sizeof( void * )) buddyfree( p, i );
                p = nextp;
        }
        /* Second chop what's left into successively smaller free blocks */
        do {
                /* the proposed nextp might be too large */
                while (nextp > q) {
                        i = i - 1;
                        twotothei = twotothei >> 1;
                        nextp = (void *)(((intptr_t)p) + twotothei);
                }
                /* chop off a chip and free it if big enough */
                if (twotothei >= sizeof( void * )) buddyfree( p, i );
                /* walk on to the next step */
                p = nextp;
                i = i - 1;
                twotothei = twotothei >> 1;
                nextp = (void *)(((intptr_t)p) + twotothei);
        } while (p < q);
}