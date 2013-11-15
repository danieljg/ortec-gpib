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
#include <gpib/ib.h>
#define strsize    40
#define skip       9
#define counts_av  10

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

/* main program block */
int main () {
 puts("measuring help tool v0.0.1\n");
 //define some variables and constants
 FILE *fileptr;
 char buff[32];
 char response[strsize];
 char cdata[strsize];
 char cdata0[skip]; char cdata1[skip];
 char cdata2[skip]; char cdata3[skip];
 int data0; int data1; int data2; int data3;
 int count_idx; int counter; int j;
 int ch_x=120; int ch_o=111;
 double array1[30]; double array2[30]; double array3[30];
 double average1; double average2; double average3;
 double tempset; double tempactual;
 const int minor = 0;
 const int pad   = 4;//configurable on the 974 by switches
 const int sad   = 0;
 const int send_eoi = 1;
 const int eos_mode = 0;
 const int timeout = T1s;
 //find the ortec counter
 counter =  ibdev(minor, pad, sad, timeout, send_eoi, eos_mode);
 //read startup string
 gpibread(counter, response);
 //initialize device
 gpibwrite(counter,"INIT\n");
 //read ok string
 gpibread(counter, response);
 // set to computer mode
 gpibwrite(counter,"COMPUTER\n");
 //set to remote control
 gpibwrite(counter,"ENABLE_REMOTE\n");
 //auto-transmit counter data at end of scan
 gpibwrite(counter,"ENABLE_ALARM\n");
 //display channel 2
 gpibwrite(counter,"SET_DISP 2\n");
 //set the counter for a second
 gpibwrite(counter,"SET_COU_PR 1,1\n");
 //friendly hello
 puts("Looks like we'll be measuring some data.");
 //set output file
 puts("Output goes to out.dat");
 fileptr = fopen("out.dat","a");
 fprintf(fileptr,"#Temp(set) Temp(actual) Photons/sec\n");
 puts("Now I'll require some data feeding.");
 //one-infinite-loop
 for(;;) {
 puts("Temp (set) [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') break; tempset=atof(buff);
 puts("Temp (actual) [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') break; tempactual=atof(buff);
 average=0.0;
  //counting loop
  for(count_idx=1 ; count_idx <= counts_av ; count_idx++) {
  //clear counters
  gpibwrite(counter,"CL_COU\n");
  //start the count
  gpibwrite(counter,"START\n");
  gpibread(counter, cdata);
  //stop counter (why?)
  gpibwrite(counter,"STOP\n");
  //process the string
  memcpy(cdata0,cdata,8);        cdata0[8]='\0';
  memcpy(cdata1,cdata+skip,8);   cdata1[8]='\0';
  memcpy(cdata2,cdata+2*skip,8); cdata2[8]='\0';
  memcpy(cdata3,cdata+3*skip,8); cdata3[8]='\0';
  data0 = atoi(cdata0); data1 = atoi(cdata1);
  data2 = atoi(cdata2); data3 = atoi(cdata3);
  //keep the tally
  average1=average1+data1; average2=average2+data2; average3=average3+data3;
  }
 //compute average
 average1=average1/counts_av; average2=average2/counts_av; average3=average3/counts_av;
 //write output
 fprintf(fileptr,"%f\t%f\t%f\t%f\t\%f\n",tempset,tempactual,average1,average2,average3);
 //fprintf(fileptr,"%f\t%f\n",tempset,tempactual);
 }
 fclose(fileptr);
 //end of code
 puts("REMEMBER TO STORE THE DATA!!!");
 puts("else, you'll suffer...");
 return 0;
}
