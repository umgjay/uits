/*
 *  uitsAudioFileManager.h
 *  UITS_Tool
 *
 *  Created by Chris Angelli on 12/7/09.
 *  Copyright 2009 Universal Music Group. All rights reserved.
 *
 *  $Date$
 *  $Revision$
 *
 */


/*
 * Prevent multiple inclusion...
 */

#ifndef _uitsaudiofileManager_h
#  define _uitsaudiofileManager_h

#define AUDIO_IO_BUFFER_SIZE 500000



/*
 * UITS Supported audio file types
 */ 
enum uitsAudioFileTypes {
	MP3,
	MP4
};

/* The audio callbacks */

typedef int  isValidFileCB		(char*);
typedef char *getMediaHashCB	(char *);
typedef int  embedPayloadCB		(char *, char *, char *, int);
typedef char *extractPaylaodCB	(char *);

typedef struct {
	int					uitsAudioFileType;
	isValidFileCB		*uitsAudioIsValidFile;
	getMediaHashCB		*uitsAudioGetMediaHash;
	embedPayloadCB		*uitsAudioEmbedPayload;
	extractPaylaodCB	*uitsAudioExtractPayload;
} UITS_AUDIO_CALLBACKS;

/*
 * Functions
 *
 */

char	*uitsAudioExtractPayload	(char *audioFileName);

int		uitsAudioEmbedPayload		(char *audioFileName, 
									 char *audioOutFileName, 
									 char *uitsPayloadXML,
									 int  numPadBytes);

char	*uitsAudioGetMediaHash		(char *audioFileName); 

UITS_AUDIO_CALLBACKS *uitsAudioGetCB (char *audioFileName);
int uitsAudioBufferedCopy			(FILE *audioInFP, 
									 FILE *audioOutFP, 
									 unsigned long numBytes);


#endif