/* io.c
 * an I/O replacement, but without the O
 * (input only)
 */

#include <unistd.h>
#include <stdlib.h>

#include "io.h"

#define BUFSIZE  31

struct myfile {
	int fd;          /* file descriptor */
	char * buf;	     /* buffer */
	int pos;         /* position */
	int cnt;         /* characters remaining */
	long int bpos;   /* position of buffer in file */
};

MYFILE * myfdopen( int fd, const char * mode ) {
	/* identical specification to fdopen() */
	MYFILE * f = malloc( sizeof(MYFILE) );
	f->fd = fd;
	f->buf = malloc( BUFSIZE );
	f->pos = 0;
	f->cnt = 0;
	f->bpos = 0;
	return f;
}

int mygetc( MYFILE * f ) {
	/* similar to fgetc */
	if (f->pos >= f->cnt) {
		f->bpos = f->bpos + f->cnt;
		f->cnt = read( f->fd, f->buf, BUFSIZE );
		f->pos = 0;
	}
	if (f->cnt == 0) {
		return -1; /* that is FEOF */
	} else {
		char ch = f->buf[f->pos];
		f->pos = f->pos + 1;
		return ch;
	}
}

long int myseek( MYFILE * f, long int pos ) {
	/* similar to fseek( ... SEEK_SET ) */
	int ercode;
	int newpos = pos % BUFSIZE;
	int newbpos = pos - newpos;
	ercode = lseek( f->fd, newbpos, SEEK_SET );
	if (ercode < 0) {
		return -1;
	} else if (newbpos == f->bpos) { /* seek within current buffer */
		f->pos = newpos;
		return 0;
	} else { /* seek must read new buffer */
		f->pos = newpos;
		f->bpos = newbpos;
		f->cnt = read( f->fd, f->buf, BUFSIZE );
		return 0;
	}
	return lseek( f->fd, pos, SEEK_SET );
}

long int mytell( MYFILE * f ) {
	/* similar to ftell */
	return f->bpos + f->pos;
}
