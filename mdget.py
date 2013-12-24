#!/usr/bin/env python
import socket
from obspy.core import UTCDateTime

#######################################################################
#Code for Mdget in python
#By Adam Ringler
#This is a python version of mdget.  It uses Dave Ketchum's metadata
#server and parse out the important information
#
#Here are the functions
#getString()
#modifyDateTime()
#parseresp()
#getvalue()
#
#######################################################################

#Variables that will not change often
debug = True
host = "137.227.224.97"
port = 2052
maxslept = 60 / 0.05
maxblock = 2*10240

#Here are the variables needed to be parsed
station = "BBSR"
location ="*"
channel ="LHZ"
network ="IU"

stime = UTCDateTime("2012-01-01T00:00:00.0")

def getString(net,sta,loc,chan):
#This function formats the string to be used in mdget
	debugmdgetString = False
#No wild carding of networks
	mdgetString = "-s " + net

#Now we split with * and append a . in front to allow for wild cards
	if "*" in sta:
		mdgetString += ".*".join(sta.split("*"))		
	else:
		mdgetString += sta.ljust(5,'-')
	if "*" in chan:
		mdgetString += ".*".join(chan.split("*"))
	else:
		mdgetString += chan
	if loc == "*":
		mdgetString += ".*".join(loc.split("*"))
	else:	
		mdgetString += loc.ljust(2,'-')
	if debugmdgetString:
		print 'Here is the string: ' + mdgetString
	return mdgetString


def modifyDateTime(timeIn):
#This function modifies a UTCDateTime and returns the string for
#mdget so that it can be used
	debugmodifyDateTime = False
	
	timeStringMdget = str(timeIn.year) + '/' 
	timeStringMdget += str(timeIn.month).zfill(2) + '/'
	timeStringMdget += str(timeIn.day).zfill(2) + '-'
	timeStringMdget += str(timeIn.hour).zfill(2) + ":"
	timeStringMdget += str(timeIn.minute).zfill(2) + ":"
	timeStringMdget += str(timeIn.second).zfill(2) 
	
	if debugmodifyDateTime:
		print 'Here is the string in: ' + timeIn.ctime()
		print 'Here is the string out: ' + timeStringMdget
	return timeStringMdget

def parseresp(data):
#This function parses the data and gets out the key values for 
#the station
	debugParseresp = False
#Here we parse the 
	datalessobject = {}
	if debugParseresp:
		print data[0]
#Get the network as well as the other parameters that are easy
	datalessobject['Network'] = getvalue("* NETWORK",data)
	datalessobject['Station'] = getvalue("* STATION",data)
	datalessobject['Channel'] = getvalue("* COMPONENT",data)
	datalessobject['Location'] = getvalue("* LOCATION",data)
	datalessobject['Start Date'] = getvalue("* EFFECTIVE",data)
	datalessobject['End Date'] = getvalue("* ENDDATE",data)
	datalessobject['Input Unit'] = getvalue("* INPUT UNIT",data)
	datalessobject['Output Unit'] = getvalue("* OUTPUT UNIT",data)
	datalessobject['Description'] = getvalue("* DESCRIPTION",data)
	datalessobject['Sampling Rate'] = getvalue("* RATE (HZ)",data)
	datalessobject['Latitude'] = getvalue("* LAT-SEED",data)
	datalessobject['Longitude'] = getvalue("* LONG-SEED",data)
	datalessobject['Elevation'] = getvalue("* ELEV-SEED",data)
	datalessobject['Depth'] = getvalue("* DEPTH",data)
	datalessobject['Dip'] = getvalue("* DIP",data)
	datalessobject['Azimuth'] = getvalue("* AZIMUTH",data)	
	datalessobject['Instrument Type'] = getvalue("* INSTRMNTTYPE",data)
	datalessobject['Sensitivity'] = getvalue("* SENS-SEED",data)
	
#Now we need to get the poles and zeros
	nzeros = int(getvalue("ZEROS",data))
	npoles = int(getvalue("POLES",data))
	zerosindex = data.index("* ****") + 3
	polesindex = zerosindex + nzeros + 1
	poles = []
	zeros = []
	if debugParseresp:
		print 'Here is the index of the zeros: ' + str(zerosindex)
		print 'Here is the index of the poles: ' + str(polesindex)
		print 'Here is the first zero: ' + str(data[zerosindex])
		print 'Here is the first pole: ' + str(data[polesindex])
		print 'Here is the number of zeros: ' + str(nzeros)
		print 'Here is the number of poles: ' + str(npoles)

#Lets go through the zeros
	for zstart in range(0,nzeros):
#Remove all of the extra white spaces
		curzero = ' '.join(data[zerosindex + zstart].split())
		if debugParseresp:
			print 'Here is what is coming in: ' + curzero
#Split into two		
		curzero = curzero.split()
		curzero = float(curzero[0]) + 1j*float(curzero[1])
		zeros.append(curzero)
		if debugParseresp:
			print 'Here is another zero:' + str(curzero)
	datalessobject['Zeros'] = zeros
#Lets start dealing with the poles
	for pstart in range(0,npoles):
		curpole = ' '.join(data[polesindex + pstart].split())
		if debugParseresp:
			print 'Here is what is coming in: ' + curpole
#Split into two		
		curpole = curpole.split()
		curpole = float(curpole[0]) + 1j*float(curpole[1])
		poles.append(curpole)
		if debugParseresp:
			print 'Here is another pole:' + str(curpole)
	datalessobject['Poles'] = poles	

	return  datalessobject

def getvalue(strSearch, data):
#This is a helper function to not call the search function a bunch
#Currently this function only deals with 1 parameter so it needs to be
#modified to deal with poles and zeros
	value = [s for s in data if strSearch in s]
	value = (value[0].replace(strSearch,'')).strip()
	return value





#We need a function to parse the imput string for various epochs
importstring = getString(network,station,location,channel)

#Open the socket and connect
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))

#Set blocking we will want a different timeout approach
s.setblocking(maxslept)


importstring += ' -b ' + modifyDateTime(stime)
importstring +=  " -c r\n"
if debug:
	print importstring

s.sendall(importstring)

#Now lets get the data
getmoredata = True
datagood = []
while getmoredata:
#Pulling one epoch of data and splitting it
	data = (s.recv(maxblock)).split("\n")
	if debug:
		print data
		print 'Okay moving on'

#Check if we are at the end of the return or not
	if data[-2] == '* <EOR>':
		if debug:
			print 'Found the end of the output'
		getmoredata = False
#Need to parse the response and add it to the datagood variable
	if 'no channels found' in data[0]:
		print 'No channels found\n'
	else:
		datagood.append(parseresp(data))

s.close()

for datablock in datagood:
	print 'Parsed data for: ' + datablock['Station'] + ' ' + \
		datablock['Location'] + ' ' + datablock['Channel']





#Need to include the main part of the program and make things pretty
