#
# Project: UMG Unique Identifier Technology Solution (UITS) Tool
# File:    Makefile
#
# This makefile will build the UITS Tool from the command line.
# It has been tested to run under Ubuntu version 13.04
#  
#  $Date$
#  $Revision$
# 

#
# Compiler tools definitions...
#

CC      = gcc
OPTIM   = -Os -g 
CFLAGS  = $(OPTIM) -I /usr/include/libxml2 -I /usr/include/openssl -I mxml/include -I /usr/include/FLAC
LDFLAGS = $(OPTIM)
OBJECTS = main.o uitsAudioFileManager.o uitsMP3Manager.o uitsPayloadManager.o uitsOpenSSL.o uitsMP4Manager.o uitsFLACManager.o uitsAIFFManager.o uitsWAVManager.o uitsError.o uitsGenericManager.o uitsHTMLManager.o cmePayloadManager.o xmlManager.o uitsDSFManager.o
RM = rm

#
# Rules...
#
#

# the source files are up a directory
vpath %.c ../source

#
# UITS_Tool Target
#
UITS_Tool: $(OBJECTS)
	$(CC) $(LDFLAGS) -o UITS_Tool   $(OBJECTS) mxml/lib/libmxml.a -lxml2 -lssl -lFLAC -lpthread -lcrypto -luuid


#
# UITS_Tool objects
#

.o : %.c
	$(CC) $(CFLAGS) -c $<


#
# Clean everything...
#

clean:
	$(RM) $(OBJECTS)

#
# End 
#
