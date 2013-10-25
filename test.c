#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#define strsize 1024


void readstring(char *thearray) {
char string[strsize]="STRING";
strcpy(thearray,string);
printf("%s\n",thearray);
}

int main() {
char s[strsize]="s";
char array[12];
char *ptrtoarray;
ptrtoarray=array;
readstring(ptrtoarray);
printf("%s\n",s);
readstring(s);
printf("%s\n",ptrtoarray);
printf("%s\n",array);
printf("%s\n",s);
}

