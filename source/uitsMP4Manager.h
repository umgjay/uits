/*
 *  uitsMP4Manager.h
 *  UITS_Tool
 *
 *  Created by Chris Angelli on 4/2/10.
 *  Copyright 2010 Universal Music Group. All rights reserved.
 *
 *  $Date$
 *  $Revision$
 *
 */

/*
 * Prevent multiple inclusion...
 */

#ifndef _uitsmp4manager_h_
#  define _uitsmp4manager_h_

#ifdef NO_UUID			/* hack for windows: no uuid lib installed there so hardcode some stuff */
#include "uitsWindows_uuid_parse.h"

#else

#include <uuid/uuid.h>	 /*for storing the UITS payload in a uuid atom */

#endif

#define UUID_SIZE 16	/* 16 hex bytes in a uuid */
/*
 * IO Buffer for reading/writing MP4 files
 */

#define MP4_HEADER_SIZE 8

typedef struct {
	unsigned long size;		
	unsigned char type[5];			/* 4-character type, null-terminated */
	unsigned long saveSeek;	/* saved seek location for the header start */
} MP4_ATOM_HEADER;

typedef struct {
	unsigned char atomType[5];	
	MP4_ATOM_HEADER *atomHeader;
} MP4_NESTED_ATOM;

#define MAX_MP4_SUBTYPES 20

typedef struct {
	char *mp4Subtype;
	char *mp4subtypeDescription;
} MP4_SUBTYPES;

/* 
 * PUBLIC Functions 
 */

int mp4IsValidFile			(char *audioFileName); 

int mp4EmbedPayload		    (char *audioFileName, 
							 char *audioFileNameOut, 
							 char *uitsPayloadXML,
							 int  numPadBytes);

char *mp4ExtractPayload		(char *audioFileName); 
char *mp4ExtractPayload_UITS1 (char *audioFileName);

char *mp4GetMediaHash		(char *audioFileName);

int mp4UpdateChunkOffsetTable(FILE *audioOutFP, int uitsAtomSize);

MP4_ATOM_HEADER *mp4FindAtomHeaderNested (FILE *fpin, MP4_NESTED_ATOM *nestedAtoms);

/*
 * PRIVATE Functions
 */

MP4_ATOM_HEADER *mp4FindAtomHeader (FILE *fpin,  char *atomType, unsigned long endSeek);
MP4_ATOM_HEADER *mp4ReadAtomHeader  (FILE *fpin);
int   mp4CopyAtom			(FILE *fpin, FILE *fpout);
#endif

// EOF


