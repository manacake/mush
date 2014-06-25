/* mush.c
 *  a Minimally Usable SHell
 *  edited by Tina Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char command[100];      /* the cmd line */
char *argv[100];        /* result of parsing the cmd line */
char *path;             /* path from environment */
char *ppath[100];       /* result of parsed path */
extern char **environ;  /* the environment */

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
                argv[i] = &command[j]; /* assigns address of c[j] to a[i]*/
                i++;
                while ((command[j] != ' ')
                &&     (command[j] != '\000')) {
                        j++;
                }
                if (command[j] == '\000') break;
                command[j] = '\000';
                j++;
        }
        argv[i] = NULL;
}

void parsepath() {
        int i = 0;      /* index into ppath */
        int j = 0;      /* index into path */
        path = getenv( "PATH" ); /* get pointer to $PATH */
        if (path != NULL) {      /* if path is defined in environ... */
                path = strcpy( malloc( strlen( path )), path); /* make copy of path */
                /* split path using : delimiters, put in them in ppath array */
                for (;;) {
                        if (path[j] == '\000') break;
                        ppath[i] = &path[j];
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
        ppath[i] = NULL;
}

/* helper method to concat path and start of command */
char *glue(char *left, char *right) {
        char *res;
        /* the length of res string, include 1 for '/' and null */
        res = malloc( strlen( left ) + strlen( right ) + 2);
        strcpy(res, left);      /* from our parsed path */
        strcat(res, "/");
        strcat(res, right);     /* start of command */
        return res;
}

void launch() {
        if (fork() == 0) { /*child*/
                execve(argv[0], argv, environ); /* try to execute literal command */
                int i = 0;
                parsepath();                  /* if that fails, try other paths */
                while (ppath[i] != NULL) {
                        execve( glue( ppath[i], argv[0] ), argv, environ);
                        i++;
                }
                printf( "no such command\n" );
                exit( EXIT_FAILURE );
        } else { /*parent*/
                wait( NULL );
        }
}

main(){
        for (;;) {
                getcommand();
                parseargv();
                launch();
        }
}