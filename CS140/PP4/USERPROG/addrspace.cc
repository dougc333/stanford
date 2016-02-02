// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with ethe -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "machine.h"
#include "noff.h"
#include "constants.h"

#ifdef PP2_2
#include "memman.h"
#endif // PP2_2


extern "C" void bzero(void *, unsigned int);
extern "C" void bcopy(const void *, void *, unsigned int);

unsigned int AddrSpace::nextAsid = 1;

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

#ifndef SIMOS
static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}
#endif

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
#ifdef PP3_1
  pageTable = new HashTable<int, PTEntry *>(PTHashKey, PTHashFunc);
  segmentIdTable = new HashTable<int, MySegment *>(SegmentHashKey, SegmentHashFunc);
  segmentIdCounter = 0;
  segmentIdCounterLock = new Lock("segmentIdCounterLock");
#else
    pageTable = new TranslationEntry[NumPhysPages];
    for (int i = 0; i < NumPhysPages; i++) {
        pageTable[i].valid = FALSE;
    }
#endif

    asid = nextAsid++;
#ifdef PP2_2
    // DO NOT zero out the entire address space
#else 
    // zero out the entire address space 
    bzero(kernel->machine->mainMemory, MemorySize);
#endif // PP2_2
#ifdef PP2_3
    stackOffset = 0; // Initially no arguements are on the stack
#endif // PP2_3
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifdef PP3_1
  delete segmentIdTable;
  delete pageTable;
  delete segmentIdCounterLock;
#else
   delete [] pageTable;
#endif // PP3_1
}


//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

