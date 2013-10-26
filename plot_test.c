#include <stdio.h>
#include <math.h>

int main() {
  double xarray[30];
  double yarray[30];
  FILE *pipe_gp;
  pipe_gp = popen("gnuplot --persist", "w");

//  fputs("set terminal png\n", pipe_gp);
  int i;
  int j;
  for (j=1; j<=200; j++) {
  fputs("plot '-'\n", pipe_gp);

   for (i=0; i<30; ++i) {
    double x = (i+j/2.0)/100.0; 
    xarray[i] = x;
    yarray[i] = x - (x*x)/2.0 + (x*x*x)/6.0 - (x*x*x*x)/24;
    fprintf(pipe_gp, "%f %f\n", xarray[i], yarray[i]);
    }

   fputs("e\n", pipe_gp);

   for (i=0;i<10000000;i++) {}
   }
  pclose(pipe_gp);
  return 0;
}

