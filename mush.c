/* mush.c
 *  a Minimally Usable SHell
 *  edited by Tina Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAXNEST 8

extern char **environ; /* the environment */
char command[100]; /* the command line */
char * argv[100];  /* result of parsing the command line */
int argc;          /* the number of arguments */
bool errors;       /* were there any errors in it */
char * path;       /* the search path extracted from the environment */
char * patha[100]; /* result of parsing the path */
bool seekable;     /* is this file seekable? */
int loopnest;      /* nesting level of loops */
long int looptop[ MAXNEST ];  /* file positions of loop tops */

void getcommand() {
        char ch;
        int i = 0;
        putchar( '>' );
        do {
                ch = getchar();
                command[i] = ch;
                i++;
        } while (ch != '\n');
        command[i - 1] = '\000';
}

void parseargv() {
        int i = 0;
        int j = 0;
        for (;;) {
                while (command[j] == ' ') j++;
                if (command[j] == '\000') break;
                argv[i] = &command[j];
                i++;
                while ((command[j] != ' ')
                &&     (command[j] != '\000')) {
                        j++;
                }
                if (command[j] == '\000') break;
                command[j] = '\000';
                j++;
        }
        argc = i - 1;
        argv[i] = NULL;
}

void dollarsigns() {
        /* process shell variable substitutions */
        int i = 0; /* loop index */

        while (argv[i] != NULL) {
                if (argv[i][0] == '$') { /* ref to a shell variable */
                        char * value; /* value of the variable */

                        value = getenv( &argv[i][1] );
                        if (value != NULL) { /* value is defined */
                                argv[i] = value;
                        } else { /* value is not defined */
                                printf( "no such variable: %s\n",
                                        argv[i] );
                                errors = true;
                        }
                }
                i++;
        }
}

void getparsepath() {
        /* get the path and pick it apart into patha */
        int i = 0; /* index into patha, array of path part pointers */
        int j = 0; /* index into path, the text of the value of $PATH */

        /* get a pointer to $PATH in the environ */
        path = getenv( "PATH" );

        if (path != NULL) { /* $PATH might not be defined in environ */

                /* make a copy of path so we can still pass environ */
                path = strcpy( malloc( strlen( path ) ), path );

                /* pick out components of path, index them in patha */
                /* this logic was borrowed from parseargv */
                for (;;) {
                        if (path[j] == '\000') break;
                        patha[i] = &path[j];
                        i++;
                        while ((path[j] != ':')
                        &&     (path[j] != '\000')) {
                                j++;
                        }
                        if (path[j] == '\000') break;
                        path[j] = '\000';
                        j++;
                }
        }
        patha[i] = NULL;
}

char * gluepath( char * left, char * right ) {
        /* concatenate left and right, with a / between them */
        char * result;

        /* how long is result string? including / and null terminator */
        result = malloc( strlen( left ) + 1 + strlen( right ) + 1 );

        strcpy( result, left );
        strcat( result, "/" );
        strcat( result, right );
        return result;
}

void launch() {
        /* we flush here to keep output in order */
        fflush( stdout );
        if (fork() == 0) { /*child*/
                int i;

                /* first try it with the literal command */
                execve( argv[0], argv, environ );

                /* if that fails, try with successive path components */
                getparsepath();
                i = 0;
                while (patha[i] != NULL) {
                        execve( gluepath( patha[i], argv[0] ),
                                argv, environ );
                        i++;
                }

                /* if that fails, we are lost */
                printf( "no such command\n" );
                exit( EXIT_FAILURE );
        } else { /*parent*/
                wait( NULL );
        }
}

void doexit() {
        exit( EXIT_SUCCESS );
}

void dosetenv() {
        if (argc < 1) {
                printf( "setenv with missing argument\n" );
        } else if (argc == 1) {
                /* setenv without a specified value */
                int ret = setenv( argv[1], "", 1 );
                if (ret != 0) {
                        printf( "bad variable name\n" );
                }
        } else if (argc == 2) {
                /* the normal setenv with a value specified */
                int ret = setenv( argv[1], argv[2], 1 );
                if (ret != 0) {
                        printf( "bad variable name\n" );
                }
        } else { /* argc > 2 */
                printf( "setenv with extra arguments\n" );
        }
}

void doloop() {
        if (argc > 0) {
                printf( "loop with extra arguments\n" );
        }
        loopnest = loopnest + 1;
        if (loopnest >= MAXNEST) {
                printf( "loop with too much nesting\n" );
        } else {
                looptop[ loopnest ] = ftell( stdin );
        }
}

void dowhile() {
        if (loopnest >= MAXNEST) {
                printf( "while with too much nesting\n" );
                loopnest = loopnest - 1;
        } else if (loopnest < 0) {
                printf( "while with no matching loop command\n" );
        } else if (!seekable) {
                printf( "while when non-seekable input file\n" );
                loopnest = loopnest - 1;
        } else if (argc > 1) {
                printf( "while with too many arguments\n" );
                loopnest = loopnest - 1;
        } else if ((argc == 1) && (argv[1][0] != '\000')) {
                /* no error and argument interpreted as true */
                fseek( stdin, looptop[ loopnest ], SEEK_SET );
                /* we stay in loop -- don't decrement loopnest */
        } else {
                /* no error and argument interpreted as false */
                loopnest = loopnest - 1;
        }
} 

void docommand() {
        /* process built-in commands or launch an application */

        if ( argv[0] == NULL ) {
                /* prevent segmentation fault on blank lines */
        } else if (!strcmp( argv[0], "exit" )) {
                doexit();
        } else if (!strcmp( argv[0], "setenv" )) {
                dosetenv();
        } else if (!strcmp( argv[0], "loop" )) {
                doloop();
        } else if (!strcmp( argv[0], "while" )) {
                dowhile();
        } else { /* not a built-in command */
                launch();
        }
}

int main() {
        /* initialization for loop processing */
        seekable = (ftell( stdin ) >= 0);
        loopnest = -1;

        for (;;) {
                errors = false;
                getcommand();
                parseargv();
                dollarsigns();
                if (!errors) docommand();
        }
}