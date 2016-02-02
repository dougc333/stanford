// swapman.cc
//
//  Routines to manage swap space on a multiprocess system
//
//

#include "constants.h"
#ifdef PP3


#include "swapman.h"
#include "machine.h"
#include "openfile.h"

#define SizeOfTheSwap 10 // # of swap pages


SwapManager::SwapManager(){

  swapMap = new Bitmap(SizeOfTheSwap);
  // kernel->fileSystem->Create("SwapFile", SizeOfTheSwap*PageSize);
  kernel->fileSystem->Create("SwapFile", 0);
  swapFile = kernel->fileSystem->Open("SwapFile");
}

SwapManager::~SwapManager(){

  delete swapMap;
  delete swapFile;

}

//----------------------------------------------------------------------
// SwapManager::SwapToMem
//  copy swap page, sspn, to physical mem page ppn
//----------------------------------------------------------------------

bool SwapManager::SwapToMem(int sspn, int ppn)
{
  // zero out page 

  //   bzero(&(kernel->machine->mainMemory[ppn*PageSize]), PageSize);

  swapFile->ReadAt( &(kernel->machine->mainMemory[ppn*PageSize]), 
			       PageSize, sspn * PageSize);
  return TRUE;
}

//----------------------------------------------------------------------
// SwapManager::CopyMemToSwap
// 
// Copies a page in physical memory to a swp space page.  Returns TURE
// if the page was copied successfully, FALSE otherwise.
//----------------------------------------------------------------------

bool SwapManager::CopyMemToSwap(int ppn, int sspn)
{
  int writtenChar;
   
  // write the page to the swap file
  writtenChar = swapFile->WriteAt( &(kernel->machine->mainMemory[ppn*PageSize]), 
				       PageSize, sspn * PageSize);
  // make sure we wrote every thing
  if (writtenChar == PageSize)
    return TRUE;
  else 
    return FALSE;
}

//----------------------------------------------------------------------
// SwapManager::FileToSwap
// 
// 1. 
// 2. 
//----------------------------------------------------------------------

void SwapManager::FileToSwap(OpenFile *executable, Segment *seg, AddrSpace *space)
{
  char PageBuffer[PageSize];
  int sspn;
  if (seg->size > 0) {

      int startPage = seg->virtualAddr/PageSize;

      int numSegPages=divRoundUp(seg->size, PageSize);
/*
      if ((numSegPages==1) && ((divRoundUp(seg->virtualAddr+seg->size,
					  PageSize) - divRoundUp(seg->virtualAddr,
					  PageSize)+1)==2))
	numSegPages++;
	*/
      int addr;
	
      if (numSegPages > 1) {
	int numPagesLoaded = 0;
	int fileOffset = seg->inFileAddr;
	// load in the first page
	
	sspn = space->vpn2ppn(startPage); 
	addr = seg->virtualAddr % PageSize;
	executable->ReadAt(&(PageBuffer[addr]), 
			   PageSize - (seg->virtualAddr % PageSize), 
			   fileOffset);

	swapFile->WriteAt(&(PageBuffer[addr]), PageSize-(seg->virtualAddr % PageSize), 
			   sspn * PageSize + addr);

	fileOffset+=PageSize-(seg->virtualAddr % PageSize);
	numPagesLoaded++;
	// load subsequent blocks
	while (numPagesLoaded<numSegPages-1) {

	  sspn = space->vpn2ppn(startPage + numPagesLoaded);

	  executable->ReadAt(PageBuffer, PageSize, fileOffset);
	  
	  swapFile->WriteAt(PageBuffer, PageSize, sspn * PageSize);

	  fileOffset+=PageSize;
	  numPagesLoaded++;
	}

	// load the last page
	sspn = space->vpn2ppn(startPage + numPagesLoaded);
	executable->ReadAt(PageBuffer, 
			     ((seg->virtualAddr + seg->size) % PageSize), 
			     fileOffset);
	  
	swapFile->WriteAt(PageBuffer, ((seg->virtualAddr + seg->size) % PageSize), sspn * PageSize);

      }
      else { // code resides in only one block
	sspn = space->vpn2ppn(startPage);
	addr = seg->virtualAddr % PageSize;
      
	if (((((seg->virtualAddr + seg->size) % PageSize) - 
	      (seg->virtualAddr % PageSize)) >= 0) || 
	    (seg->virtualAddr + seg->size) % PageSize==0) {

	  executable->ReadAt(&(PageBuffer[addr]), 
			     (seg->virtualAddr + seg->size) - 
			     (seg->virtualAddr), 
			     seg->inFileAddr);
	  
	  swapFile->WriteAt(&(PageBuffer[addr]), 
			    (seg->virtualAddr + seg->size) -
			    (seg->virtualAddr), 
			    sspn * PageSize + addr);
/*	  
	  executable->ReadAt(&(PageBuffer[addr]), 
			     ((seg->virtualAddr + seg->size) % PageSize) - 
			     (seg->virtualAddr % PageSize), 
			     seg->inFileAddr);
	  
	  swapFile->WriteAt(&(PageBuffer[addr]), 
			    ((seg->virtualAddr + seg->size) % PageSize) -
			    (seg->virtualAddr % PageSize), 
			    sspn * PageSize + addr);*/

	} else {
	  int numPagesLoaded = 0;
	  int fileOffset = seg->inFileAddr;
	  // load in the first page
	  
	  sspn = space->vpn2ppn(startPage); 
	  addr = seg->virtualAddr % PageSize;
	  executable->ReadAt(&(PageBuffer[addr]), 
			     PageSize - (seg->virtualAddr % PageSize), 
			     fileOffset);
	  
	  swapFile->WriteAt(&(PageBuffer[addr]), PageSize-(seg->virtualAddr % PageSize), 
			    sspn * PageSize + addr);
	  
	  fileOffset+=PageSize-(seg->virtualAddr % PageSize);
	  numPagesLoaded++;
	  
	  
	  sspn = space->vpn2ppn(startPage + numPagesLoaded);
	  executable->ReadAt(PageBuffer, 
			     ((seg->virtualAddr + seg->size) % PageSize), 
			     fileOffset);
	  
	  swapFile->WriteAt(PageBuffer, ((seg->virtualAddr + seg->size) % PageSize), sspn * PageSize);
	  
	}
      }
  }
}


 
#endif // PP3_1
