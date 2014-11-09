//
//  uitsDSFManager.c
//  uits-osx-xcode
//
//  Created by Chris Angelli on 8/9/14.
//
//

#include "uits.h"

char *dsfModuleName = "uitsDSFManager.c";

/*
 *
 * Function: dsfIsValidFile
 * Purpose:	 Determine if an audio file is a DSF file
 *           DSF fileshave 4 chunks: DSD chunk, fmt chunk, data chunk and metadata chunk.
 *           This function checks to see if the file starts with a DSD chunk. If so, it's a DSD file.
 *
 * Returns:   TRUE if DSF, FALSE otherwise
 */

int dsfIsValidFile (char *audioFileName)
{
	
	FILE *audioFP;
	DSF_CHUNK_INFO *dsfChunkInfo = calloc(1, sizeof(DSF_CHUNK_INFO));
		
	audioFP = fopen(audioFileName, "rb");
	uitsHandleErrorPTR(dsfModuleName, "dsfIsValidFile", audioFP, ERR_FILE, "Couldn't open audio file for reading\n");
	
	/* If this is a DSF file, it will start with an 'DSD ' chunk header */
	dsfChunkInfo = dsfReadChunkInfo(audioFP);
	fclose (audioFP);
	
    // DSF tag header starts with 'D' 'S' 'D'
	if (dsfChunkInfo->header[0] == 'D' &&
        dsfChunkInfo->header[1] == 'S' &&
        dsfChunkInfo->header[2] == 'D' ) {
		vprintf("Audio file is DSF\n");
		return (TRUE);
	} else {
		return (FALSE);
	}
	
}

/*
 *
 * Function: dsfEmbedPayload
 * Purpose:	 Embed the UITS payload into an ID3 chunk at the end of the DSF file
 *				1. If there is already an ID3 chunk in the file, add a PRIV tag with UITs data
 *				2. If there is no ID3 chunk, add it
 *				3. Update the DSD header to contain updated total file size and pointer to metadata (ID3) chunk
 *              4. Update the ID3 chunk by adding a PRIV frame containing the UITS data and updating the chunk size
 *            DSD chunk looks like this:
 *                  4 byte  'DSD '
 *                  8 byte   Size of DSD chunk
 *                  8 byte   Total file size
 *                  8 byte   Pointer to Metadata Chunk
 * Returns:   success or ERR
 */
