// disk.cc 
//	Routines to simulate a physical disk device; reading and writing
//	to the disk is simulated as reading and writing to a UNIX file.
//	See disk.h for details about the behavior of disks (and
//	therefore about the behavior of this simulation).
//
//	Disk operations are asynchronous, so we have to invoke an interrupt
//	handler when the simulated operation completes.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "disk.h"
#include "debug.h"
#include "main.h"
#include "machine.h"
#include "machine_defs.h"

#define CMD_READ  0x28
#define CMD_WRITE 0x2a

//----------------------------------------------------------------------
// Disk::Disk()
//	"toCall" -- object to call when disk read/write request completes
//----------------------------------------------------------------------

Disk::Disk(char* name, CallBackObj *toCall)
{
    DEBUG(dbgDisk,("Initializing the disk.\n"));
    callWhenDone = toCall;
    active = FALSE;
    (void)name;
}

//----------------------------------------------------------------------
// Disk::~Disk()
// 	Clean up disk simulation, by closing the UNIX file representing the
//	disk.
//----------------------------------------------------------------------

Disk::~Disk()
{
}

//----------------------------------------------------------------------
// Disk::PrintSector()
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
// Disk::ReadRequest/WriteRequest
//	   Set up an interrupt handler to be called later,
//	      that will notify the caller when the simulator says
//	      the operation has completed.
//
//	Note that a disk only allows an entire sector to be read/written,
//	not part of a sector.
//
//	"sectorNumber" -- the disk sector to read/write
//	"data" -- the bytes to be written, the buffer to hold the incoming bytes
//----------------------------------------------------------------------

void
Disk::ReadRequest(int sectorNumber, char* data)
{
    ASSERT((unsigned int)data > K0BASE);
    ASSERT(!active);				// only one request at a time
    ASSERT((sectorNumber >= 0) && (sectorNumber < NumSectors));
    
    DEBUG(dbgDisk,("Reading from sector %d\n", sectorNumber));

    volatile DevDiskRegisters* diskr = DEV_DISK_REGISTERS(0, 0, 1);

    diskr->interruptNode = 0;
    diskr->offset        = (unsigned int)data % PageSize;

    int numPages = 0;
    register char *physaddr = data;
    while (physaddr < (data + SectorSize)) {
      diskr->k0Addr[numPages] = (DevRegister) physaddr;
      physaddr += PageSize;
      if (numPages == 0) {
        physaddr -= diskr->offset;
      }
      numPages++;
    }
    ASSERT(numPages < DEV_DISK_MAX_DMA_LENGTH);
    diskr->k0Addr[numPages] = (DevRegister)0;

    diskr->command[0] = (DevRegister) CMD_READ;
    diskr->command[2] = (DevRegister) ((sectorNumber >> 24) & 0xff);
    diskr->command[3] = (DevRegister) ((sectorNumber >> 16) & 0xff);
    diskr->command[4] = (DevRegister) ((sectorNumber >>  8) & 0xff);
    diskr->command[5] = (DevRegister) ((sectorNumber      ) & 0xff);
    diskr->command[7] = (DevRegister) ((1            >>  8) & 0xff);
    diskr->command[8] = (DevRegister) ((1                 ) & 0xff);

    diskr->startIO    = (DevRegister)0;

    if (debug->IsEnabled('d'))
	PrintSector(FALSE, sectorNumber, data);
    
    active = TRUE;
    UpdateLast(sectorNumber);
    kernel->stats->numDiskReads++;
    kernel->interrupt->Schedule(this, 1000, DiskInt);
}

void
Disk::WriteRequest(int sectorNumber, char* data)
{
    ASSERT(!active);
    ASSERT((sectorNumber >= 0) && (sectorNumber < NumSectors));
    
    DEBUG(dbgDisk,("Writing to sector %d\n", sectorNumber));

    volatile DevDiskRegisters* diskr = DEV_DISK_REGISTERS(0, 0, 1);

    diskr->interruptNode = 0;
    diskr->offset        = (unsigned int)data % PageSize;

    int numPages = 0;
    register char *physaddr = data;
    while (physaddr < (data + SectorSize)) {
      diskr->k0Addr[numPages] = (DevRegister) physaddr;
      physaddr += PageSize;
      if (numPages == 0) {
        physaddr -= diskr->offset;
      }
      numPages++;
    }
    ASSERT(numPages < DEV_DISK_MAX_DMA_LENGTH);
    diskr->k0Addr[numPages] = (DevRegister)0;

    diskr->command[0] = (DevRegister) CMD_WRITE;
    diskr->command[2] = (DevRegister) ((sectorNumber >> 24) & 0xff);
    diskr->command[3] = (DevRegister) ((sectorNumber >> 16) & 0xff);
    diskr->command[4] = (DevRegister) ((sectorNumber >>  8) & 0xff);
    diskr->command[5] = (DevRegister) ((sectorNumber      ) & 0xff);
    diskr->command[7] = (DevRegister) ((1            >>  8) & 0xff);
    diskr->command[8] = (DevRegister) ((1                 ) & 0xff);

    diskr->startIO    = (DevRegister)0;

    if (debug->IsEnabled('d'))
	PrintSector(TRUE, sectorNumber, data);
    
    active = TRUE;
    UpdateLast(sectorNumber);
    kernel->stats->numDiskWrites++;
    kernel->interrupt->Schedule(this, 1000, DiskInt);
}

//----------------------------------------------------------------------
// Disk::CallBack()
// 	Called by the machine simulation when the disk interrupt occurs.
//----------------------------------------------------------------------

void
Disk::CallBack ()
{ 
    volatile DevDiskRegisters* diskr = DEV_DISK_REGISTERS(0, 0, 1);

    if (diskr->intr_pending) {
      DevRegister status = diskr->errnoVal;
      ASSERT(status == 0);
      
      int bytesTransferred = (int) diskr->bytesTransferred;
      ASSERT(bytesTransferred == SectorSize);
      
      diskr->intr_pending = 0;
      diskr->doneIO = (DevRegister)0;
      active = FALSE;
      callWhenDone->CallBack();
    } else {
      ASSERT(0);
    }
}

//----------------------------------------------------------------------
// Disk::UpdateLast
//   	Keep track of the most recently requested sector.  So we can know
//	what is in the track buffer.
//----------------------------------------------------------------------

void
Disk::UpdateLast(int newSector)
{
    lastSector = newSector;
    DEBUG(dbgDisk,("Updating last sector = %d\n", lastSector));
}
