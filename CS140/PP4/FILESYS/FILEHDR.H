// filehdr.h 
//	Data structures for managing a disk file header.  
//
//	A file header describes where on disk to find the data in a file,
//	along with other information about the file (for instance, its
//	length, owner, etc.)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "constants.h"


#ifndef FILEHDR_H
#define FILEHDR_H

#include "synchdisk.h"
#include "pbitmap.h"

#ifdef PP4
// we now account for the indirect and double indirect pointers
#define NumDirect 	((SectorSize - 5 * sizeof(int)) / sizeof(int))
#else
#define NumDirect 	((SectorSize - 2 * sizeof(int)) / sizeof(int))
#endif // PP4

#define MaxFileSize 	(NumDirect * SectorSize)

// The following class defines the Nachos "file header" (in UNIX terms,  
// the "i-node"), describing where on disk to find all of the data in the file.
// The file header is organized as a simple table of pointers to
// data blocks. 
//
// The file header data structure can be stored in memory or on disk.
// When it is on disk, it is stored in a single sector -- this means
// that we assume the size of this data structure to be the same
// as one disk sector.  Without indirect addressing, this
// limits the maximum file length to just under 4K bytes.
//
// There is no constructor; rather the file header can be initialized
// by allocating blocks for the file (if it is a new file), or by
// reading it from disk.

class FileHeader {
  public:
    bool Allocate(PersistBitMap *bitMap, int fileSize);// Initialize a file header, 
						//  including allocating space 
						//  on disk for the file data
    void Deallocate(PersistBitMap *bitMap);  		// De-allocate this file's 
						//  data blocks

    void FetchFrom(int sectorNumber); 	// Initialize file header from disk
    void WriteBack(int sectorNumber); 	// Write modifications to file header
					//  back to disk

    int ByteToSector(int offset);	// Convert a byte offset into the file
					// to the disk sector containing
					// the byte

    int FileLength();			// Return the length of the file 
					// in bytes

    void Print();			// Print the contents of the file.
#ifdef PP4
    bool FileHeader::AddSector(PersistBitMap *freeMap, int fileHeadSector, bool Param);
    int GetNumSectors();
    void SetNumBytes(int newSize);
#endif

  private:
    int numBytes;			// Number of bytes in the file
    int numSectors;			// Number of data sectors in the file
    int dataSectors[NumDirect];		// Disk sector numbers for each data 
					// block in the file
#ifdef PP4
    int indirectLocation;               // Disk sector of the indirect table
    int dblIndirectLocation;            // Disk sector of the dbl indirect
    int dblIndirectLocation2;
    // table
#endif // PP4
};

#endif // FILEHDR_H