int dsfEmbedPayload		    (char *audioFileName,
							 char *audioFileNameOut,
							 char *uitsPayloadXML,
							 int  numPadBytes)
{
    
 	FILE			*audioInFP, *audioOutFP;
	MP3_ID3_HEADER	*id3Header;
    int64_t metadataChunkPointer = 0;
    int64_t updatedMetadataChunkSize;
    int64_t updatedTotalFileSize;
    unsigned long	audioInFileSize;
    char *existingUITSPayload = NULL;
    
	vprintf("About to embed payload for %s into %s\n", audioFileName, audioFileNameOut);
    
	if (numPadBytes) {
		vprintf("WARNING: Tried to add pad bytes to DSF file. This is not supported.\n");
	}
    
    // make sure there's no existing payload in the output file
    existingUITSPayload = dsfExtractPayload(audioFileName);
    
    if (existingUITSPayload) {
        uitsHandleErrorINT(dsfModuleName, "dsfEmbedPayload", err, 1, ERR_DSF, "DSF file already contains a UITS payload\n");
    }
    
    /* open the audio input and output files */
	audioInFP = fopen(audioFileName, "rb");
	uitsHandleErrorPTR(dsfModuleName, "dsfEmbedPayload", audioInFP, ERR_FILE, "Couldn't open audio file for reading\n");
    
	audioOutFP = fopen(audioFileNameOut, "wb");
	uitsHandleErrorPTR(dsfModuleName, "dsfEmbedPayload", audioOutFP, ERR_FILE, "Couldn't open audio file for writing\n");
    
	/* copy the entire file */
    audioInFileSize = uitsGetFileSize(audioInFP);
    uitsAudioBufferedCopy(audioInFP, audioOutFP, audioInFileSize);
    
    /* The UITS data is written in a metadata chunk, which is an ID3 frame */
    /* If there is an existing ID3 frame, add to it. Otherwise, create an ID3 frame */
    
    metadataChunkPointer = dsfGetMedataChunkPointer(audioInFP);
    
    if (metadataChunkPointer) {
        /* read the original ID3 header for use later */
        fseeko(audioInFP, metadataChunkPointer, SEEK_SET);
        id3Header = mp3ReadID3Header(audioInFP);
    } else {    // need to create the metadata chunk (ID3 frame)
        id3Header = calloc(sizeof(MP3_ID3_HEADER), 1);
        id3Header->majorVersion = 3;
        id3Header->minorVersion = 0;
        id3Header->flags = '\0';
        id3Header->size = 0;    // this will get set to the correct value after PRIV frame is written
        
        /* write the ID3 header to the end of the file */
        fseeko(audioInFP, 0, SEEK_END);
        metadataChunkPointer = ftello(audioInFP);
        err = mp3WriteID3Header(audioOutFP, id3Header);
        uitsHandleErrorINT(dsfModuleName, "dsfEmbedPayload", err, OK, ERR_DSF, "Couldn't write new ID3 header\n");
    }
    
 	/* audioOutFP now points to the end of ID3 data. Write the new PRIV frame */
	err = mp3WritePRIVFrame(audioOutFP, uitsPayloadXML);
	uitsHandleErrorINT(dsfModuleName, "dsfEmbedPayload", err, OK, ERR_DSF, "Couldn't write PRIV frame to audio output file\n");
    
    /* write the new total file size */
    updatedTotalFileSize = ftello(audioOutFP);
    updatedMetadataChunkSize = updatedTotalFileSize - metadataChunkPointer;
    fseeko(audioOutFP, DSF_CHUNK_DEFINITION_SIZE, SEEK_SET);
    // spec says LSB first, but implementations seem otherwise
    //    lswap(&updatedTotalFileSize);
	fwrite(&updatedTotalFileSize, 8, 1, audioOutFP);
    
    /* update the metdata chunk size to reflect the added PRIV frame */
    /* Update the ID3V2 tag size */
	id3Header->size = 	updatedMetadataChunkSize - 10;		/* size does not include 10 header bytes */
    
    fseeko(audioOutFP, metadataChunkPointer, SEEK_SET);
    err = mp3WriteID3Header(audioOutFP, id3Header);
	uitsHandleErrorINT(dsfModuleName, "dsfEmbedPayload", err, OK, ERR_DSF, "Couldn't write  ID3 header\n");
    
    /* cleanup */
	fclose(audioInFP);
	fclose(audioOutFP);
    
    return (OK);
}


int64_t dsfGetMedataChunkPointer (FILE *audioInFP)
{
    DSF_CHUNK_INFO  *dsfChunkInfo;    
    int64_t	audioInFileSize = 0;    // must be 8 bytes
    int64_t metadataChunkPointer = 0;
    int64_t  saveSeek = ftello(audioInFP);
    
    // start at the beginning of the file
    fseeko(audioInFP, 0, SEEK_SET);
    
    /* If this is a DSF file, it will start with an 'DSD ' chunk header */
    dsfChunkInfo = dsfReadChunkInfo(audioInFP);
    
    // DSF tag header starts with 'D' 'S' 'D'
    if (dsfChunkInfo->header[0] != 'D' ||
        dsfChunkInfo->header[1] != 'S' ||
        dsfChunkInfo->header[2] != 'D' ) {
        uitsHandleErrorINT(dsfModuleName, "dsfGetMedataChunkPointer", err, -1, ERR_DSF, "Couldn't read DSD chunk\n");
        return (0);
    }
    
    // seek past chunk header tag and chunk size
    fseeko(audioInFP, DSF_CHUNK_DEFINITION_SIZE, SEEK_SET);
    
    // read in the 8-byte total file size
    err = fread(&audioInFileSize, 1L, 8, audioInFP);
    uitsHandleErrorINT(dsfModuleName, "dsfGetMedataChunkPointer", err, 8, ERR_DSF, "Couldn't read total file size\n");
    
    // read in the 8-byte pointer to metadata chunk
    err = fread(&metadataChunkPointer, 1L, 8, audioInFP);
    uitsHandleErrorINT(dsfModuleName, "dsfGetMedataChunkPointer", err, 8, ERR_DSF, "Couldn't read metadata chunk pointer size\n");
    
    fseeko(audioInFP, saveSeek, SEEK_SET);  // return file pointer to original location
    
    return (metadataChunkPointer);
}

