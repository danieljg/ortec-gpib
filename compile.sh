#!/bin/bash
echo compiling
gcc ortec.c -o ortec -lgpib
gcc ortec_2ch.c -o ortec_2ch -lgpib
echo done, now run it as 'sudo ./ortec'
echo or else, as 'sudo ./ortec_2ch'
