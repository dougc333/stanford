// openfile.cc 
//	Routines to manage an open Nachos file.  As in UNIX, a
//	file must be open before we can read or write to it.
//	Once we're all done, we can close it (in Nachos, by deleting
//	the OpenFile data structure).
//
//	Also as in UNIX, for convenience, we keep the file header in
//	memory while the file is open.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "filehdr.h"
#include "openfile.h"
#include "main.h"

//----------------------------------------------------------------------
// OpenFile::OpenFile
// 	Open a Nachos file for reading and writing.  Bring the file header
//	into memory while the file is open.
//
//	"sector" -- the location on disk of the file header for this file
//----------------------------------------------------------------------

OpenFile::OpenFile(int sector)
{ 
#ifdef PP4
  hdr = kernel->headManager->Subscribe(sector, &lock);
  lock->Acquire();
  if (!kernel->headManager->IsAlreadyFetched(sector)) {
    hdr->FetchFrom(sector);
    kernel->headManager->SetAlreadyFetched(sector, TRUE);
  }
  lock->Release();
#else 
    hdr = new FileHeader;
    hdr->FetchFrom(sector);
#endif
    seekPosition = 0;
    fileHeaderSector=sector;
}

//----------------------------------------------------------------------
// OpenFile::~OpenFile
// 	Close a Nachos file, de-allocating any in-memory data structures.
//----------------------------------------------------------------------

OpenFile::~OpenFile()
{
#ifdef PP4_4
  kernel->synchDisk->FlushDiskCache(); // write back all file changes to disk
  kernel->headManager->UnSubscribe(fileHeaderSector);
#else
    delete hdr;
#endif // PP4_4
}

//----------------------------------------------------------------------
// OpenFile::Seek
// 	Change the current location within the open file -- the point at
//	which the next Read or Write will start from.
//
//	"position" -- the location within the file for the next Read/Write
//----------------------------------------------------------------------

void
OpenFile::Seek(int position)
{
    seekPosition = position;
}	

//----------------------------------------------------------------------
// OpenFile::Read/Write
// 	Read/write a portion of a file, starting from seekPosition.
//	Return the number of bytes actually written or read, and as a
//	side effect, increment the current position within the file.
//
//	Implemented using the more primitive ReadAt/WriteAt.
//
//	"into" -- the buffer to contain the data to be read from disk 
//	"from" -- the buffer containing the data to be written to disk 
//	"numBytes" -- the number of bytes to transfer
//----------------------------------------------------------------------

int
OpenFile::Read(char *into, int numBytes)
{
   int result = ReadAt(into, numBytes, seekPosition);
   seekPosition += result;
   return result;
}

int
OpenFile::Write(char *into, int numBytes)
{
   int result = WriteAt(into, numBytes, seekPosition);
   seekPosition += result;
   return result;
}

//----------------------------------------------------------------------
// OpenFile::ReadAt/WriteAt
// 	Read/write a portion of a file, starting at "position".
//	Return the number of bytes actually written or read, but has
//	no side effects (except that Write modifies the file, of course).
//
//	There is no guarantee the request starts or ends on an even disk sector
//	boundary; however the disk only knows how to read/write a whole disk
//	sector at a time.  Thus:
//
//	For ReadAt:
//	   We read in all of the full or partial sectors that are part of the
//	   request, but we only copy the part we are interested in.
//	For WriteAt:
//	   We must first read in any sectors that will be partially written,
//	   so that we don't overwrite the unmodified portion.  We then copy
//	   in the data that will be modified, and write back all the full
//	   or partial sectors that are part of the request.
//
//	"into" -- the buffer to contain the data to be read from disk 
//	"from" -- the buffer containing the data to be written to disk 
//	"numBytes" -- the number of bytes to transfer
//	"position" -- the offset within the file of the first byte to be
//			read/written
//----------------------------------------------------------------------

