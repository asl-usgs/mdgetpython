mdgetpython
===========

This is a python version of mdget.  This is basically a copy of 
Dave Ketchum's C code for accessing his metadata server.  It is currently
in a very crude form

To DO
===========

Need to verify wild cards for odd situations

Need to setup a main and all the fancy python stuff

Need to include error handling


How to Run
===========
./mdget.py -h
usage: mdget.py [-h] [-s STATION] [-l LOCATION] -n NETWORK [-c CHANNEL] [-d]
                [-t TIME] [-o OUTPUT]

Code to get dataless from mdget

optional arguments:
  -h, --help            show this help message and exit
  -s STATION, --station STATION
                        Name of the station of interest: SSSSS
  -l LOCATION, --location LOCATION
                        Name of the location of interest: LL
  -n NETWORK, --network NETWORK
                        Name of the network of interest: NN
  -c CHANNEL, --channel CHANNEL
                        Name of the channel of interest: CCC
  -d, --debug           Run in debug mode
  -t TIME, --time TIME  Time of Epoch: YYYY DDD
  -o OUTPUT, --output OUTPUT
                        Name of parsed value of interest


