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
#define strsize 12

/* function declarations */
// write a string to the counter
void gpibwrite(int identifier, char command[strsize]) {
 ibwrt(identifier,command,strlen(command));
}
// read a string from the counter
void gpibread( int identifier, char *response) {
 ibrd(identifier, response, strsize-1);
 printf("%s\n",response);
}

/* main program block */
int main () {
 puts("v0.0.0.1\n");
 //define some variables and constants
 char command[strsize];
 char response[strsize];
 int stat;
 int counter;
 const int minor = 0;
 const int pad   = 4;//configurable on the 974 by switches
 const int sad   = 0;
 const int send_eoi = 1;
 const int eos_mode = 0;
 const int timeout = T1s;
 //find the ortec counter
 counter =  ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);
 //read startup string
 puts("STARTUP\n");
 gpibread(counter, response);
 // set to computer mode
 gpibwrite(counter,"COMPUTER\n");
 //read ok string
 puts("COMPUTER MODE RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
/*
 //initialize device
 gpibwrite(counter,"INIT\n");
 //read ok string
 puts("INITIALIZE RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 puts("INITIALIZE RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
*/
 //set to remote control
 gpibwrite(counter,"ENABLE_REMOTE\n");
 puts("REMOTE MODE RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 //auto-transmit counter data at end of scan
 gpibwrite(counter,"ENABLE_ALARM\n");
 puts("ENABLE ALARM RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 //display channel 2
 gpibwrite(counter,"SET_DISP 2\n");
 puts("SET RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);
 //set the counter for a second
 gpibwrite(counter,"SET_COU_PR 1,1\n");
 puts("SET_COU_PR RESPONSE\n");
 memset(response,0,strsize);
 gpibread(counter, response);

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

 return 0;
}

int plot_data () {
 FILE *gp = popen("gnuplot -persist","w");
 fputs("load 'startup.p'\n", gp);
 pclose(gp);
}
