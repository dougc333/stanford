// memman.cc
//
//  Routines to manage address space on a multiprocess system
//
//

#include "constants.h"
#ifdef PP2_2


#include "memman.h"
#include "machine.h"


MemoryManager::MemoryManager(){

  addrSpaceList = new List<AddrSpace *>;
  memoryMap = new Bitmap(NumPhysPages);
  pageTableSetupLock = new Lock("PageTableSetupLock");
}

MemoryManager::~MemoryManager(){

  while (!addrSpaceList->IsEmpty())
    addrSpaceList->RemoveFront();
  delete addrSpaceList;
  delete memoryMap;
  delete pageTableSetupLock;
}


//----------------------------------------------------------------------
// MemoryManager::SetupAddressSpace
// 
// 1. Checks to see if we have enough memory for the process.
// 2. Sets up page table for the address space
//----------------------------------------------------------------------

  
bool MemoryManager::SetupAddressSpace(AddrSpace *space, unsigned int sizePages)
{
  // Start critical section
  pageTableSetupLock->Acquire();

  // check and see if we have enough space for this address space
  if (sizePages > memoryMap->NumClear())
    return FALSE;

  // Set up page table for new process
  for (unsigned int i = 0; i < sizePages; i++) {
    space->pageTable[i].virtualPage = i;
    space->pageTable[i].physicalPage = memoryMap->FindAndSet();
    space->pageTable[i].valid = TRUE;
    space->pageTable[i].use = FALSE;
    space->pageTable[i].dirty = FALSE;
    space->pageTable[i].readOnly = FALSE;  
  }
  
  addrSpaceList->Append(space);

  // End critical section
  pageTableSetupLock->Release(); 

  return TRUE;
}

//----------------------------------------------------------------------
// MemoryManager::RemoveAddressSpace
// 
// 1. Deallocate the memory for this address space
// 2. Sets up page table for the address space
//----------------------------------------------------------------------

  
void MemoryManager::RemoveAddressSpace(AddrSpace *space)
{
  // Start critical section
  pageTableSetupLock->Acquire();

  // Set up page table for new process
  for (unsigned int i = 0; i < NumPhysPages; i++) {
    if (space->pageTable[i].valid == TRUE) {
      memoryMap->Clear(space->pageTable[i].physicalPage);
      space->pageTable[i].valid = FALSE;
    }
  }
  
  addrSpaceList->Remove(space);

  // End critical section
  pageTableSetupLock->Release(); 
}

//----------------------------------------------------------------------
// MemoryManager::UserToKernel
//   Copy bytesToCopy from userVA to kernelBuf.  If an invalid translation
// occurs then it bails.  It returns the number of bytes copied over.
//----------------------------------------------------------------------

int MemoryManager::UserToKernel(char *kernelBuf, int userVA, int bytesToCopy) {
  int userPA;
  // Copy one byte at a time
  for (int curByte = 0; curByte < bytesToCopy; curByte++) {
    // tranlate userVA to userPA
    userPA = va2pa(userVA + curByte, kernel->currentThread->space);
    if (userPA == -1) return curByte;  // Bailout on invalid translation
    // copy byte into buffer
    kernelBuf[curByte]=kernel->machine->mainMemory[userPA];
  }
  return bytesToCopy;  // indicate all bytes copied
}

//----------------------------------------------------------------------
// MemoryManager::KernelToUserInt
//   Copy bytesToCopy from kernelBuf to userVA.  If an invalid translation
// occurs then it bails.  It returns the number of bytes copied over.
//----------------------------------------------------------------------

int MemoryManager::KernelToUser(char *kernelBuf, int userVA, int bytesToCopy) {
  int userPA;
  // Copy one byte at a time
  if (bytesToCopy>=0){
    for (int curByte = 0; curByte < bytesToCopy; curByte++) {
      // tranlate userVA to userPA
      userPA = va2pa(userVA + curByte, kernel->currentThread->space);
      if (userPA == -1) return curByte;  // Bailout on invalid translation
      // copy byte from buffer
      kernel->machine->mainMemory[userPA]=kernelBuf[curByte];
    }
  }
  else {
    for (int curByte = 0; curByte < -1*bytesToCopy; curByte++) {
      // tranlate userVA to userPA
      userPA = va2pa(userVA - curByte, kernel->currentThread->space);
      if (userPA == -1) return curByte;  // Bailout on invalid translation
      // copy byte from buffer
      kernel->machine->mainMemory[userPA]=kernelBuf[curByte];
    }
  }
  return bytesToCopy;  // indicate all bytes copied
}


//----------------------------------------------------------------------
// MemoryManager::va2pa
//  Performs a virtual address to physical address translation.  Returns
// the physical address or a -1 if the address cannot be tranlated.
//----------------------------------------------------------------------


//this is replaced with a 

int MemoryManager::va2pa(int va, AddrSpace *space) {
  // Is this a valid translation
  if (space->pageTable[va/PageSize].valid == FALSE)
    return -1;
  else // do translation
    return (int) (space->pageTable[va/PageSize].physicalPage *
		  PageSize + (va % PageSize));
}

#endif // PP2_2