/*
 *
 * Function: dsfExtractPayload
 * Purpose:	 Find and extract the UITS payload from a DSF file
 * Returns:   OK or ERROR
 */

char *dsfExtractPayload		(char *audioFileName)
{
    FILE			*audioInFP;
	MP3_ID3_HEADER	*id3Header;
    int64_t metadataChunkPointer = 0;
    char *uitsPayloadXML = NULL;
	int				frameType;
    unsigned long   fileOffset;
    unsigned long	audioFileSize;
    
    dprintf("dsfExtractPayload\n");
    
    // seek to the start of the metdataChunk
	audioInFP = fopen(audioFileName, "rb");
	uitsHandleErrorPTR(dsfModuleName, "dsfEmbedPayload", audioInFP, ERR_FILE, "Couldn't open audio file for reading\n");
    
    audioFileSize = uitsGetFileSize(audioInFP);

    metadataChunkPointer = dsfGetMedataChunkPointer(audioInFP);
    
    // if there's no metadata chunk, return NULL
    if (!metadataChunkPointer) {
        return NULL;
    }
    
    /* read the original ID3 header for use later */
    fseeko(audioInFP, metadataChunkPointer, SEEK_SET);
	id3Header = mp3ReadID3Header(audioInFP);

    /* read ID3 Tags and Frames, consume padding until a PRIV frame containing the UITS payload is found */
	while (!uitsPayloadXML & (ftello(audioInFP) < audioFileSize)) {
		frameType = mp3IdentifyFrame(audioInFP);
		switch (frameType) {
			case ID3V2TAG:		// this skips the outer metadata boundary, a "tag"
				mp3HandleID3Tag (audioInFP, NULL);
				break;
				
			case ID3FRAME:		// does this frame conatin the UITS Payload?
				uitsPayloadXML =  mp3FindUITSPayload (audioInFP);
				break;
				
			case PADDING:		// skip padding
				mp3SkipPadBytes (audioInFP);
				break;
					
			default:
				vprintf("Unidentified frame at %lld\n", (ftello(audioInFP) - 4));
				break;
		}
 	}
    	
	return (uitsPayloadXML);
}

/*
 *
 * Function: dsfGetMediaHash
 * Purpose:	 Calcluate the media hash for a DSF file
 *				1. Skip the DSD chunk
 *				2. Skip the 'fmt' chunk
 *				3. Read 'data' chunk's sample data
 *                      4 byte 'd' 'a' 't' 'a'
 *                      8 byte size (including header)
 *                      n byte data
 *				4. Read and hash the audio data
 *
 * Returns:   Pointer to the hashed frame data
 */


