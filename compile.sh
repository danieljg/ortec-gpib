#!/bin/bash
echo compiling
gcc ortec.c -o ortec -lgpib_b -lncurses
gcc ortec_994.c -o ortec_994 -lgpib_b -lncurses
gcc ortec_2ch.c -o ortec_2ch -lgpib_b -lncurses
gcc ortec_full.c -o ortec_full -lgpib_b -lncurses
gcc ortec_coinc.c -o ortec_coinc -lgpib_b -lncurses
gcc automatic_micro.c -o automatic_micro -lgpib_b -lncurses
gcc automatic_delay.c -o automatic_delay -lgpib_b -lncurses
gcc temp_measure.c -o temp_measure -lgpib
gcc automatic_temp.c -o automatic_temp -lgpib
gcc automatic_temp_inverse.c -o automatic_temp_inverse -lgpib
gcc micro_measure.c -o micro_measure -lgpib
echo done, now run it as 'sudo ./ortec'
echo or else, as 'sudo ./ortec_2ch'
