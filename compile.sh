#!/bin/bash
# first we do some cleaning
echo cleaning up
rm -f -r linux-gpib-3.2.19
# now we uncompress the library
echo uncompressing
tar -zxvf linux-gpib-3.2.19.tar.gz > /dev/null
#now we replace the ibtest code by our own
echo replacing code
cp ortec.c linux-gpib-3.2.19/examples/ibtest.c
# then we configure and make the thing
echo compiling
cd linux-gpib-3.2.19/
./configure > /dev/null
make > /dev/null
# now we install it
echo installing, may need your password
sudo make install > /dev/null
# done, run as 'sudo ibtest'
echo done, now run it as 'sudo ibtest'
