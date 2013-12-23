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
debug = False
host = "137.227.224.97"
port = 2052
maxslept = 30 / 0.05
maxblock = 10240

#Here are the variables needed to be parsed
station = "MACI"
location =""
channel ="BHZ"
network ="IU"

stime = UTCDateTime("2012-01-01T00:00:00.0")

def getString(net,sta,loc,chan):
#This function formats the string to be used in mdget
	debugmdgetString = False

	mdgetString = "-s " + net
	mdgetString += sta.ljust(5,'-')
	mdgetString += chan
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
	data = data.split("\n")
	if debugParseresp:
		print data[0]
#Get the network
	
	datalessobject['Network'] = getvalue("* NETWORK",data)
	datalessobject['Description'] = getvalue("* DESCRIPTION",data)
	datalessobject['Station'] = getvalue("* STATION",data)
	datalessobject['Component'] = getvalue("* COMPONENT",data)
	datalessobject['Location'] = getvalue("* LOCATION",data)
	datalessobject['Sensitivity'] = getvalue("* SENS-SEED",data)
	datalessobject['Latitude'] = getvalue("* LAT-SEED",data)
	datalessobject['Longitude'] = getvalue("* LONG-SEED",data)
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
if debug:
	print importstring

importstring += ' -b ' + modifyDateTime(stime)
importstring +=  " -c r \n"
s.sendall(importstring)

#Now lets get the data
data = s.recv(maxblock)
s.close()

#Now we have the data lets parse it
if debug:
	print data

datalessobject = parseresp(data)
print datalessobject










#Need to include the main part of the program and make things pretty
