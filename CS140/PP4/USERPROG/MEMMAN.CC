// memman.cc
//
//  Routines to manage address space on a multiprocess system
//
//

#include "constants.h"
#ifdef PP2_2


#include "memman.h"
#include "machine.h"
#include "addrspace.h"

MemoryManager::MemoryManager(){

  addrSpaceList = new List<AddrSpace *>;
  memoryMap = new Bitmap(NumPhysPages);
  pageTableSetupLock = new Lock("PageTableSetupLock");
#ifdef PP3_1
  tlbLock = new Lock("tlbLock");
  ipt = new HashTable<int, IPTEntry *>(iptHashKey, iptHashFunc);
#ifndef PP4
  swapMan = new SwapManager;
#endif //PP4
#endif // PP3_1
#ifdef RANDOM
  RandomInit((unsigned) this);
#endif
}

MemoryManager::~MemoryManager(){

  while (!addrSpaceList->IsEmpty())
    addrSpaceList->RemoveFront();
  delete addrSpaceList;
  delete memoryMap;
  delete pageTableSetupLock;
  delete tlbLock;
}


//----------------------------------------------------------------------
// MemoryManager::SetupAddressSpace
// 
// 1. Checks to see if we have enough memory for the process.
// 2. Sets up page table for the address space
//----------------------------------------------------------------------

  
bool MemoryManager::SetupAddressSpace(AddrSpace *space, unsigned int sizePages)
{
#ifdef PP3_1
  PTEntry *ptEntry;
#endif PP3_1

  // Start critical section
  pageTableSetupLock->Acquire();

  // check and see if we have enough space for this address space
#ifdef PP3
  curTLBEntry=0;
  if (sizePages > (unsigned) swapMan->swapMap->NumClear())
    return FALSE;
  // adjust size to allocate code and data first 
  sizePages -= space->stackPages;
#else
  if (sizePages > memoryMap->NumClear())
    return FALSE;
#endif // PP3
  

  // Set up page table for new process
  for (unsigned int i = 0; i < sizePages; i++) {
#ifdef PP3
    // make a new pageTable Entry
    ptEntry= new PTEntry;

    ptEntry->virtualPage = i;
    ptEntry->physicalPage = swapMan->swapMap->FindAndSet();
    ptEntry->valid = TRUE;
    ptEntry->use = FALSE;
    ptEntry->dirty = FALSE;
    ptEntry->readOnly = FALSE;

    space->pageTable->Insert(ptEntry);
#else
    space->pageTable[i].virtualPage = i;
    space->pageTable[i].physicalPage = memoryMap->FindAndSet();
    space->pageTable[i].valid = TRUE;
    space->pageTable[i].use = FALSE;
    space->pageTable[i].dirty = FALSE;
    space->pageTable[i].readOnly = FALSE;
#endif PP3
  }

#ifdef PP3
  // set stack up outer space of VM
  for (unsigned int i = NumVMPages - space->stackPages; i < NumVMPages; i++) {
    // make a new pageTable Entry
    ptEntry= new PTEntry;

    ptEntry->virtualPage = i ;
    ptEntry->physicalPage = swapMan->swapMap->FindAndSet();
    ptEntry->valid = TRUE;
    ptEntry->use = FALSE;
    ptEntry->dirty = FALSE;
    ptEntry->readOnly = FALSE;

    space->pageTable->Insert(ptEntry);
}

#endif PP3  
  

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
#ifdef PP3

  HashIterator<int, PTEntry *> iter(space->pageTable);
  IPTEntry *iptEntry = new IPTEntry;
  PTEntry *ptEntry;
  IPTEntry *iptToRemove;
  // iterate through process' swap page table and deallocate
  // all its pages and deallocate those pages in memory
  for(;!iter.IsDone(); iter.Next()) {
    ptEntry=iter.Item();
    
    // clear  swap bitmap
    swapMan->swapMap->Clear(ptEntry->physicalPage);

    // temporary ipt Entry to lookup and remove entry
    iptEntry->virtualPage=ptEntry->virtualPage;
    iptEntry->pid=kernel->currentThread->space->ASID();

    //
    
    // clear entry from physmem  bitmap
    if (ipt->IsInTable(iptHashKey(iptEntry))){
      iptToRemove=ipt->Remove(iptHashKey(iptEntry));
      memoryMap->Clear(iptToRemove->physicalPage);
    }
    
  }
#else
  // Set up page table for new process
  for (unsigned int i = 0; i < NumPhysPages; i++) {
    if (space->pageTable[i].valid == TRUE) {
      memoryMap->Clear(space->pageTable[i].physicalPage);
      space->pageTable[i].valid = FALSE;
    }
  }
#endif // PP3  
  addrSpaceList->Remove(space);

  // End critical section
  pageTableSetupLock->Release(); 
}

