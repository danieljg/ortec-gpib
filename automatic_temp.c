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
 puts("temperature dependence measurement helper v0.2\n");
 //define some variables and constants
 FILE *fileptr; char buff[32];
 char response[strsize];
 char cdata[strsize]; char bdata[strsize];
 char cdata0[skip]; char cdata1[skip]; char bdata0[skip]; char bdata1[skip];
 char cdata2[skip]; char cdata3[skip];
 int data0; int data1; int data2; int data3; int datab;
 int counts_av; int wait_time; 
 int count_idx; int counter; int counter_b; int j; int i;
 double array1[30]; double array2[30]; double array3[30]; double arrayb[30];
 double average1;   double average2;   double average3;   double averageb;
 double tempset; double tempactual; double deltat;
 const int minor    = 0;
 const int pad      = 4;//configurable on the 974 by switches
 const int pad_994  = 5;//configurable on the 994 by switches
 const int sad      = 0;
 const int send_eoi = 1;
 const int eos_mode = 0;
 const int timeout  = T1s;
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
 //set the counter for a second
 gpibwrite(counter,"SET_COUNT_PRESET 1,1\n"); gpibwrite(counter_b,"SET_COUNT_PRESET 10,1\n");
 //friendly hello
 puts("Looks like we'll be measuring some data.");
 //set output file
 puts("Remember: Output will be appended to temp.dat and measurements are from cold to hot");
 fileptr = fopen("temp.dat","a");
 fprintf(fileptr,"#Temp(set) Temp(actual) singles-1 singles-2 coincidences corrected_coinc accidentals");

 //inform the user and ask for number of seconds to average
 puts("Now I'll require some data feeding to aid in data collection.");
 puts("How many seconds will you average per data point? [press x to quit]");
 fgets(buff,32,stdin); if (buff[0]=='x') {fclose(fileptr); return 0;} counts_av=atof(buff);
 fprintf(fileptr," (%is averages) \n",counts_av);
 //get number of points per degree celcius
 puts("How many points per degree celcius will you measure? [press x to quit]");
 fgets(buff,32,stdin); if (buff[0]=='x') {fclose(fileptr); return 0;} deltat=atof(buff);
 deltat=1.0/deltat; printf("deltat = %.2f\n",deltat); //conversion to celcius and print to screen
 //get starting set temperature
 puts("What is the starting temperature (set)? [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} tempset=atof(buff);
 //get starting actual temperature
 puts("What is the actual temperature? [ press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} tempactual=atof(buff);

 //inform the user
 puts("Ready to measure, press enter to begin [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;}

 //one-infinite-loop
 for(;;) {
 //inform the user and reset averages
 puts("Measuring... please wait"); average1=0.0; average2=0.0; average3=0.0; averageb=0.0;
  //counting loop
  for(count_idx=1 ; count_idx <= counts_av ; count_idx++) {
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
  //keep the tally
  average1=average1+data1; average2=average2+data2; average3=average3+data3; averageb=averageb+datab;
  }
 //compute average
 average1=average1/counts_av; average2=average2/counts_av;
 average3=average3/counts_av; averageb=averageb/counts_av;
 //write output
 fprintf(fileptr,"%.1f\t%.1f\t%f\t%f\t%f\t%f\t%f\n",tempset,tempactual,average1,average2,average3,average3-averageb,averageb);
 printf("%.1f\t%.1f\t%f\t%f\t%f\t%f\t%f\n",tempset,tempactual,average1,average2,average3,average3-averageb,averageb);
 //remember to flush!
 fflush(fileptr);
 //calculate new set temperature
 tempset=tempset+deltat;
 //request temperature adjustment and inform user to wait
 printf("Please set the temperature to %f\n",tempset);
 puts("Wait for the actual temperature to stabilize...");
 //read new actual temperature
 puts("Please feed the new actual temperature [press x to quit]");
 puts("no input -> tempactual+deltat -- z -> same temperature");
 printf(" q- %.1f w- %.1f e- %.1f r- %.1f t- %.1f \n a- %.1f s- %.1f d- %.1f f- %.1f g- %.1f \n",tempactual+0.6,tempactual+0.7,tempactual+0.8,tempactual+0.9,tempactual+1.0,tempactual+0.1,tempactual+0.2,tempactual+0.3,tempactual+0.4,tempactual+0.5);
 fgets(buff,32,stdin); if(buff[0]=='x') break;
 if(buff[0]=='a') tempactual=tempactual+0.1;
 else if(buff[0]=='s') tempactual=tempactual+0.2;
 else if(buff[0]=='d') tempactual=tempactual+0.3;
 else if(buff[0]=='f') tempactual=tempactual+0.4;
 else if(buff[0]=='g') tempactual=tempactual+0.5;
 else if(buff[0]=='q') tempactual=tempactual+0.6;
 else if(buff[0]=='w') tempactual=tempactual+0.7;
 else if(buff[0]=='e') tempactual=tempactual+0.8;
 else if(buff[0]=='r') tempactual=tempactual+0.9;
 else if(buff[0]=='t') tempactual=tempactual+1.0;
 else if(buff[0]=='z') tempactual=tempactual;
 else if(buff[0]=='\n') tempactual=tempactual+deltat;
 else tempactual=atof(buff);
 }
 fclose(fileptr);
 //end of code
 puts("REMEMBER TO STORE THE DATA!!!");
 puts("else, you'll suffer...");
 return 0;
}
