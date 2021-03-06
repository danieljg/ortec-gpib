//Program to setup and control the Ortec 974 counter
//while plotting the results in real time
//Daniel Jimenez 23-Oct-2013
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
 char cdata[strsize];
 char cdata0[skip]; char cdata1[skip];
 char cdata2[skip]; char cdata3[skip];
 int data0; int data1; int data2; int data3;
 int count_idx; int counter; int j; int ch;
 double array1[30];
 double average;
 const int minor = 0;
 const int pad   = 4;//configurable on the 974 by switches
 const int sad   = 0;
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
 counter =  ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);
 //read startup string
 gpibread(counter, response);
 //initialize device
 gpibwrite(counter,"INIT\n");
 //read ok string
 gpibread(counter, response);
 gpibread(counter, response);
 // set to computer mode
 gpibwrite(counter,"COMPUTER\n");
 //read ok string
 gpibread(counter, response);
 //set to remote control
 gpibwrite(counter,"ENABLE_REMOTE\n");
 gpibread(counter, response);
 //auto-transmit counter data at end of scan
 gpibwrite(counter,"ENABLE_ALARM\n");
 gpibread(counter, response);
 //display channel 2
 gpibwrite(counter,"SET_DISP 2\n");
 gpibread(counter, response);
 //set the counter for a second
 //gpibwrite(counter,"SET_COU_PR 1,1\n");
 //let's try a fifth of a second
 gpibwrite(counter,"SET_COU_PR 2,0\n"); 
 gpibread(counter, response);

 //counting loop
 for(count_idx=1 ; count_idx <= max_counts ; count_idx++) {
  //clear counters
  gpibwrite(counter,"CL_COU\n");
  gpibread(counter, response);
  //start the count
  gpibwrite(counter,"START\n");
  gpibread(counter, response);
  gpibread(counter, cdata);
  //stop counter (why?)
  gpibwrite(counter,"STOP\n");
  gpibread(counter, response);
  //process the string
  memcpy(cdata0,cdata,8);        cdata0[8]='\0';
  memcpy(cdata1,cdata+skip,8);   cdata1[8]='\0';
  memcpy(cdata2,cdata+2*skip,8); cdata2[8]='\0';
  memcpy(cdata3,cdata+3*skip,8); cdata3[8]='\0';
  data0 = atoi(cdata0); data1 = atoi(cdata1);
  data2 = atoi(cdata2); data3 = atoi(cdata3);
  //build the array
  for (j=0;j<30;j++) {
   array1[j]=array1[j+1];
  }
  array1[29]=data1;
  //calculate the average
  average=0.0;
  for (j=0;j<10;j++) {
   average = average+array1[29-j];
  }
  average=average/10.0;
  //plot some stuff
  fprintf(pipe_gp,"set title \'Ortec test, last = %d, 10-average=%f\'\n",data1,average);
  fputs("plot '-' w boxes ti 'channel 1'\n", pipe_gp);
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