//----------------------------------------------------------------------
// MemoryManager::UserToKernel
//   Copy bytesToCopy from userVA to kernelBuf.  If an invalid translation
// occurs then it bails.  It returns the number of bytes copied over.
//----------------------------------------------------------------------

int MemoryManager::UserToKernel(char *kernelBuf, unsigned long userVA, int bytesToCopy) {
  int userPA;
  // Copy one byte at a time
  for (int curByte = 0; curByte < bytesToCopy; curByte++) {
    // tranlate userVA to userPA
#ifdef PP3
    userPA = GetAndPlaceVA(userVA + curByte, kernel->currentThread->space);
#else
    userPA = va2pa(userVA + curByte, kernel->currentThread->space);
#endif // PP3
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

int MemoryManager::KernelToUser(char *kernelBuf, unsigned long userVA, int bytesToCopy) {
  int userPA;
  // Copy one byte at a time
  if (bytesToCopy>=0){
    for (int curByte = 0; curByte < bytesToCopy; curByte++) {
      // tranlate userVA to userPA
#ifdef PP3
      userPA = GetAndPlaceVA((userVA + curByte), kernel->currentThread->space);
#else
      userPA = va2pa(userVA + curByte, kernel->currentThread->space);
#endif // PP3
      if (userPA == -1) return curByte;  // Bailout on invalid translation
      // copy byte from buffer
      kernel->machine->mainMemory[userPA]=kernelBuf[curByte];
    }
  }
  else {
    for (int curByte = 0; curByte < -1*bytesToCopy; curByte++) {
      // tranlate userVA to userPA
#ifdef PP3
      userPA = GetAndPlaceVA(userVA - curByte, kernel->currentThread->space);
#else
      userPA = va2pa(userVA - curByte, kernel->currentThread->space);
#endif // PP3
      if (userPA == -1) return curByte;  // Bailout on invalid translation
      // copy byte from buffer
      kernel->machine->mainMemory[userPA]=kernelBuf[curByte];
    }
  }
  return bytesToCopy;  // indicate all bytes copied
}

#ifndef PP3
//----------------------------------------------------------------------
// MemoryManager::va2pa
//  Performs a virtual address to physical address translation.  Returns
// the physical address or a -1 if the address cannot be tranlated.
//----------------------------------------------------------------------

int MemoryManager::va2pa(int va, AddrSpace *space) {
  // Is this a valid translation
  if (space->pageTable[va/PageSize].valid == FALSE)
    return -1;
  else // do translation
    return (int) (space->pageTable[va/PageSize].physicalPage *
		  PageSize + (va % PageSize));
}
#endif // PP3


#endif // PP2_2

#ifdef PP3_1
//----------------------------------------------------------------------
// iptHashKey
//  Generates a key for the ipt hash table. Sum pid, vpn and ppn
//----------------------------------------------------------------------
int iptHashKey(IPTEntry *iptEntry) {
  return (iptEntry->pid * NumPhysPages) + iptEntry->virtualPage;
}

//----------------------------------------------------------------------
// iptHashFunc
//  just return key as the hash
//----------------------------------------------------------------------
unsigned int iptHashFunc(int key){
  return (unsigned int) key;
}

//----------------------------------------------------------------------
// locate
//  returns a bool telling if the PPN was found
//  if found the PPN is passed by argument.
//----------------------------------------------------------------------
bool MemoryManager::Locate (IPTEntry **iptEntry)
{   
  if(ipt->Find(iptHashKey(*iptEntry), iptEntry))  
      return TRUE;
  else
      return FALSE;
}


//----------------------------------------------------------------------
// TLBUpdate
//  performs second chance clock algorithm to decide which TLB entry to update
//----------------------------------------------------------------------
void MemoryManager::TLBUpdate(IPTEntry *iptEntry)
{

  Interrupt *interrupt = kernel->interrupt;
  //printf("[TLB update for thread: %s]\n", kernel->currentThread->getName());
  IPTEntry *tlbEntry = new IPTEntry;
  IPTEntry *foundEntry;

  //tlbLock->Acquire();
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  // make sure victims flags are sent back to its ipt entry
  tlbEntry->pid=kernel->currentThread->space->ASID();
  if (kernel->machine->tlb[curTLBEntry].valid) { // For valid tlb entries
    // see if that tlb entry is in the ipt
    tlbEntry->virtualPage = kernel->machine->tlb[curTLBEntry].virtualPage;
    kernel->memoryManager->ipt->Find(iptHashKey(tlbEntry), &foundEntry);
    // if that entry is in ipt then up date that entry in ipt
    if (foundEntry != NULL) {
      foundEntry->use   = kernel->machine->tlb[curTLBEntry].use;
      foundEntry->dirty = kernel->machine->tlb[curTLBEntry].dirty;
    }
  }
  // Invalidate any entr that point to the new entry'sppn
  for(int curEntry=0; curEntry < TLBSize; curEntry++) {
    if (kernel->machine->tlb[curEntry].physicalPage == 
	iptEntry->physicalPage)
	  kernel->machine->tlb[curEntry].valid = FALSE;
  }  

  // Put new tlb entry
  kernel->machine->tlb[curTLBEntry].virtualPage  = iptEntry->virtualPage;
  kernel->machine->tlb[curTLBEntry].physicalPage = iptEntry->physicalPage;
  kernel->machine->tlb[curTLBEntry].valid        = TRUE;
  kernel->machine->tlb[curTLBEntry].readOnly     = iptEntry->readOnly;
  kernel->machine->tlb[curTLBEntry].use          = FALSE;
  kernel->machine->tlb[curTLBEntry].dirty        = iptEntry->dirty;

  curTLBEntry = (curTLBEntry+1) % TLBSize;
  (void) interrupt->SetLevel(oldLevel);
  //  tlbLock->Release();
  delete tlbEntry;
  // printf("[Done TLBUpdate for thread: %s]\n", kernel->currentThread->getName());
} 

//----------------------------------------------------------------------
// IPTUpdate;
//  Goes throught the tlb and updates all the ipt entries that correspond to
// tlb entries so that their use and dirty bits are set.
//----------------------------------------------------------------------
void MemoryManager::IPTUpdate(void) {
  IPTEntry *tlbEntry = new IPTEntry;
  IPTEntry *foundEntry = NULL;
  int key = 0;
  //printf("[IPT update for thread: %s]\n", kernel->currentThread->getName());

  // make an ipt entry out of a tlb entry
  tlbEntry->pid=kernel->currentThread->space->ASID();

  for(int curEntry=0; curEntry < TLBSize; curEntry++) { // loop through tlb
    if (kernel->machine->tlb[curEntry].valid) { // For valid tlb entries
      // see if that tlb entry is in the ipt
      tlbEntry->virtualPage = kernel->machine->tlb[curEntry].virtualPage;
      key=iptHashKey(tlbEntry);
      kernel->memoryManager->ipt->Find(key, &foundEntry);
      // if that entry is in ipt then up date that entry in ipt
      if (foundEntry != NULL) {
	foundEntry->use = kernel->machine->tlb[curEntry].use;
	foundEntry->dirty = kernel->machine->tlb[curEntry].dirty;
      }
      foundEntry=NULL;
    }
  }
  delete tlbEntry;
  //printf("[Done IPTUpdate for thread: %s]\n", kernel->currentThread->getName()); 
}


//----------------------------------------------------------------------
// TLBFlush;
//  clears TLB
//----------------------------------------------------------------------
void MemoryManager::TLBFlush(void)  
{
  Interrupt *interrupt = kernel->interrupt;
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  //printf("[TLB flushing for thread: %s]\n", kernel->currentThread->getName()); 
  for(int curEntry=0; curEntry < TLBSize; curEntry++)
    kernel->machine->tlb[curEntry].valid = FALSE;
  (void) interrupt->SetLevel(oldLevel);
  //printf("[Done TLB flushing for thread: %s]\n", kernel->currentThread->getName()); 
}

//----------------------------------------------------------------------
// GetAndPlaceVA(int va, AddrSpace *space);
//  This function finds out where a va is.  If is in physical memory
// return the pa. If is not in physical memory, bring in the page to
// physcial memory and then return the page's pa.
//----------------------------------------------------------------------
int MemoryManager::GetAndPlaceVA(unsigned long va, AddrSpace *space) {
  int key;
  IPTEntry *iptEntry = new IPTEntry;
  IPTEntry *foundEntry;
  
  iptEntry->virtualPage = (unsigned) va / PageSize;
  iptEntry->pid = space->ASID();

  key=iptHashKey(iptEntry);
  //  delete iptEntry;
  
  // is the page in memory? 
  if (ipt->Find(key, &foundEntry) )
    {
      return (foundEntry->physicalPage) * PageSize + (va % PageSize); // yes! return pa
    }
  else {
    kernel->stats->numPageFaults++;
    PTEntry *ptEntry = kernel->currentThread->space->GetPTEntry(iptEntry->virtualPage);
    // if this page is valid then load into physical memory
    if (ptEntry  != NULL)
      {
	kernel->memoryManager->IPTUpdate(); // sync ipt entries with tlb 
	// setup entry
	iptEntry->physicalPage = PageIn(ptEntry); // page in the swap page
	iptEntry->valid = TRUE;
	iptEntry->readOnly = ptEntry->readOnly;
	iptEntry->use = TRUE;
	iptEntry->dirty = TRUE;
	// pageTableSetupLock->Acquire();
	ipt->Insert(iptEntry); // place in ipt
	// pageTableSetupLock->Release();
	TLBUpdate(iptEntry);
	return iptEntry->physicalPage * PageSize + (va % PageSize);

      }  
    else {
      // This might be a stack growth
      int lastStackPage=NumVMPages -
	kernel->currentThread->space->stackPages;
      // printf("GAP:VP: %d lastStack: %d\n", iptEntry->virtualPage ,lastStackPage);
      if ((lastStackPage - iptEntry->virtualPage) == 1) {
	//printf("Making more stack\n");
	ptEntry = kernel->memoryManager->VMAllocate(iptEntry->virtualPage);
	if (ptEntry != NULL) // do we even have an entry
	  {
	    kernel->currentThread->space->stackPages++;
	    kernel->memoryManager->IPTUpdate(); // sync tlb with ipt
	    
	    // move the page from swap to phys mem.
	    iptEntry->physicalPage = kernel->memoryManager->PageIn(ptEntry); 
	    
	    // make an entry for the new page and stick it into ipt and tlb
	    iptEntry->valid = TRUE;
	    iptEntry->readOnly = ptEntry->readOnly;
	    iptEntry->use = ptEntry->use;
	    iptEntry->dirty = ptEntry->dirty;

	    pageTableSetupLock->Acquire();
	    ipt->Insert(iptEntry);
	    pageTableSetupLock->Release();
	    TLBUpdate(iptEntry);
	    return iptEntry->physicalPage * PageSize + (va % PageSize);
	  } 
      }
      else  { // this page does not exist
	printf("\nGAP: Process [%s] has segmenation faulted trying to get VA=%u\n", 
	       (int) kernel->currentThread->getName(), va);
	// Set the exit code for join to use
	kernel->currentThread->SetExitCode(-1);
	// deallocate threads address space
	kernel->memoryManager->RemoveAddressSpace(kernel->currentThread->space);
	kernel->currentThread->Finish(); // Tell thread to finish	
	return -1;
      }
    }
  } 
  // return -1;
}

//----------------------------------------------------------------------
// PageIn
//  This function takes a page table entry and pages in the page for that
// entry from the swap file into the main memory.  If main memory is full
// it will use the LRU algorithm to find a victim, write out dirty
// victims, and write in the new pages.  The physical memory page that the
// page is swapped into is return or a -1 if an error occurs.
//----------------------------------------------------------------------
int MemoryManager::PageIn(PTEntry *ptEntry) {
  int ppn = -1;  // physical mem page # that we will use
  IPTEntry *foundEntry;
  PTEntry *fptEntry;
   // try to get a free spot
  if ((ppn = memoryMap->FindAndSet()) != -1) {
    swapMan->SwapToMem(ptEntry->physicalPage, ppn); // move page from swap
    // to memory
    return ppn;
  }
  else { // no space, look for victim using LRU "second chance"
    // Start critical section
    pageTableSetupLock->Acquire();
    // Perform second chance algo.
#ifdef RANDOM
    // printf("Doing RANDOM on IPT.\n");
    unsigned victim, numclear, randNum;
    randNum=RandomNumber();
    //    numused=NumPhysPages - memoryMap->NumClear();
    victim = randNum % NumPhysPages;
    HashIterator<int, IPTEntry *> iter(ipt); 
    
    do
      {
	foundEntry = iter.Item();
	ppn = foundEntry->physicalPage;
	iter.Next();
      } while (victim--);
  

#endif
#ifdef LRU
    //printf("Doing LRU on IPT.\n");
    while (ppn == -1) { // need 2 loops because iterator only iterates once
      HashIterator<int, IPTEntry *> iter(ipt); 
      
      // keep iterating until we find a page to use
      while((!iter.IsDone()) && (ppn == -1)) {
	foundEntry = iter.Item();
	if (!foundEntry->use)
	  ppn = foundEntry->physicalPage;
	else
	  foundEntry->use = FALSE;
	iter.Next();
      }
    }
#endif
    
    // Write out dirty files
    if (foundEntry->dirty) {
      // where does the dirty entry go in swap?
      ListIterator<AddrSpace *> iter(addrSpaceList);
      AddrSpace *foundSpace;
      bool found = FALSE;
      while ((!iter.IsDone()) && (!found)) {
	foundSpace=iter.Item();
	if (foundSpace->ASID() == (unsigned) foundEntry->pid) found=TRUE;
	iter.Next();
      }
      fptEntry = foundSpace->GetPTEntry(foundEntry->virtualPage);


      Interrupt *interrupt = kernel->interrupt;
      IntStatus oldLevel = interrupt->SetLevel(IntOff);  
      for(int curEntry=0; curEntry < TLBSize; curEntry++) {
	if (kernel->machine->tlb[curEntry].physicalPage == 
	    foundEntry->physicalPage) {
	  kernel->machine->tlb[curEntry].valid = FALSE;
	}
      }    
      (void) interrupt->SetLevel(oldLevel);

      // put the dirty entry back into swap file
      swapMan->CopyMemToSwap(ppn, fptEntry->physicalPage);
    }

    ipt->Remove(iptHashKey(foundEntry)); // pull the old page out of the ipt
    
    // Move new page into phys mem.
    swapMan->SwapToMem(ptEntry->physicalPage, ppn); 
    // Start critical section
    pageTableSetupLock->Release();
    return ppn;
  }

}

//--------------------------------------------------------------------------
// MemoryManager::VMAllocate
//  Given a virtual page number, it will allocate that page in the current
// address space.  if it is not
// already allocated.  It returns NULL if the pages is alredy allocated or
// the vpn is not in the VA space.  Otherwise it returns a ptr to the page
// table entry.  Note: this does not allcate pages in physical memory.  It
// allocates pages in the swap space. So if the swap space is full then a
// NULL is returned
//--------------------------------------------------------------------------
PTEntry *MemoryManager::VMAllocate(int vpn) {

  PTEntry *ptEntry;
  HashTable<int, PTEntry *> *pageTable =
    kernel->currentThread->space->pageTable;

  // See if we have space in the swap to do this. if not return NULL
  if (swapMan->swapMap->NumClear()==0) 
    return NULL;
 
  ptEntry = new PTEntry;
  ptEntry->virtualPage = vpn;

  // Search the page table for this vpn. If the search is successfull then
  // the page has been allocated previously and return NULL
  if (pageTable->IsInTable(vpn))
    return NULL;

  // At this point we can actual claim the page and return a ptr to it.
  ptEntry->physicalPage = swapMan->swapMap->FindAndSet();
  ptEntry->valid = TRUE;
  ptEntry->use = FALSE;
  ptEntry->dirty = FALSE;
  ptEntry->readOnly = FALSE;
  
  pageTable->Insert(ptEntry);

  return ptEntry;
}

//----------------------------------------------------------------------
// IPTClearUse
//  clears the used bit in ipt. helps in keeping use uptodate
//----------------------------------------------------------------------
void MemoryManager::IPTClearUse(void) {
  IPTEntry *foundEntry;

  pageTableSetupLock->Acquire();

  HashIterator<int, IPTEntry *> iter(ipt); 
      
  while(!iter.IsDone()) {
    foundEntry = iter.Item();
    foundEntry->use = FALSE;
    iter.Next();
  }

  pageTableSetupLock->Release();
}

#endif // PP3_1

#ifdef PP4
//----------------------------------------------------------------------
// CreateSwap
//  create the swap, after the file system is created.
//----------------------------------------------------------------------
void MemoryManager::CreateSwap(){
  swapMan = new SwapManager;
}
#endif
