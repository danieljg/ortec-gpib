//Program to setup and control both the Ortec 974 counter
//and the Ortec 994 counter for live accidental detection
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
// read a string from the counter
void gpibread( int identifier, char *response) {
 memset(response,0,strsize);
 ibrd(identifier, response, strsize-1);
 //printf("%s\n",response);
}
// write a string to the counter
void gpibwrite(int identifier, char command[strsize]) {
 char response[strsize];
 ibwrt(identifier,command,strlen(command));
 gpibread(identifier, response);
}
// wait for keypress
int keywaiting(int *key) {
  int character;
  character = getch();
  if ((character != ERR) && key) *key = character;
  return (character != ERR);
}

/* main program block */
int main () {
 puts("v0.2\n");
 //define some variables and constants
 char command[strsize]; char buff[32];
 char response[strsize];
 char cdata[strsize]; char bdata[strsize];
 char cdata0[skip]; char cdata1[skip]; char bdata0[skip]; char bdata1[skip];
 char cdata2[skip]; char cdata3[skip];
 int data0; int data1; int data2; int data3; int datab;
 int count_idx; int counter; int counter_b; int j; int character;
 double array1[30]; double array2[30]; double array3[30]; double arrayb[30];
 double average1;   double average2;   double average3;   double averageb;
 const int minor    = 0;
 const int pad      = 4;//configurable on the 974 by switches
 const int pad_994  = 5;//configurable on the 994 by switches
 const int sad      = 0;
 const int send_eoi = 1;
 const int eos_mode = 0;
 const int timeout  = T1s;
 //initialize arrays
 for(j=0;j<30;j++){
  array1[j]=0.0;  array2[j]=0.0; array3[j]; arrayb[j]=0.0;
 }
 //ask for time of measurement
 puts("Looks like we'll be measuring some data. Please choose."); puts("1   second   -- press 'enter'");
 puts("0.2 seconds  -- press 'a' then 'enter'"); puts("[press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x')  return 0;
 //make gnuplot pipe and load the startup script
 FILE *pipe_gp = popen("gnuplot -noraise","w");
 fputs("load 'startup_2ch.p'\n", pipe_gp);
 //find the ortec counters
 counter   =  ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);
 counter_b =  ibdev(minor, pad_994, sad, timeout, send_eoi, eos_mode);
 //read startup string
 gpibread(counter, response); gpibread(counter_b, response);
 //initialize device
 gpibwrite(counter,"INIT\n"); gpibwrite(counter_b,"INIT\n");
 //read ok string
 gpibread(counter, response); gpibread(counter_b, response);
 // set to computer mode
 gpibwrite(counter,"COMPUTER\n"); gpibwrite(counter_b,"COMPUTER\n");
 //set to remote control
 gpibwrite(counter,"ENABLE_REMOTE\n"); gpibwrite(counter_b,"ENABLE_REMOTE\n");
 //auto-transmit counter data at end of scan
 gpibwrite(counter,"ENABLE_ALARM\n"); gpibwrite(counter_b,"ENABLE_ALARM\n");
 //display timer on both counters
 gpibwrite(counter,"SET_DISPLAY 1\n"); gpibwrite(counter_b,"SET_DISPLAY 0\n");
 //set the counter time
 if(buff[0]=='a') { // 0.2 seconds
  gpibwrite(counter,"SET_COUNT_PRESET 2,0\n"); gpibwrite(counter_b,"SET_COUNT_PRESET 02,1\n");
 }
 else { // 1 second
  gpibwrite(counter,"SET_COUNT_PRESET 1,1\n"); gpibwrite(counter_b,"SET_COUNT_PRESET 10,1\n");
 }

 //initialize ncurses
 initscr();
 nodelay(stdscr, TRUE);
 noecho();
 keypad(stdscr, TRUE);
 curs_set(0);

 //counting loop
 for(count_idx=1 ; count_idx <= max_counts ; count_idx++) {
  //clear counters
  gpibwrite(counter,"CLEAR_COUNTERS\n"); gpibwrite(counter_b,"CLEAR_COUNTERS\n");
  //start the count
  gpibwrite(counter,"START\n");  gpibwrite(counter_b,"START\n");
  gpibread(counter, cdata);      gpibread(counter_b, bdata);
  gpibwrite(counter_b,"STOP\n"); gpibwrite(counter,"STOP\n");
  //process the string
  memcpy(cdata0,cdata,8);        cdata0[8]='\0';
  memcpy(cdata1,cdata+skip,8);   cdata1[8]='\0';
  memcpy(cdata2,cdata+2*skip,8); cdata2[8]='\0';
  memcpy(cdata3,cdata+3*skip,8); cdata3[8]='\0';
  data0 = atoi(cdata0); data1 = atoi(cdata1);
  data2 = atoi(cdata2); data3 = atoi(cdata3);
  memcpy(bdata0,bdata,8);        bdata0[8]='\0';
  memcpy(bdata1,bdata+skip,8);   bdata1[8]='\0';
  datab = atoi(bdata1);
  //build the array
  for (j=0;j<30;j++) {
   array1[j]=array1[j+1]; array2[j]=array2[j+1]; array3[j]=array3[j+1]; arrayb[j]=arrayb[j+1];
  }
  array1[29]=data1; array2[29]=data2; array3[29]=data3; arrayb[29]=datab;
  //calculate the average
  average1=0.0; average2=0.0; average3=0.0; averageb=0.0;
  for (j=0;j<10;j++) {
   average1 = average1+array1[29-j]; average2 = average2+array2[29-j];
   average3 = average3+array3[29-j]; averageb = averageb+arrayb[29-j];
  }
  average1=average1/10.0; average2=average2/10.0; average3=average3/10.0; averageb=averageb/10.0;
  //plot some stuff
  fprintf(pipe_gp,"set title \"Ch1=%d (av=%0.1f), Ch2=%d (av=%0.1f), Ch3=%d (av=%0.1f)\\n pairs-accs=%d (av=%0.1f), accs=%d (av=%0.1f)\"\n",data1,average1,data2,average2,data3,average3,data3-datab,average3-averageb,datab,averageb);
  fputs("plot '-' w boxes ti 'channel 1' axes x1y1, '-' w boxes ti 'channel 2' axes x1y2\n", pipe_gp);
  for (j=0;j<30;j++) {
   fprintf(pipe_gp, "%f %f\n", j+0.25 ,array1[j]);
  }
  fprintf(pipe_gp, "e\n");
  for (j=0;j<30;j++) {
   fprintf(pipe_gp, "%f %f\n", j+0.75, array2[j]);
  }
  fprintf(pipe_gp, "e\n");
  //remember to flush!
  fflush(pipe_gp);
  //if theres a keypress, gotta press the killswitch
  if (keywaiting(&character)) break;
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
