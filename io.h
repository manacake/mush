/* io.h -- include file for MP5 */

typedef struct myfile MYFILE;

MYFILE * myfdopen( int fd, const char * mode );
  /* identical specification to fdopen() */

int mygetc( MYFILE * f );
  /* identical specification to fgetc */

long int myseek( MYFILE * f, long int pos );
  /* equivalent to fseek( ... SEEK_SET ) */

long int mytell( MYFILE * f );
  /* identical specification to ftell */