int
OpenFile::ReadAt(char *into, int numBytes, int position)
{
#ifdef PP4
  lock->Acquire();
#endif

    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    char *buf;
#ifdef PP4_4
    bool readAhead = FALSE;
    int newNumBytes = 0;
    int additionalSector;
#endif

#ifdef PP4
    if ((numBytes <= 0) || (position >= fileLength)) {
      lock->Release();
      return 0; 				// check request
    }
#else
    if ((numBytes <= 0) || (position >= fileLength))
    	return 0; 				// check request
#endif
    if ((position + numBytes) > fileLength)		
	numBytes = fileLength - position;
#ifdef PP4_4
    else {
      newNumBytes = numBytes + SectorSize;  // try to get another sector
      if ((position + newNumBytes) > fileLength)		
	newNumBytes = fileLength - position;
      if (newNumBytes == numBytes)
	readAhead = FALSE;
      else {
	readAhead = TRUE;
	additionalSector = divRoundDown(position + newNumBytes - 1, SectorSize);
      }
    }

    DEBUG(dbgCache,("Reading %d bytes at %d from file of length %d. Orignal request of %d bytes\n", 
		    newNumBytes, position, fileLength, numBytes));
#endif
    DEBUG(dbgFile,("Reading %d bytes at %d from file of length %d\n", numBytes, position, fileLength));

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

#ifdef PP4_4
    if (readAhead)
      kernel->synchDisk->ReadAhead(hdr->ByteToSector(lastSector*SectorSize), 
				   hdr->ByteToSector(additionalSector*SectorSize));
#endif 

    // read in all the full and partial sectors that we need
    buf = new char[numSectors * SectorSize];
    for (i = firstSector; i <= lastSector; i++)	
      kernel->synchDisk->ReadSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) * SectorSize]);

    // copy the part we want
    bcopy(&buf[position - (firstSector * SectorSize)], into, numBytes);
 
#ifdef PP4
    lock->Release();
#endif
    delete [] buf;
    return numBytes;
}

int
OpenFile::WriteAt(char *from, int numBytes, int position)
{
#ifdef PP4
  lock->Acquire();
#endif
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    bool firstAligned, lastAligned;
    char *buf;
    int newSectorsNeeded = 0;
#ifndef PP4
    if ((numBytes <= 0) || (position >= fileLength))
	return 0;				// check request
#else
    if (numBytes <= 0) {
      lock->Release();
      return 0;
    }

    //if (numBytes > SectorSize - (fileLength%SectorSize)){
    //if (divRoundUp(fileLength, SectorSize)*SectorSize < position
    //	+numBytes)
    if (hdr->GetNumSectors() * SectorSize < position  + numBytes)
      {
	PersistBitMap *freeMap;
	freeMap = kernel->fileSystem->GetFreeMap(); // new PersistBitMap(kernel->fileSystem->freeMapFile, NumSectors);
	freeMap->FetchFrom();
	
	//we know we need to add sectors, figure out how many sectors to add
	//and continue with the rest of the writeAt.
	//number of sectors we need are 
	//     newSectorsNeeded= divRoundUp((numBytes-(SectorSize -
	//				      (fileLength%SectorSize))) , 
	//			   SectorSize);
	newSectorsNeeded = divRoundUp((numBytes+position)-
				      (hdr->GetNumSectors()*SectorSize), SectorSize);
	
	for (int i=0; i< newSectorsNeeded; i++) {
	  hdr->AddSector(freeMap, fileHeaderSector, FALSE) ;
	}

	freeMap->WriteBack();
	kernel->fileSystem->ReleaseFreeMap(); //delete freeMap;
	if (hdr->GetNumSectors()*SectorSize < position + numBytes)
	  numBytes=hdr->GetNumSectors()*SectorSize-position;
	
	//	hdr->SetNumBytes(position+numBytes);
	//printf("the file Header SEctor: %d\n", fileHeaderSector);
	//if (newSectorsNeeded)
	//	hdr->WriteBack(fileHeaderSector);
      }

      if (fileLength < position + numBytes) {
    //   if (position + numBytes <= hdr->GetNumSectors()*SectorSize) {
	hdr->SetNumBytes(position+numBytes);
	hdr->WriteBack(fileHeaderSector);
      }
    //fileLength = hdr->FileLength();
#endif

  //  if ((position + numBytes) > fileLength)
    //numBytes = fileLength - position;
    DEBUG(dbgFile,("Writing %d bytes at %d to file of length %d\n", numBytes, position, fileLength));

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

    buf = new char[numSectors * SectorSize];

    firstAligned = (position == (firstSector * SectorSize));
    lastAligned = ((position + numBytes) == ((lastSector + 1) * SectorSize));

// read in first and last sector, if they are to be partially modified
    if (!firstAligned)
        ReadAtForWriteAt(buf, SectorSize, firstSector * SectorSize);	
    if (!lastAligned && ((firstSector != lastSector) || firstAligned))
        ReadAtForWriteAt(&buf[(lastSector - firstSector) * SectorSize], 
				SectorSize, lastSector * SectorSize);	

// copy in the bytes we want to change 
    bcopy(from, &buf[position - (firstSector * SectorSize)], numBytes);

// write modified sectors back
    for (i = firstSector; i <= lastSector; i++)	
      kernel->synchDisk->WriteSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) *
					    SectorSize]);
