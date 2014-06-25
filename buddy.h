/* buddy.h */

/* buddy system allocate one block of size 2 to the i */
void * buddyalloc( int i );

/* buddy system free the block p of size 2 to the i */
void buddyfree( void * p, int i );

/* buddy system enlarge the heap by at least size bytes */
void enlargeheap( int size );