char *dsfGetMediaHash		(char *audioFileName)
{
	FILE *audioFP;
	DSF_CHUNK_INFO *dsfChunkInfo;
	int nextChunkStart, audioSampleDataSize;
	UITS_digest *mediaHash;
	char *mediaHashString;

    dprintf("dsfGetMediaHash\n");

    audioFP = fopen(audioFileName, "rb");
	uitsHandleErrorPTR(dsfModuleName, "dsfGetMediaHash", audioFP, ERR_FILE, "Couldn't open audio file for reading\n");
	
	/* If this is a DSF file, it will start with an 'DSD ' chunk header */
	dsfChunkInfo = dsfReadChunkInfo(audioFP);
	
    // DSF tag header starts with 'D' 'S' 'D'
	if (dsfChunkInfo->header[0] != 'D' ||
        dsfChunkInfo->header[1] != 'S' ||
        dsfChunkInfo->header[2] != 'D' ) {
        uitsHandleErrorINT(dsfModuleName, "dsfReadChunkInfo", err, -1, ERR_DSF, "Couldn't read DSD chunk\n");
		return (NULL);
	} 

    // skip past DSD chunk
	nextChunkStart = dsfChunkInfo->size;
	fseeko(audioFP, nextChunkStart, SEEK_CUR);
    
	/* read 'fmt' chunk */
	dsfChunkInfo = dsfReadChunkInfo(audioFP);
	
    // Format tag header starts with 'f' 'm' 't'
	if (dsfChunkInfo->header[0] != 'f' ||
        dsfChunkInfo->header[1] != 'm' ||
        dsfChunkInfo->header[2] != 't' ) {
        uitsHandleErrorINT(dsfModuleName, "dsfReadChunkInfo", err, -1, ERR_DSF, "Couldn't read fmt chunk\n");
		return (NULL);
	}

    // skip past fmt chunk
	nextChunkStart = dsfChunkInfo->size;
	fseeko(audioFP, nextChunkStart, SEEK_CUR);
    
	/* read 'data' chunk */
	dsfChunkInfo = dsfReadChunkInfo(audioFP);
	
    // data chunk tag header starts with 'data'
	if (dsfChunkInfo->header[0] != 'd' ||
        dsfChunkInfo->header[1] != 'a' ||
        dsfChunkInfo->header[2] != 't' ||
        dsfChunkInfo->header[3] != 'a' ) {
        uitsHandleErrorINT(dsfModuleName, "dsfReadChunkInfo", err, -1, ERR_DSF, "Couldn't read data chunk\n");
		return (NULL);
	}

    // seek to start of sample data
    
	fseeko(audioFP, DSF_CHUNK_DEFINITION_SIZE, SEEK_CUR);
    audioSampleDataSize = dsfChunkInfo->size - DSF_CHUNK_DEFINITION_SIZE;
	
	mediaHash = uitsCreateDigestBuffered (audioFP, audioSampleDataSize, "SHA256") ;
	
	mediaHashString = uitsDigestToString(mediaHash);
	
	/* cleanup */
	fclose(audioFP);
	
	return (mediaHashString);

}


/*
 *
 * Function: dsfReadChunkHeader
 * Purpose:	 Read an DSF file chunk header and size
 *           chunk header format is :
 *                 type: 4-bytes
 *           chunk size: 8-bytes (sizes are Least Significant Byte first, aka big-endian)
 *
 *           The file pointer is returned to it's original position
 * Passed:   File pointer
 * Returns:  Pointer to header structure or NULL if error
 */


DSF_CHUNK_INFO *dsfReadChunkInfo (FILE *fpin)
{
	DSF_CHUNK_INFO *dsfChunkInfo = calloc(1, sizeof(DSF_CHUNK_INFO));
	unsigned char	header[DSF_HEADER_SIZE];
	int64_t     chunkSize;
    
	dsfChunkInfo->saveSeek = ftello(fpin);

    // read in the 4-byte chunk header, should be 'D' 'S' 'D' ' '
	err = fread(header, 1L, DSF_HEADER_SIZE, fpin);
	uitsHandleErrorINT(dsfModuleName, "dsfReadChunkInfo", err, 4, ERR_DSF, "Couldn't read dsf chunk info\n");
    
    //	printf("%c%c%c%c\n", header[0], header[1], header[2], header[3]);
	
	memcpy (dsfChunkInfo->header, header, DSF_HEADER_SIZE);
    
    // read in the 8-byte chunk size
	err = fread(&chunkSize, 1L, 8, fpin);
	uitsHandleErrorINT(dsfModuleName, "dsfReadChunkInfo", err, 8, ERR_DSF, "Couldn't read dsf chunk size\n");
    
    
    // DSF file format spec says size should be stored LSB first, however test files have MSB first, so no swap
//	lswap(&chunkSize);
	dsfChunkInfo->size = chunkSize;
    

	/* return file pointer to original position */
	
	fseeko(fpin, dsfChunkInfo->saveSeek, SEEK_SET);
    
//    dprintf("Read DSF chunk  header: %s, size: %ld\n", dsfChunkInfo->header, dsfChunkInfo->size);

    return (dsfChunkInfo);
}



