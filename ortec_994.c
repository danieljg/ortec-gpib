//Program to run and test the ortec_994 counter
//while plotting the results in real time
//Daniel Jimenez March 2014
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <ncurses.h>
#include <gpib/ib_b.h>
#define strsize    40
#define skip       9
#define max_counts 1800

/* function declarations */
// write a string to the counter
void gpibwrite(int identifier, char command[strsize]) {
 ibwrt(identifier,command,strlen(command));
}
// read a string from the counter
void gpibread( int identifier, char *response) {
 memset(response,0,strsize);
 ibrd(identifier, response, strsize-1);
 //printf("%s\n",response);
}
// wait for keypress
int keywaiting(int *key) {
  int ch;
  ch = getch();
  if ((ch != ERR) && key) *key = ch;
  return (ch != ERR);
}

/* main program block */
int main () {
 puts("v0.1\n");
 //define some variables and constants
 char command[strsize];
 char response[strsize];
 char bdata[strsize];
 char bdata0[skip]; char bdata1[skip];
 int datab;
 int count_idx; int counter_b; int j; int ch;
 double array1[30];
 double average;
 const int minor    = 0;
 const int pad_994  = 5;//configurable on the 994 by switches
 const int sad      = 0;
 const int send_eoi = 1;
 const int eos_mode = 0;
 const int timeout = T1s;
 //initialize array
 for(j=0;j<30;j++){
  array1[j]=0.0;
 }
 //initialize ncurses
 initscr();
 nodelay(stdscr, TRUE);
 noecho();
 keypad(stdscr, TRUE);
 curs_set(0);

 //make gnuplot pipe and load the startup script
 FILE *pipe_gp = popen("gnuplot -noraise","w");
 fputs("load 'startup.p'\n", pipe_gp);

 //find the ortec counter
 counter_b =  ibdev(minor, pad_994, sad, timeout, send_eoi, eos_mode);
 //read startup string
 gpibread(counter_b, response);
 //initialize device
 gpibwrite(counter_b,"INIT\n");
 //read ok string
 gpibread(counter_b, response); gpibread(counter_b, response);
 // set to computer mode
// gpibwrite(counter_b,"COMPUTER\n");
 //read ok string
 gpibread(counter_b, response);
 //set to remote control
// gpibwrite(counter_b,"ENABLE_REMOTE\n");
 gpibread(counter_b, response);
 //auto-transmit counter data at end of scan
 gpibwrite(counter_b,"ENABLE_ALARM\n");
 gpibread(counter_b, response);
 //display channel 2
 gpibwrite(counter_b,"SET_DISPLAY 0\n");
 gpibread(counter_b, response);
 //set the counter for a second
 //gpibwrite(counter,"SET_COU_PR 1,1\n");
 //let's try a fifth of a second
 gpibwrite(counter_b,"SET_COUNT_PRESET 10,1\n"); 
 gpibread(counter_b, response);

 //counting loop
 for(count_idx=1 ; count_idx <= max_counts ; count_idx++) {
  //clear counters
  gpibwrite(counter_b,"CLEAR_COUNTERS\n");
  gpibread(counter_b, response);
  //start the count
  gpibwrite(counter_b,"START\n");
  gpibread(counter_b, response);
  gpibread(counter_b, bdata);
  //stop counter (why?)
  gpibwrite(counter_b,"STOP\n");
  gpibread(counter_b, response);
  //process the string
  memcpy(bdata0,bdata,8);        bdata0[8]='\0';
  memcpy(bdata1,bdata+skip,8);   bdata1[8]='\0';
  datab = atoi(bdata1);
  //build the array
  for (j=0;j<30;j++) {
   array1[j]=array1[j+1];
  }
  array1[29]=datab;
  //calculate the average
  average=0.0;
  for (j=0;j<10;j++) {
   average = average+array1[29-j];
  }
  average=average/10.0;
  //plot some stuff
  fprintf(pipe_gp,"set title \'Ortec test, last = %d, 10-average=%f\'\n",datab,average);
  fputs("plot '-' w boxes ti 'channel B'\n", pipe_gp);
  for (j=0;j<30;j++) {
   fprintf(pipe_gp, "%f %f \n", j+0.5 ,array1[j]);
  }
  fprintf(pipe_gp, "e\n");
  //remember to flush!
  fflush(pipe_gp);
  //if theres a keypress, gotta press the killswitch
  if (keywaiting(&ch)) break;
 }
 //close the pipe
 pclose(pipe_gp);
 //done with ncurses
 clear();
 refresh();
 endwin();
 //end of code
 return 0;
}