#ifdef PP4
  lock->Release();
#endif

    delete [] buf;
    return numBytes;
}

//----------------------------------------------------------------------
// OpenFile::Length
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
OpenFile::Length() 
{ 
    return hdr->FileLength(); 
}

#ifdef PP4
//----------------------------------------------------------------------
// OpenFile::getHeader
// return the header file pointer
//----------------------------------------------------------------------
FileHeader* 
OpenFile::getHeader()
{
  return hdr;
}        


int
OpenFile::ReadAtForWriteAt(char *into, int numBytes, int position)
{
    int fileLength = hdr->FileLength();
    int i, firstSector, lastSector, numSectors;
    char *buf;
#ifdef PP4_4
    bool readAhead = FALSE;
    int newNumBytes = 0;
    int additionalSector;
#endif

    if ((numBytes <= 0) || (position >= fileLength))
    	return 0; 				// check request
    if ((position + numBytes) > fileLength)		
	numBytes = fileLength - position;
#ifdef PP4_4
    else {
      newNumBytes = numBytes + SectorSize;  // try to get another sector
      if ((position + newNumBytes) > fileLength)		
	newNumBytes = fileLength - position;
      if (newNumBytes == numBytes)
	readAhead = FALSE;
      else {
	readAhead = TRUE;
	additionalSector = divRoundDown(position + newNumBytes - 1, SectorSize);
      }
    }

    DEBUG(dbgCache,("Reading %d bytes at %d from file of length %d. Orignal request of %d bytes\n", 
		    newNumBytes, position, fileLength, numBytes));
#endif
    DEBUG(dbgFile,("Reading %d bytes at %d from file of length %d\n", numBytes, position, fileLength));

    firstSector = divRoundDown(position, SectorSize);
    lastSector = divRoundDown(position + numBytes - 1, SectorSize);
    numSectors = 1 + lastSector - firstSector;

#ifdef PP4_4
    if (readAhead)
      kernel->synchDisk->ReadAhead(hdr->ByteToSector(lastSector*SectorSize), 
				   hdr->ByteToSector(additionalSector*SectorSize));
#endif 

    // read in all the full and partial sectors that we need
    buf = new char[numSectors * SectorSize];
    for (i = firstSector; i <= lastSector; i++)	
      kernel->synchDisk->ReadSector(hdr->ByteToSector(i * SectorSize), 
					&buf[(i - firstSector) * SectorSize]);

    // copy the part we want
    bcopy(&buf[position - (firstSector * SectorSize)], into, numBytes);
 
    delete [] buf;
    return numBytes;
}   
#endif
