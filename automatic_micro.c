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
  int ch;
  ch = getch();
  if ((ch != ERR) && key) *key = ch;
  return (ch != ERR);
}

/* main program block */
int main () {
 puts("micrometer measurement help tool v0.1\n");
 //define some variables and constants
 FILE *fileptr;
 char buff[32];
 char response[strsize];
 char cdata[strsize];
 char cdata0[skip]; char cdata1[skip];
 char cdata2[skip]; char cdata3[skip];
 int data0; int data1; int data2; int data3;
 int counts_av; int wait_time; int ch;
 int count_idx; int counter; int j; int i;
 int ch_x=120; int ch_o=111; int nmod;
 double array1[30]; double array2[30]; double array3[30];
 double average1; double average2; double average3;
 double tempset; double tempactual; double deltax; double xpos; double xmax;
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
 gpibwrite(counter,"SET_COUNT_PRESET 1,1\n");
 //friendly hello
 puts("Looks like we'll be measuring some data.");

 //set output file
 puts("Remember: Output goes to micro.dat and displacements are from the center -> out.");
 fileptr = fopen("micro.dat","a");

 //inform the user, read working temperature and write to header
 puts("Now I'll require some data feeding to aid in data collection.");
 puts("Please feed me the temperature (set)    [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} tempset=atof(buff);
 puts("Please feed me the temperature (actual) [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} tempactual=atof(buff);
 fprintf(fileptr,"#Temp(set) %.2f\t Temp(actual) %.2f\t Photons/sec\n",tempset,tempactual);

 //read number of seconds to average
 puts("How many seconds will you average per data point? [press x to quit]");
 fgets(buff,32,stdin); if (buff[0]=='x') {fclose(fileptr); return 0;} counts_av=atof(buff);

 //set time delay within data points
 puts("How many seconds will you wait between consecutive measurements? [press x to quit]");
 fgets(buff,32,stdin); if (buff[0]=='x') {fclose(fileptr); return 0;} wait_time=atof(buff);

 //get the number of points per turn and calculate deltax
 puts("How many data points per millimeter? [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} deltax=atof(buff);
 deltax=1000.0/deltax; //conversion to um

 //read the starting position
 puts("What starting position should be recorded? (mm) [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} xpos=atof(buff);
 xpos=1000.0*xpos;

 //read the number of millimeters to scan
 puts("What ending position should be recorded? (mm) [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} xmax=atof(buff);
 xmax=1000.0*xmax;

 //ask for number for modulo 'let-me-know'
 puts("How many data points between a 'letmeknow'? [press x to quit]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;} nmod=atof(buff);

 // finish writing the header
 fprintf(fileptr,"#position(um)\t singles-1 \t singles-2 \t coincidences (%is average)\n",counts_av);

 //inform the user
 puts("Ready to measure, press enter to begin");
 puts("There will be a five second pause, then wait for the beep before moving.");
 puts("[press x to quit]");
 puts("[press any key to quit while measuring]");
 fgets(buff,32,stdin); if(buff[0]=='x') {fclose(fileptr); return 0;}

 //give me some time to walk
 sleep(5);

 //initialize ncurses
 initscr();
 nodelay(stdscr, TRUE);
 noecho();
 keypad(stdscr, TRUE);
 curs_set(0);

 //one-infinite-loop
 for(i=0;;i++) {
 //reset averages
 average1=0.0; average2=0.0; average3=0.0;
 //sound
 if( i==0 || i%(2*nmod)==0 ) {
  system("play -n synth 0.05 sine 1000-2500");
 }

 if( i==0 || i%nmod==0 ) {
  system("play -n synth 0.05 sine 2000-3000");
 }
 system("play -n synth 0.1 sine 400-4000");
  //counting loop
  for(count_idx=1 ; count_idx <= counts_av ; count_idx++) {
  //clear counters
  gpibwrite(counter,"CL_COU\n");
  //start the count
  gpibwrite(counter,"START\n");
  gpibread(counter, cdata);
  //stop counter (why?) [note: could be due to the DIP setting 'recycle'. It's disabled now.]
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
 //sound
 system("play -n synth 0.1 sine 4000-400");
 //compute average
 average1=average1/counts_av; average2=average2/counts_av; average3=average3/counts_av;
 //write output
 fprintf(fileptr,"%f\t%f\t%f\t%f\n",xpos,average1,average2,average3);
 printf("%f\t%f\t%f\t%f\n",xpos,average1,average2,average3);
 //remember to flush!
 fflush(fileptr);
 //advance the position
 xpos=xpos+deltax;
 //if theres a keypress, gotta press the killswitch
 if (keywaiting(&ch)) break;
 //chek if we are have finished
 if (xpos>xmax) break;
 //require adjustment
 puts("Adjust the position of the fiber now [press any key to quit]");
 //sleep for some time
 sleep(wait_time);
 }
 fclose(fileptr);
 //done with ncurses
 clear();
 refresh();
 endwin();
 system("reset");
 //end of code
 puts("REMEMBER TO STORE THE DATA!!!");
 puts("else, you'll suffer...");
 //end sound
 system("play -n synth 1 sine 800-400");
 return 0;
}