bool 
AddrSpace::Load(char *fileName) 
{
#ifdef PP2_3
  strcpy(comLine, fileName);  // copy the fileName into the command line
  // buffer so we can manipulate it safely

  // Parse the command line for file name
  argc = 0;


  // Clean out the argv buffer
  for (int i=0; i < CMDLINEMAX; i++)
    argv[i]=NULL;

  // Look for the file name. Bail if not found
#ifdef PP4
  char *comLineBuf = new char[60];
  strcpy(comLineBuf, fileName);
  if ((argv[argc]=strtok(comLineBuf," "))==NULL) return FALSE;
#else  
  if ((argv[argc]=strtok(comLine," "))==NULL) return FALSE;
#endif
  argc++;
#ifdef PP4
  char * filename = new char[60];
  OpenFile *MyOpenFile = NULL;
  strcpy(filename, argv[0]);
  char *executableName;
  executableName = kernel->fileSystem->ParseFile(filename, &MyOpenFile);
  if (executableName == NULL) {
    return FALSE;
  }
  OpenFile *executable = kernel->fileSystem->OpenInDir(executableName,
						       MyOpenFile);
  //printf(argv[1]);
#else
  OpenFile *executable = kernel->fileSystem->Open(argv[0]);
#endif // PP4
#else
  OpenFile *executable = kernel->fileSystem->Open(fileName);
#endif // PP2_3

    NoffHeader noffH;
    unsigned int size;

    if (executable == NULL) {
        printf("Unable to open file %s\n", fileName);
	return FALSE;
    }

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
#ifndef SIMOS
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
#endif
    ASSERT(noffH.noffMagic == NOFFMAGIC);


#ifdef PP3
    stackPages      = divRoundUp(UserStackSize, PageSize); 
#else
    unsigned int stackPages      = divRoundUp(UserStackSize, PageSize);
#endif // PP3
    // how big is address space?


    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
      + (stackPages* PageSize);  // Use the rounded up version of stackPages to ensure
    // that the stack pages do not mingle with other code or data pages.

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    unsigned int codePages       = divRoundUp(noffH.code.size, PageSize);
    unsigned int initDataPages   = divRoundUp(noffH.initData.size, PageSize);
    unsigned int uninitDataPages = divRoundUp(noffH.uninitData.size, PageSize);

    
    unsigned int startCodePg   = noffH.code.virtualAddr/PageSize;
    unsigned int startIDataPg  = noffH.initData.virtualAddr/PageSize;
    unsigned int startUIDataPg = noffH.uninitData.virtualAddr/PageSize;
#ifdef PP3
    unsigned int startStackPg  = NumVMPages - stackPages;
#else
    unsigned int startStackPg  = numPages - stackPages;
#endif // PP3

#ifdef PP2_2
#ifdef PP3
    if (numPages > NumVMPages)
      printf("Not enough memory to load process\n");
#else
    if (numPages > NumPhysPages)
      printf("Not enough memory to load process\n");
#endif // PP3
#else
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
#endif // PP2_2
    DEBUG(dbgAddr,("Initializing address space: %d, %d\n", numPages, size));
#ifdef PP2_2
    // Setup page table for new process
    if (kernel->memoryManager->SetupAddressSpace(this, numPages)==FALSE)
    	return FALSE;
#else
    // Set up page table for new process
    for (unsigned int i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virt page # = phys page #
	pageTable[i].physicalPage = i;
	pageTable[i].valid = TRUE;
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  
    }
#endif // PP2_2

// then, copy in the code and data segments into memory

#ifdef PP2_2
    LoadSegment(&noffH.code, executable); // Set up code segment
    LoadSegment(&noffH.initData, executable); // Set up data segment

    // set up read permissions for each segment;
    SetReadOnly(startCodePg, codePages, TRUE);
    // note at most one code page may be set as readable
    SetReadOnly(startIDataPg, initDataPages, FALSE);
    SetReadOnly(startUIDataPg, uninitDataPages, FALSE);
    SetReadOnly(startStackPg, stackPages, FALSE);
#else
    if (noffH.code.size > 0) {
        DEBUG(dbgAddr,("Initializing code segment."));
        DEBUG(dbgAddr,("%d, %d\n", noffH.code.virtualAddr, noffH.code.size));

        int addr = (pageTable[noffH.code.virtualAddr/PageSize].physicalPage *
                    PageSize + (noffH.code.virtualAddr % PageSize));

        executable->ReadAt(&(kernel->machine->mainMemory[addr]), 
                           noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG(dbgAddr,("Initializing data segment."));
	DEBUG(dbgAddr,("%d, %d\n", noffH.initData.virtualAddr, noffH.initData.size));
        int addr = (PageSize * pageTable[noffH.initData.virtualAddr/PageSize].physicalPage + noffH.initData.virtualAddr % PageSize);

        executable->ReadAt(&(kernel->machine->mainMemory[addr]),
                           noffH.initData.size, noffH.initData.inFileAddr);
    }

#endif // PP2_2

    delete executable;			// close file
    return TRUE;			// success
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program.  Load the executable into memory, then
//	(for now) use our own thread to run it.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

void 
AddrSpace::Execute(char *fileName) 
{
    kernel->currentThread->space = this;

    if (!Load(fileName)) {
        kernel->currentThread->space = NULL;
	return;				// executable not found
    }

    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register

    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
    					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
#ifdef PP2_3
    machine->WriteRegister(StackReg, stackOffset);
    DEBUG(dbgAddr,("Initializing stack pointer: %d\n", stackOffset));

    // place argc in reg4 and argv in reg5
    machine->WriteRegister(4, argc);
    machine->WriteRegister(5, argv0Addr);

#else
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr,("Initializing stack pointer: %d\n", numPages*PageSize -
		   16));
#endif // PP2_3
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
#ifdef PP3_1
  kernel->memoryManager->IPTUpdate();
#endif // PP3_1
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{

#ifndef VM
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
#endif
#ifdef SIMOS
    UpdateTLB(asid);
#endif
}

#ifdef PP2_2
//----------------------------------------------------------------------
// AddrSpace::LoadSegment
// 	Loads a segment in to the address space and takes care of
//  splitting the segment in to pages.
//----------------------------------------------------------------------
void AddrSpace::LoadSegment(Segment *seg, OpenFile *executable) {
#ifdef PP3  
  kernel->memoryManager->swapMan->FileToSwap(executable, seg, this);
#else
    if (seg->size > 0) {

      int startPage = seg->virtualAddr/PageSize;

      int numSegPages=divRoundUp((seg->virtualAddr)+(seg->size), PageSize)-
	divRoundUp(seg->virtualAddr, PageSize);

      if (numSegPages==0) numSegPages=divRoundUp(seg->size, PageSize);

      int addr;
	
      if (numSegPages > 1) {
	int numPagesLoaded = 0;
	int fileOffset = seg->inFileAddr;
	// load in the first page
	
	
	addr = (pageTable[startPage].physicalPage *
		    PageSize + (seg->virtualAddr % PageSize));
	executable->ReadAt(&(kernel->machine->mainMemory[addr]), 
			   PageSize-(seg->virtualAddr % PageSize), 
			   fileOffset);

	fileOffset+=PageSize-(seg->virtualAddr % PageSize);
	numPagesLoaded++;

	// load subsequent blocks
	while (numPagesLoaded<numSegPages-1) {
	  addr = pageTable[startPage + numPagesLoaded].physicalPage *
	    PageSize;

	  executable->ReadAt(&(kernel->machine->mainMemory[addr]), 
			     PageSize, fileOffset);

	  fileOffset+=PageSize;
	  numPagesLoaded++;
	}

	// load the last page
	addr = pageTable[startPage + numPagesLoaded].physicalPage *
	    PageSize;
	executable->ReadAt(&(kernel->machine->mainMemory[addr]), 
			   ((seg->virtualAddr + seg->size) % 
			   PageSize), 
			   fileOffset);
      }
      else { // code resides in only one block
	addr = (pageTable[startPage].physicalPage *
		PageSize + (seg->virtualAddr % PageSize));
	executable->ReadAt(&(kernel->machine->mainMemory[addr]), 
			   ((seg->virtualAddr + seg->size) % PageSize) - 
			   (seg->virtualAddr % PageSize), 
			   seg->inFileAddr);
      }
    }
#endif // PP3

} 

//----------------------------------------------------------------------
// AddrSpace::GetReady
// 	Get a user program read to run a user program.  
//       Load the executable into memory
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

bool 
AddrSpace::GetReady(char *fileName) 
{
  if (!Load(fileName)) {
    return FALSE;				// executable not found
  }
  else return TRUE;
}

//----------------------------------------------------------------------
// AddrSpace::SetReadOnly
//   Sets a page to read or write
//----------------------------------------------------------------------
void AddrSpace::SetReadOnly(unsigned startPage, unsigned int pages, bool readOnly)
{
#ifdef PP3_1
  PTEntry *ptEntry;
#endif // PP3_1
  // Set up page table for new process
  for (unsigned int i = startPage; i < startPage+pages; i++) {
#ifdef PP3_1
    ptEntry=GetPTEntry(i);
    ptEntry->readOnly = readOnly;
#else
    pageTable[i].readOnly = readOnly;
#endif // PP3_1
  }
}


#endif // PP2_2
#ifdef PP2_3
//----------------------------------------------------------------------
// AddrSpace::PassArgs
// 	sets up the stack with command line arguments
//----------------------------------------------------------------------
void AddrSpace::PassArgs(void)
{
#ifdef PP4
  char *comLineBuf = new char[60];
  strcpy(comLineBuf, comLine);
  //  if ((argv[argc]=strtok(comLineBuf," "))==NULL) return FALSE;
  strtok(comLineBuf," ");
#endif
  //  printf ("Making an initial stack for %s\n", kernel->currentThread->getName());
  // stackOffset = NumPhysPages * PageSize - 16;
#ifdef PP3
  stackOffset = 0x7ffffff0;// Setup stack pointer 
#else
  stackOffset = numPages * PageSize - 16; // Setup stack pointer    
#endif
  // get the rest of the command line arguments
  while ((argv[argc]=strtok(NULL," "))!=NULL) {
    argc++;
  }
  
  unsigned int argvAddr[CMDLINEMAX];  // Holds the address of the arguments on the stack
  int stackItem;   // Temporary argument for correct byte ordering
  
  // push the arguments on the stack in reverse order
  for (int arg=0; arg < argc; arg++) {
    // move to next position
    stackOffset-=(1+strlen(argv[argc-1-arg]));
    // record address of item on stack
    argvAddr[argc-1-arg]=stackOffset+1;

    kernel->memoryManager->KernelToUser(argv[argc-1-arg], stackOffset+1,
					(1+strlen(argv[argc-1-arg])));
  }
  
  if (stackOffset%4) { // stack not word aligned
    stackOffset-=(stackOffset%4); // align stack ptr
  }
  
  // push the argument address on the stack
  for (int arga=0; arga < argc; arga++) {
    stackOffset-=4;
    stackItem=WordToMachine(argvAddr[argc-1-arga]);
      kernel->memoryManager->KernelToUser((char *) &stackItem, stackOffset, 4);
  }

  argv0Addr= stackOffset;

  // push argv on stack
  stackItem=WordToMachine(argv0Addr);
  stackOffset-=4;
  kernel->memoryManager->KernelToUser((char *) &stackItem, stackOffset, 4);

  
  // push argc on stack
  stackItem=WordToMachine(argc);
  stackOffset-=4;
  kernel->memoryManager->KernelToUser((char *) &stackItem, stackOffset, 4);

  stackOffset-=4;
  // printf ("Finished making an initial stack for %s\n", kernel->currentThread->getName());
}



#endif // PP2_3

#ifdef PP3_1
//----------------------------------------------------------------------
// PTHashKey
//   Generates a key for the pagetable hash
//----------------------------------------------------------------------
int  PTHashKey(PTEntry *ptEntry) {
  return ptEntry->virtualPage;
}

//----------------------------------------------------------------------
// PTHashFunc
//   Hash function for pagae table hash
//----------------------------------------------------------------------
unsigned PTHashFunc(int key) {
  return key;
}
//----------------------------------------------------------------------
// AddrSpace::vpn2ppn
// 	gets the physical page number corresponding to virtual page num
//      returns a -1 if no entry found
//----------------------------------------------------------------------
int AddrSpace::vpn2ppn(int vpn) {
  PTEntry *ptEntry = new PTEntry;
  int key = 0;
  ptEntry->virtualPage=vpn;
  key=PTHashKey(ptEntry);
  // delete ptEntry;
  if(pageTable->Find(key, &ptEntry))
    return ptEntry->physicalPage;
  else
    return -1;
}

//----------------------------------------------------------------------
// AddrSpace::GetPTEntry
// 	gets the page table entry for specific virtual page
//      returns a NULL if no entry found
//----------------------------------------------------------------------
PTEntry *AddrSpace::GetPTEntry(int vpn) {
  PTEntry *ptEntry = new PTEntry;
  int key = 0;
  ptEntry->virtualPage=vpn;
  key=PTHashKey(ptEntry);
  // delete ptEntry;
  if(pageTable->Find(key, &ptEntry))
    return ptEntry;
  else
    return NULL;
}

#endif //PP3_1

#ifdef PP3_1
//----------------------------------------------------------------------
// SegmentHashKey
//  Generates a key for the segment hash table.
//----------------------------------------------------------------------
int SegmentHashKey(MySegment *segment) {
  return (segment->SegmentID);
}

//----------------------------------------------------------------------
// SegmentHashFunc
//  just return key as the hash
//----------------------------------------------------------------------
unsigned int SegmentHashFunc(int key) {
  return (unsigned int) key;
}

#endif
