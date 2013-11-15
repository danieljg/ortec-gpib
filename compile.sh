#!/bin/bash
echo compiling
gcc ortec.c -o ortec -lgpib_b -lncurses
gcc ortec_2ch.c -o ortec_2ch -lgpib_b -lncurses
gcc ortec_coinc.c -o ortec_coinc -lgpib_b -lncurses
gcc ortec_measure.c -o ortec_measure -lgpib
echo done, now run it as 'sudo ./ortec'
echo or else, as 'sudo ./ortec_2ch'
