//
//  uitsDSFManager.h
//  uits-osx-xcode
//
//  Created by Chris Angelli on 8/9/14.
//
//

#ifndef uits_osx_xcode_uitsDSFManager_h
#define uits_osx_xcode_uitsDSFManager_h

#define DSF_HEADER_SIZE 4
#define DSF_CHUNK_DEFINITION_SIZE 12   /* 4 bytes for header + 8 bytes for size */

typedef struct {
	unsigned char header[5];        /* 4-character header, null-terminated */
	int64_t  size;                  /* 8-byte chunk size, spec says LSB first (aka big-endian number), but test files have MSB first */
	int64_t  saveSeek;              /* saved seek location for the chunk start */
} DSF_CHUNK_INFO;



/*
 *  Function Declarations
 */

/*
 * PUBLIC Functions
 */

int dsfIsValidFile			(char *audioFileName);

int dsfEmbedPayload		    (char *audioFileName,
							 char *audioFileNameOut,
							 char *uitsPayloadXML,
							 int  numPadBytes);

char *dsfExtractPayload		(char *audioFileName);

char *dsfGetMediaHash		(char *audioFileName);


/*
 * PRIVATE Functions
 */

DSF_CHUNK_INFO *dsfReadChunkInfo (FILE *fpin);
int64_t dsfGetMedataChunkPointer (FILE *audioInFP);
MP3_ID3_HEADER	*newID3Header ();


#endif
