// synchdisk.cc 
//	Routines to synchronously access the disk.  The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "synch.h"
#include "synchdisk.h"


//----------------------------------------------------------------------
// ::PrintSector()
// 	Dump the data in a disk read/write request, for debugging.
//----------------------------------------------------------------------

static void
PrintSector (bool writing, int sector, char *data)
{
    int *p = (int *) data;

    if (writing)
        printf("Writing sector: %d\n", sector); 
    else
        printf("Reading sector: %d\n", sector); 
    for (unsigned int i = 0; i < (SectorSize/sizeof(int)); i++) {
	printf("%08x ", p[i]);
        if ((i%8)==7) printf("\n");
    }
    printf("\n"); 
}

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//	"name" -- UNIX file name to be used as storage for the disk data
//	   (usually, "DISK")
//----------------------------------------------------------------------

SynchDisk::SynchDisk(char* name)
{
    semaphore = new Semaphore("synch disk", 0);
    lock = new Lock("synch disk lock");
    disk = new Disk(name, this);
#ifdef PP4_4
    cache = new DiskCache(disk, semaphore);
#endif // PP4_4
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
// 	De-allocate data structures needed for the synchronous disk
//	abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk()
{
#ifdef PP4_4
  lock->Acquire();
  delete cache;
  lock->Release();
#endif // PP4_4
    delete disk;
    delete lock;
    delete semaphore;
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
// 	Read the contents of a disk sector into a buffer.  Return only
//	after the data has been read.
//
//	"sectorNumber" -- the disk sector to read
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time
#ifdef PP4_4
    cache->ReadRequest(sectorNumber, data);
#else
    disk->ReadRequest(sectorNumber, data);
    semaphore->P();			// wait for interrupt
#endif // PP4_4
    lock->Release();
    if (debug->IsEnabled('d'))
	PrintSector(FALSE, sectorNumber, data);
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"sectorNumber" -- the disk sector to be written
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{
    if (debug->IsEnabled('d'))
	PrintSector(TRUE, sectorNumber, data);
    
    lock->Acquire();			// only one disk I/O at a time
#ifdef PP4_4
    cache->WriteRequest(sectorNumber, data);
#else
    disk->WriteRequest(sectorNumber, data);    
    semaphore->P();			// wait for interrupt
#endif // PP4_4

    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::CallBack
// 	Disk interrupt handler.  Wake up any thread waiting for the disk
//	request to finish.
//----------------------------------------------------------------------

void
SynchDisk::CallBack()
{ 
    semaphore->V();
}


#ifdef PP4_4
//----------------------------------------------------------------------
// SynchDisk::FlushDiskCache()
// 	Tells the disk cache to write back all dirty sectors
//----------------------------------------------------------------------

void
SynchDisk::FlushDiskCache(void)
{ 
  DEBUG(dbgCache,("SynchDisk is Trying to flush the cache\n"));
  lock->Acquire();
  cache->Flush();
  lock->Release();
}


//----------------------------------------------------------------------
// SynchDisk::ReadAhead()
// 	Tells the disk cache to fetch another sector if appropriate
//----------------------------------------------------------------------

void
SynchDisk::ReadAhead(int last, int next)
{ 
  DEBUG(dbgCache,("SynchDisk is trying to read ahead\n"));

  if (!cache->Exists(last) && !cache->Exists(next)) {
    //printf("Read ahead last= %d next=%d!\n", last, next);
    lock->Acquire();
    cache->LoadFromDisk(next);
    lock->Release();
  }
}
#endif
