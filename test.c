#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <ncurses.h>
#include <gpib/ib_b.h>

#define strsize 1024
#define skip    9

void readstring(char *thearray) {
char string[strsize]="STRING";
strcpy(thearray,string);
printf("%s\n",thearray);
}


int keywaiting(int *key) {
  int ch;
  ch = getch();
  if ((ch != ERR) && key) *key = ch;
  return (ch != ERR);
}


int main() {
char s[strsize]="00000010;00001000;00000000;00000000;";
char s0[9];
char s1[9];
char s2[9];
char s3[9];
int i;
puts("v0.1");
for (i=1; i<=1 ; i++){
memcpy(s0,s,8);
s0[8]='\0';
//printf("%c\n",s0[8]);
//printf("%i\n",sizeof(s));
//printf("%i\n",sizeof(s2));
//printf("%s\n",s);
printf("%8.8s\n",s0);
memcpy(s1,s+skip,8);
s1[8]='\0';
printf("%8.8s\n",s1);
memcpy(s2,s+2*skip,8);
s2[8]='\0';
printf("%8.8s\n",s2);
memcpy(s3,s+3*skip,8);
s3[8]='\0';
printf("%8.8s\n",s3);
printf("%i\n",(int) strtol(s0,NULL,10));
printf("%i\n",atoi(s1));
printf("%i\n",atoi(s2));
printf("%i\n",atoi(s3));
}
/*
char array[12];
char *ptrtoarray;
ptrtoarray=array;
readstring(ptrtoarray);
printf("%s\n",s);
readstring(s);
printf("%s\n",ptrtoarray);
printf("%s\n",array);
printf("%s\n",s);
*/


  //initialize ncurses
  initscr();
  nodelay(stdscr, TRUE);
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  int ch;
  for(;;) {
   if (keywaiting(&ch)) break;
  }


  //done with ncurses
  clear();
  refresh();
  endwin();

}














