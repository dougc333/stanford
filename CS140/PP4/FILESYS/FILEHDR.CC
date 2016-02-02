// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "debug.h"
#include "filehdr.h"
#include "main.h"
#include "constants.h"
#include "math.h"
//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(PersistBitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
#ifdef PP4
    FileHeader *indirect;               // One level of indirection
    FileHeader *dblIndirect;            // Double indirection

    int hdrSpace = 0;  // space for headers
    if (numSectors > NumDirect) // space for indirect 1
      hdrSpace++;
    if (numSectors > 2 * NumDirect) // space for dbl indirect and tables
      hdrSpace += 1 + divRoundUp(numSectors - (2 * NumDirect), NumDirect);
    if (numSectors > NumDirect*(NumDirect+2)) // space for #2 dbl indirect and tables
      hdrSpace += 1 + divRoundUp(numSectors - (NumDirect*(NumDirect +2)), NumDirect);


    if (freeMap->NumClear() < numSectors + hdrSpace)
	return FALSE;		// not enough space  
#else
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
#endif // PP4

#ifdef PP4
    int dirSectors;
    // fill in the first table
    if (numSectors > NumDirect)
      dirSectors = NumDirect;
    else
      dirSectors = numSectors;

    for (int i = 0; i < dirSectors; i++)
	dataSectors[i] = freeMap->FindAndSet();

    // create an indirect table if needed
    if (numSectors > NumDirect) {
      
      indirectLocation = freeMap->FindAndSet(); // find a sector to hold
      // indirection table
      ASSERT(indirectLocation != -1);

      
      int numIndSects;   
      if (numSectors > 2 * NumDirect) // how much for indirect table
	numIndSects=NumDirect;  
      else 
	numIndSects = numSectors - NumDirect;

      indirect = new FileHeader;
      ASSERT(indirect != NULL);
      indirect->Allocate(freeMap, numIndSects*SectorSize);
      indirect->WriteBack(indirectLocation);
      delete indirect;
    } else {
      indirectLocation = -1;
      indirect = NULL;
    }
     

    // create a dbl indirect table if needed 
    if (numSectors > 2 * NumDirect) {
      dblIndirectLocation = freeMap->FindAndSet(); // find a sector to hold
      // indirection table
      ASSERT(dblIndirectLocation != -1);

      // how many level two table do we need?
      int numDblTables; // = divRoundUp(numSectors - (2 * NumDirect),   NumDirect);
      if (numSectors > (NumDirect*(NumDirect+2)))
	numDblTables = NumDirect;
      else
	numDblTables = divRoundUp(numSectors - (2 * NumDirect), NumDirect);
      // create first indirect table

      dblIndirect = new FileHeader; 
      ASSERT(dblIndirect != NULL);
      dblIndirect->Allocate(freeMap, numDblTables*SectorSize);
      dblIndirect->WriteBack(dblIndirectLocation);
      //delete dblIndirect;

      int level2TblLoc;
      FileHeader *level2Hdr;
      int bytesLeft; // = fileSize - (2 * MaxFileSize);  // # number of
      // bytes left allocate

      if (fileSize > (SectorSize*NumDirect*(NumDirect +2))) 
	bytesLeft = NumDirect*NumDirect*SectorSize;
      else
	bytesLeft = fileSize - (2 * MaxFileSize); 

      int bytesForNow;
      // now create all the 2nd level indirection tables
      for (int i = 0; i < numDblTables; i++) {
	level2TblLoc = dblIndirect->ByteToSector(i * SectorSize);
	level2Hdr = new FileHeader;
	if (bytesLeft > MaxFileSize) {
	  bytesForNow = MaxFileSize;
	  bytesLeft -= MaxFileSize;
	} else {
	  bytesForNow = bytesLeft % MaxFileSize;
	  bytesLeft = 0;
	}
	    
	level2Hdr->Allocate(freeMap, bytesForNow);
	level2Hdr->WriteBack(level2TblLoc);
	delete dblIndirect;
	delete level2Hdr;
      }
    } else {
      dblIndirectLocation = -1;
      dblIndirect = NULL;
    }

     // create a #2 dbl indirect table if needed 
    if (numSectors > NumDirect*(NumDirect +2)) {
      dblIndirectLocation2 = freeMap->FindAndSet(); // find a sector to hold
      // indirection table
      ASSERT(dblIndirectLocation2 != -1);

      // how many level two table do we need?
      int numDblTables = divRoundUp(numSectors - (NumDirect*(NumDirect +2)), NumDirect);
      // create first indirect table

      dblIndirect = new FileHeader; 
      ASSERT(dblIndirect != NULL);
      dblIndirect->Allocate(freeMap, numDblTables*SectorSize);
      dblIndirect->WriteBack(dblIndirectLocation2);
      //delete dblIndirect;

      int level2TblLoc;
      FileHeader *level2Hdr;
      int bytesLeft = fileSize - (2 * MaxFileSize);  // # number of bytes left allocate
      int bytesForNow;
      // now create all the 2nd level indirection tables
      for (int i = 0; i < numDblTables; i++) {
	level2TblLoc = dblIndirect->ByteToSector(i * SectorSize);
	level2Hdr = new FileHeader;
	if (bytesLeft > MaxFileSize) {
	  bytesForNow = MaxFileSize;
	  bytesLeft -= MaxFileSize;
	} else {
	  bytesForNow = bytesLeft % MaxFileSize;
	  bytesLeft = 0;
	}
	    
	level2Hdr->Allocate(freeMap, bytesForNow);
	level2Hdr->WriteBack(level2TblLoc);
	delete dblIndirect;
	delete level2Hdr;
      }
    } else {
      dblIndirectLocation2 = -1;
      dblIndirect = NULL;
    }

