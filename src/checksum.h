/*
	File and data checksum generators (electron math) by cat02e@fsu.edu
*/


#ifndef CHECKSUM_H
#define CHECKSUM_H

#include "datatypes.h"

BYTE simpleChecksum(void *buffer, Uint32 len);		// Used in position packets


#define G4_MODIFIER   0x77073096					// I am pretty sure these "constants"
#define G16_MODIFIER  0x076dc419					// are all constants.  That is to
#define G64_MODIFIER  0x1db71064					// say, maybe they are dependant on
#define G256_MODIFIER 0x76dc4190					// the key provided to the algorithm.

void generate4(Uint32 *offset, Uint32 key);			// Attack of the evil
void generate16(Uint32 *offset, Uint32 key);		// dictionary-generating
void generate64(Uint32 *offset, Uint32 key);		// algorithms from
void generateDictionary(Uint32 *offset, Uint32 key);// the 4th dimension!

Uint32 getFileChecksum(void *buffer,
					   Uint32 *dictionary,
					   Uint32 len);					// Produce a checksum of file contents

Uint32 getFileChecksum(char *fileName,
					   Uint32 *dictionary);			// Produce a checksum of file contents

Uint32 generateLevelChecksum(Uint32 key,
							 char *mapData);		// Security checksum::Level checksum
Uint32 generateParameterChecksum(Uint32 key,
								 Uint32 *settings);	// Security checksum::Parameter checksum

Uint32 generateEXEChecksum(Uint32 key);				// Security checksum::EXE checksum

/*
Uint32 generateEXEChecksum(Uint32 key,
						   char *fileName);			// Security checksum::EXE checksum
Uint32 generatePartialEXE(Uint32 key,
						  char *buffer,
						  Uint32 len);				// WARNING: this will delete[] buffer
*/

#endif	// CHECKSUM_H
