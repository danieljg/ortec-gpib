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
#include "gpib/ib.h"
#define strsize    40
#define skip       9
#define max_counts 60//0

/* function declarations */
// write a string to the counter
void gpibwrite(int identifier, char command[strsize]) {
 ibwrt(identifier,command,strlen(command));
}
// read a string from the counter
void gpibread( int identifier, char *response) {
 ibrd(identifier, response, strsize-1);
// printf("%s\n",response);
}

/* main program block */
int main () {
 puts("v0.0.0.1\n");
 //define some variables and constants
 char command[strsize];
 char response[strsize];
 char cdata[strsize];
 char cdata0[skip]; char cdata1[skip];
 char cdata2[skip]; char cdata3[skip];
 int data0; int data1; int data2; int data3;
 int count_idx;
 int counter;
 int j;
 double array1[30];
 const int minor = 0;
 const int pad   = 4;//configurable on the 974 by switches
 const int sad   = 0;
 const int send_eoi = 1;
 const int eos_mode = 0;
 const int timeout = T1s;
 for(j=0;j<30;j++){
  array1[j]=0.0;
 }

 //make gnuplot pipe and load the startup script
 FILE *pipe_gp = popen("gnuplot -persist","w");
 fputs("load 'startup.p'\n", pipe_gp);

 //find the ortec counter
 counter =  ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);
 //read startup string
 //puts("STARTUP\n");
 gpibread(counter, response);

 //initialize device
 gpibwrite(counter,"INIT\n");
 //read ok string
 memset(response,0,strsize);
 gpibread(counter, response);
 memset(response,0,strsize);
 gpibread(counter, response);

 // set to computer mode
 gpibwrite(counter,"COMPUTER\n");
 //read ok string
 memset(response,0,strsize);
 gpibread(counter, response);
 //set to remote control
 gpibwrite(counter,"ENABLE_REMOTE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 //auto-transmit counter data at end of scan
 gpibwrite(counter,"ENABLE_ALARM\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 //display channel 2
 gpibwrite(counter,"SET_DISP 2\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 //set the counter for a second
 gpibwrite(counter,"SET_COU_PR 1,1\n");
 memset(response,0,strsize);
 gpibread(counter, response);

 for(count_idx=1 ; count_idx <= max_counts ; count_idx++) {

  //clear counters
  gpibwrite(counter,"CL_COU\n");
  memset(response,0,strsize);
  gpibread(counter, response);

  //start the count
  gpibwrite(counter,"START\n");
  memset(response,0,strsize);
  gpibread(counter, response);
  memset(response,0,strsize);
  gpibread(counter, cdata);

  //stop counter (why?)
  gpibwrite(counter,"STOP\n");
  memset(response,0,strsize);
  gpibread(counter, response);

  //process the string
  memcpy(cdata0,cdata,8);        cdata0[8]='\0';
  memcpy(cdata1,cdata+skip,8);   cdata1[8]='\0';
  memcpy(cdata2,cdata+2*skip,8); cdata2[8]='\0';
  memcpy(cdata3,cdata+3*skip,8); cdata3[8]='\0';
  data0 = atoi(cdata0); data1 = atoi(cdata1);
  data2 = atoi(cdata2); data3 = atoi(cdata3);
  //printf("%i  %i  %i  %i\n",data0,data1,data2,data3);

  //build the array
  for (j=0;j<30;j++) {
   array1[j]=array1[j+1];
  }
  array1[29]=data1;

  //plot some stuff
  fputs("plot '-' w boxes ti 'channel 1'\n", pipe_gp);
  //fputs("plot '-' w boxes ti 'channel 1', '-' w boxes ti 'channel 2'\n", pipe_gp);
  for (j=0;j<30;j++) {
   fprintf(pipe_gp, "%f %f \n", j+0.5 ,array1[j]);
  }
  fprintf(pipe_gp, "e\n");
  fflush(pipe_gp);  

  }

/*
 //
 gpibwrite(counter,"\n");
 puts("RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
*/

/*
 puts("3\n");
 gpibread(counter, response);
 puts("4\n");
 gpibread(counter, response);
 //set to computer mode
 gpibread(counter,response);
 gpibwrite(counter,"INIT");
*/


// printf("command %10s '\n", command);

 pclose(pipe_gp);

 return 0;
}