#else
    for (int i = 0; i < numSectors; i++)
	dataSectors[i] = freeMap->FindAndSet();
#endif // PP4
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(PersistBitMap *freeMap)
{
#ifdef PP4
  FileHeader *dblIndirect;            // Double indirection
  FileHeader *indirect;
  // Deallocate Data sectors
  for (int i = 0; i < numSectors; i++) {
    int offset = i * SectorSize;
    int curSector = ByteToSector(offset);
    //    ASSERT(freeMap->Test((int) curSector));  // ought to be marked!
    freeMap->Clear((int) curSector);
  }

  // Deallocate sector for indirect
  if (indirectLocation != -1) { 
    indirect = new FileHeader;
    indirect->FetchFrom(indirectLocation);
    //indirect->Deallocate(freeMap);
    //ASSERT(freeMap->Test((int) indirectLocation));  // ought to be marked!
    freeMap->Clear((int) indirectLocation);
    indirectLocation = -1;
    delete indirect;
  }

  // Deallocate sectors used for level 2 tables
  if (dblIndirectLocation != -1) { 
    // Get rid of leve tables
    dblIndirect = new FileHeader;
    dblIndirect->FetchFrom(dblIndirectLocation);
    //dblIndirect->Deallocate(freeMap);
    // Finally get rid of double indirect
    //ASSERT(freeMap->Test((int) dblIndirectLocation));  // ought to be marked!
    freeMap->Clear((int) dblIndirectLocation);
    dblIndirectLocation = -1;
    delete dblIndirect;
  }

  if (dblIndirectLocation2 != -1) { 
    // Get rid of leve tables
    dblIndirect = new FileHeader;
    dblIndirect->FetchFrom(dblIndirectLocation2);
    //dblIndirect->Deallocate(freeMap);
    // Finally get rid of double indirect
    // ASSERT(freeMap->Test((int) dblIndirectLocation2));  // ought to be marked!
    freeMap->Clear((int) dblIndirectLocation2);
    dblIndirectLocation2 = -1;
    delete dblIndirect;
  }
  numBytes = 0;
  numSectors = 0;
#else
  for (int i = 0; i < numSectors; i++) {
    ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
    freeMap->Clear((int) dataSectors[i]);
  }
#endif // PP4
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
  kernel->synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
  kernel->synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
#ifdef PP4
  FileHeader *indirect;               // One level of indirection
  FileHeader *dblIndirect;            // Double indirection
  FileHeader *level2Header;
  int result = -1;
  if(offset  > MaxFileSize*(MaxFileSize +2) - 1) {
    printf("Offset for disk file i/o out of bounds.");
    return -1;
  }

  if (offset < MaxFileSize) 
    return(dataSectors[offset / SectorSize]);
  else if (offset >= MaxFileSize && offset < 2*MaxFileSize){ // We are in
    // the first indirect
    indirect = new FileHeader;
    indirect->FetchFrom(indirectLocation);
    result = indirect->ByteToSector(offset - MaxFileSize);
    delete indirect;
    return result; //indirect->ByteToSector(offset - MaxFileSize);
  } else if ((offset >= 2*MaxFileSize) && (offset < SectorSize*NumDirect*(NumDirect+2)) ) { // We are in the dbl indirect
    dblIndirect = new FileHeader;
    level2Header = new FileHeader;

    dblIndirect->FetchFrom(dblIndirectLocation);

    int localIndex = offset % MaxFileSize;
    int doubleIndex = (int)(floor((offset - 2*MaxFileSize) / MaxFileSize));
    int level2Loc = dblIndirect->ByteToSector(doubleIndex*SectorSize);
    level2Header->FetchFrom(level2Loc);
    result = level2Header->ByteToSector(localIndex);
    delete level2Header;
    delete dblIndirect;
    return result; //level2Header->ByteToSector(localIndex);
  } else if (offset >= SectorSize*NumDirect*(NumDirect+2)) { // We are in the #2 dbl indirect
    dblIndirect = new FileHeader;
    level2Header = new FileHeader;

    dblIndirect->FetchFrom(dblIndirectLocation2);

    int localIndex = offset % MaxFileSize;
    int doubleIndex = (int)(floor((offset -  SectorSize*NumDirect*(NumDirect+2)) / MaxFileSize));
    int level2Loc = dblIndirect->ByteToSector(doubleIndex*SectorSize);
    level2Header->FetchFrom(level2Loc);
    result = level2Header->ByteToSector(localIndex);
    delete level2Header;
    delete dblIndirect;
    return result; //level2Header->ByteToSector(localIndex);
  }

  return -1;
#else
    return(dataSectors[offset / SectorSize]);
#endif
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
#ifdef PP4
        kernel->synchDisk->ReadSector(ByteToSector(i*SectorSize), data);
#else
        kernel->synchDisk->ReadSector(dataSectors[i], data);
#endif // PP4
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}
#ifdef PP4
//----------------------------------------------------------------------
// FileHeader::AddSector(bool Param)
// 	....
//---------------------------------------------------------------------
bool 
FileHeader::AddSector(PersistBitMap *freeMap, int fileHeadSector, bool Param){

/*  if (numSectors > 3718) {
    int freeSectors = freeMap->NumClear();
    printf("Sectors Remaining %d\n", freeSectors);
    printf("NumSectors = %d NumBytes = %d\n", numSectors, numBytes);
  }*/

  if (numSectors < NumDirect) {
    // NumSectors: total # of sectors per disk 

    //FindAndSet returns a free sector
    // numSectors: total # of sectors per file
    dataSectors[numSectors] = freeMap->FindAndSet();
    if (dataSectors[numSectors] == -1 ) return FALSE;
    //Write back the updated attributes for the fileheader for the file
    //we are dealing with

    numSectors++;


    WriteBack(fileHeadSector); // save filehdr to disk

    return TRUE;
  }   
  else 
    if ( numSectors < 2*NumDirect && !Param) {
      // if Param is true we have an indirect or a double indirect
      FileHeader *Indirect = new FileHeader;
      if (indirectLocation == -1) {
	indirectLocation = freeMap->FindAndSet(); // find a sector to hold
	// new indirection table
	if (indirectLocation == -1) {
	  delete Indirect;
	  return FALSE;
	}
	if (!Indirect->Allocate(freeMap, SectorSize)) { //  MaxFileSize)) {
	  delete Indirect;
	  return  FALSE;
	}
	Indirect->WriteBack(indirectLocation);
	numSectors+=1; //NumDirect;
      }
      else {
	Indirect->FetchFrom(indirectLocation);
	Indirect->AddSector(freeMap, indirectLocation,TRUE);
	numSectors++;
      }
      //numSectors++;
      WriteBack(fileHeadSector); // save filehdr to disk

      delete Indirect;
      return TRUE;
    } 
    else 
      if (numSectors < NumDirect*(NumDirect + 2) && !Param ) {
	// if Param is true we have an indirect or a double indirect
	FileHeader *DblIndirect = new FileHeader;
	
	if (dblIndirectLocation == -1) { // we need to make a new dbl
	  // indirect and level 2 header tables
	  dblIndirectLocation = freeMap->FindAndSet();
	  if (dblIndirectLocation == -1) {
	    delete DblIndirect;
	    return FALSE; 
	  }
	  if (!DblIndirect->Allocate(freeMap, SectorSize)) {
	    delete DblIndirect;
	    return FALSE;
	  }
	  DblIndirect->WriteBack(dblIndirectLocation);
	  
	  int level2Loc = DblIndirect->ByteToSector(0);
	  FileHeader *level2Hdr = new FileHeader;
	  //	  if (!level2Hdr->Allocate(freeMap, SectorSize)) return
	  // FALSE;
	  if (!level2Hdr->Allocate(freeMap, SectorSize)) { // MaxFileSize)) {
	    delete DblIndirect;
	    delete level2Hdr;
	    return FALSE;
	  }
	  level2Hdr->WriteBack(level2Loc);
	  numSectors+=1; //NumDirect;
	  delete level2Hdr;
	}
	else { // Dbl indirect exists
	  DblIndirect->FetchFrom(dblIndirectLocation);
	  FileHeader *level2Hdr = new FileHeader;
	  int i=0;
	  bool Done=FALSE;
	  int level2Loc;
	  //Search through the first 2nd level table

	  //	  while (!Done && (i < numSectors-2*NumDirect)) {
	  while (!Done && (i < DblIndirect->GetNumSectors())) {
	    level2Loc = DblIndirect->ByteToSector(i*SectorSize);
	    level2Hdr->FetchFrom(level2Loc);
	    Done = level2Hdr->AddSector(freeMap, level2Loc, TRUE);
	    i++;
	  }
	  level2Hdr->WriteBack(level2Loc);
	  delete level2Hdr;
	  if (!Done) { // then add a new second level table;
	    DblIndirect->AddSector(freeMap, dblIndirectLocation,TRUE);
	    level2Loc = DblIndirect->ByteToSector(i*SectorSize);
	    level2Hdr = new FileHeader;
	    if (!level2Hdr->Allocate(freeMap, SectorSize)) {
	      delete DblIndirect;
	      delete level2Hdr;
	      return FALSE;
	    }
	    level2Hdr->WriteBack(level2Loc);
	    delete level2Hdr;
	  }
	  numSectors++;
	}
	//numSectors++; 
	WriteBack(fileHeadSector); // save filehdr to disk
	delete DblIndirect;
	return TRUE;
      }
      else // dbl indirect #2!!
	if (numSectors < 2*NumDirect*(NumDirect + 1) && !Param ) {
	// if Param is true we have an indirect or a double indirect
	FileHeader *DblIndirect = new FileHeader;
	
	if (dblIndirectLocation2 == -1) { // we need to make a new dbl
	  // indirect and level 2 header tables
	  dblIndirectLocation2 = freeMap->FindAndSet();
	  if (dblIndirectLocation2 == -1) {
	    delete DblIndirect;
	    return FALSE; 
	  }
	  if (!DblIndirect->Allocate(freeMap, SectorSize)) {
	    delete DblIndirect;
	    return FALSE;
	  }
	  DblIndirect->WriteBack(dblIndirectLocation2);
	  
	  int level2Loc = DblIndirect->ByteToSector(0);
	  FileHeader *level2Hdr = new FileHeader;
	  //	  if (!level2Hdr->Allocate(freeMap, SectorSize)) return
	  // FALSE;
	  if (!level2Hdr->Allocate(freeMap, SectorSize)) { // MaxFileSize)) {
	    delete DblIndirect;
	    delete level2Hdr;
	    return FALSE;
	  }
	  level2Hdr->WriteBack(level2Loc);
	  numSectors+=1; //NumDirect;
	  delete level2Hdr;
	}
	else { // Dbl indirect exists
	  DblIndirect->FetchFrom(dblIndirectLocation2);
	  FileHeader *level2Hdr = new FileHeader;
	  int i=0;
	  bool Done=FALSE;
	  int level2Loc;
	  //Search through the first 2nd level table

	  //	  while (!Done && (i < numSectors-2*NumDirect)) {
	  while (!Done && (i < DblIndirect->GetNumSectors())) {
	    level2Loc = DblIndirect->ByteToSector(i*SectorSize);
	    level2Hdr->FetchFrom(level2Loc);
	    Done = level2Hdr->AddSector(freeMap, level2Loc, TRUE);
	    i++;
	  }
	  level2Hdr->WriteBack(level2Loc);
	  delete level2Hdr;
	  if (!Done) { // then add a new second level table;
	    DblIndirect->AddSector(freeMap, dblIndirectLocation2,TRUE);
	    level2Loc = DblIndirect->ByteToSector(i*SectorSize);
	    level2Hdr = new FileHeader;
	    if (!level2Hdr->Allocate(freeMap, SectorSize)) {
	      delete DblIndirect;
	      delete level2Hdr;
	      return FALSE;
	    }
	    level2Hdr->WriteBack(level2Loc);
	    delete level2Hdr;
	  }
	  numSectors++;
	}
	//numSectors++; 
	WriteBack(fileHeadSector); // save filehdr to disk
	delete DblIndirect;
	return TRUE;
      }
      else
	return FALSE;
  
}
//end AddSector
 

int FileHeader::GetNumSectors() {
  return numSectors;
}

void FileHeader::SetNumBytes(int newSize) {
  numBytes = newSize;
}
#endif   
    
    
    
    
    
  

    
