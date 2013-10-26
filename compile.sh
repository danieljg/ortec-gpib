#!/bin/bash
echo compiling
gcc ortec.c -o ortec -lgpib
echo done, now run it as 'sudo ortec'
