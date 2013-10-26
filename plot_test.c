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
  fputs("load 'startup_2ch.p'\n",pipe_gp);
  //fputs("set xrange [*:*]\n",pipe_gp);
  fputs("set yrange [-1:1]\n",pipe_gp);
  for (j=1; j<=100; j++) {
  fputs("plot '-' w boxes, '-' w boxes\n", pipe_gp);

   for (i=0; i<30; ++i) {
    double x = (i+4*j)/100.0; 
    xarray[i] = sin(x);
    yarray[i] = cos(x);
    fprintf(pipe_gp, "%f %f\n",i+0.25, yarray[i]);
    }
   fputs("e\n", pipe_gp);
   for(i=0; i<30; i++) {
    fprintf(pipe_gp,"%f %f\n",i+0.75, xarray[i]);
   }
   fputs("e\n", pipe_gp);
   fflush(pipe_gp);

   for (i=0;i<10000000;i++) {}
   }
  pclose(pipe_gp);
  return 0;
}

