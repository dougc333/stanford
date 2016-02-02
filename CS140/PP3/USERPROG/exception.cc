// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "constants.h"


#ifdef PP3
#include "memman.h"
#include "stdlib.h"
#endif //PP3



#ifdef PP2_1
#include "string.h"
#include "kernel.h"
#include "thread.h"
#include "machine.h"
#include "openfile.h"

void CallMachineRun(int junk);
void HandleExit(void);
void HandleJoin(void);
void HandleExec(void);
void HandleCreate(void);
void HandleOpen(void);
void HandleClose(void);
void HandleRead(void);
void HandleWrite(void);
void HandleUnexpected(void);
#endif //PP2_1


#ifdef PP3
void HandlePageFault(void);
void HandleLength(void);
void HandleMmap(void);
void HandleMunmap(void);
#endif // PP3

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
  int type = kernel->machine->ReadRegister(2);
  
  switch (which) {
  case SyscallException:
    switch(type) {
    case SC_Halt:
      DEBUG(dbgAddr, ("Shutdown, initiated by user program.\n"));
      kernel->interrupt->Halt();
      break;
#ifdef PP2_1
    case SC_Exit:
      HandleExit();
      break;
      
    case SC_Join:
      HandleJoin();
      break;
      
    case SC_Exec:
      HandleExec();
      break;
      
    case SC_Create:
      HandleCreate();
      break;
      
    case SC_Open:	  
      HandleOpen();
      break;
      
    case SC_Close:
      HandleClose();
      break;
      
    case SC_Read:
      HandleRead();
      break;
      
    case SC_Write:
      HandleWrite();
      break;
      
#endif //PP2_1

#ifdef PP3
    case SC_Length:
      HandleLength();
      break;

    case SC_Mmap:
      HandleMmap();
      break;

    case SC_Munmap:
      HandleMunmap();
      break;

#endif // PP3

    default:
      printf("Unexpected system call %d\n", type);
      kernel->machine->IncrementPC(); 
      break;
    }
#ifdef PP2_1
    kernel->machine->IncrementPC();
#endif //PP2_1
    break;

#ifdef PP3
  case PageFaultException:
    HandlePageFault();
    break;
#endif PP3

  default:
    printf("Unexpected user mode exception %d\n", which);
    HandleUnexpected();
    break;
    
  }
  
#ifdef PP2_1
  return;
#endif // PP2_1

  ASSERTNOTREACHED();
}


#ifdef PP2_1
//-----------------------------------------------------------------------
// CallMachineRun
//    Wrapper program to run program
//-----------------------------------------------------------------------

void CallMachineRun(int junk){
#ifdef PP2_3
  kernel->currentThread->space->PassArgs(); // setup stack with argument passing
#endif // PP2_3
  kernel->currentThread->space->InitRegisters();// set the initial register values
  kernel->currentThread->space->RestoreState();
  kernel->machine->Run();
}
//------------------------------------------------------------------------
// HandleUnexpected
//   Exception handler for SC_Exec
//------------------------------------------------------------------------
void HandleUnexpected(void) {
  DEBUG(dbgAddr, ("Unexpected user mode exception, initiated by user program.\n"));

  // Set the exit code for join to use
  kernel->currentThread->SetExitCode(-1);
  // deallocate threads address space
  kernel->memoryManager->RemoveAddressSpace(kernel->currentThread->space);
  kernel->currentThread->Finish(); // Tell thread to finish
}


//------------------------------------------------------------------------
// HandleExit
//   Exception handler for SC_Exec
//------------------------------------------------------------------------
void HandleExit(void) {
  DEBUG(dbgAddr, ("Exit, initiated by user program.\n"));

  // printf("Exiting thread [%s]\n", kernel->currentThread->getName());

  // Set the exit code for join to use
  kernel->currentThread->SetExitCode(kernel->machine->ReadRegister(4));
  // deallocate threads address space
  kernel->memoryManager->RemoveAddressSpace(kernel->currentThread->space);
  kernel->currentThread->Finish(); // Tell thread to finish
}

//------------------------------------------------------------------------
// HandleExec
//   Exception handler for SC_Exec
//------------------------------------------------------------------------
void HandleExec(void) {
  DEBUG(dbgAddr, ("Exec, initiated by user program.\n"));
  // printf("Exec, initiated by user program.\n");
  Thread *newThread;
  int cmdLinePtr; // ptr to command line in user space
  int threadID;   // return value for exec
  char *cmdLine; // buffer to hold command line
  
  cmdLine = new char[CMDLINEMAX];
  
  // get command line
  cmdLinePtr = kernel->machine->ReadRegister(4);

  // copy command line in ther kernel space
 int curChar =0;
  bool stopRead=FALSE;
  do {
    kernel->memoryManager->UserToKernel(cmdLine+curChar, cmdLinePtr+curChar, 1);
    if ((cmdLine[curChar]!='\0') && ((curChar+1)<60))
      curChar++;
    else stopRead=TRUE;
    
  } while (!stopRead);

  //kernel->memoryManager->UserToKernel(cmdLine, cmdLinePtr, 60);
  
  cmdLine[60]='\0'; // make sure commanline is terminated 

  // make a new thread
  newThread = new Thread(cmdLine);
  newThread->space = new AddrSpace();
  threadID = newThread->space->ASID();
  
  if (newThread->space->GetReady(cmdLine)) { // make sure file opens and loads
    newThread->Fork((VoidFunctionPtr) CallMachineRun,  (void *) threadID);
  }
  else {
    // don't run application on error
    delete newThread->space;
    delete newThread;
    threadID = -1;
  }
  
  // printf("[SC_Exec returning threadID: %s %d]\n", cmdLine, threadID);
  kernel->machine->WriteRegister(2,threadID);
  
}

//------------------------------------------------------------------------
// HandleJoin
//   Exception handler for SC_Exec
//------------------------------------------------------------------------
void HandleJoin(void) {
  DEBUG(dbgAddr, ("Join called\n"));
  //printf("Join called\n");
  unsigned int i;
  Thread* desiredChild;
  Thread* curChild;
  int spaceId;

  desiredChild=NULL;

  // get spaceId of child
  spaceId = kernel->machine->ReadRegister(4);
  //printf("SC_Join receiving spaceId: %d\n", spaceId);

  //printf("Trying to Joining Thread\n");

  // Look for child in the child list
  for (i = 0; i < kernel->currentThread->childList->NumInList(); i++)
    {
      curChild = kernel->currentThread->childList->RemoveFront();
      //printf("Join search %s, %d\n", curChild->getName(),curChild->space->ASID() );
      if ( curChild->space->ASID() ==(unsigned) spaceId)
	desiredChild = curChild;
      kernel->currentThread->childList->Append(curChild);
    }

  if (desiredChild!=NULL){
    //printf("Joining Thread [%s]\n", desiredChild->getName());
    desiredChild->Join();
    //printf("Joined Thread [%s] returned with %d \n",
    //    	   desiredChild->getName(), desiredChild->GetExitCode());
    kernel->machine->WriteRegister(2, desiredChild->GetExitCode());
  }
  else {
    kernel->machine->WriteRegister(2, -1); // return -1 when not joined
    //printf("Did not join\n");
  }
}


void HandleCreate(void) {

 //  for SC_Open, SC_Create, SC_Read, SC_Write
   
  char *filename;   
  int test;
  int pointerToOpenFile;
  char* MyString;
  filename = new char[60];
  
  pointerToOpenFile = kernel->machine->ReadRegister(4);
  MyString = (char *)pointerToOpenFile;
  int curChar =0;
  bool stopRead=FALSE;
  do {
    kernel->memoryManager->UserToKernel(filename+curChar, pointerToOpenFile+curChar, 1);
    if ((filename[curChar]!='\0') && ((curChar+1)<60))
      curChar++;
    else stopRead=TRUE;
    
  } while (!stopRead);
  //kernel->memoryManager->UserToKernel(filename, pointerToOpenFile, 60);
  
  
  // printf("Create filename: %s\n",filename);
  // printf("pointerToOpenFile: %d\n",pointerToOpenFile); 
  
  
  test = strlen(filename);
  //printf("test is: %d\n",test);
  
  if (test==0)
    {
      printf("Error Creating File: No file name.\n");
      return;
    } else {
      if (!kernel->fileSystem->Create(filename))
	printf("Error Creating File\n");
    }

}


void HandleOpen(void){
  char *filename;   
  OpenFile *filenamehandle;
  int test;
  int pointerToOpenFile;
  char* MyString;
  filename = new char[60];


  pointerToOpenFile = kernel->machine->ReadRegister(4);
  MyString = (char *) pointerToOpenFile;
  int curChar =0;
  bool stopRead=FALSE;
  do {
    kernel->memoryManager->UserToKernel(filename+curChar, pointerToOpenFile+curChar, 1);
    if ((filename[curChar]!='\0') && ((curChar+1)<60))
      curChar++;
    else stopRead=TRUE;
    
  } while (!stopRead);
    //  kernel->memoryManager->UserToKernel(filename, pointerToOpenFile, 60);
  // printf("Open filename: %s\n",filename);
  // printf("pointerToOpenFile: %d\n",pointerToOpenFile); 
	  
  test=strlen(filename);
  if (test == 0)
    {
      //printf("Invalid filename, file not opened\n");
      kernel->machine->WriteRegister(2, -1);
      return;
    }
  
  filenamehandle = kernel->fileSystem->Open(filename);
  //test for this file name NOT NULL
  if (filenamehandle == NULL) {
    kernel->machine->WriteRegister(2, -1);
    return;
  }
  // printf("adding file to list of open files in thread\n");
  // printf("status of filetable in thread is: %d",kernel->currentThread->openedFileList->NumInList);
  kernel->currentThread->openedFileList->Append(filenamehandle); 
  kernel->machine->WriteRegister(2, filenamehandle->file); 
}


void HandleClose(void){

  char *filename;   
  filename = new char[60];
  unsigned int i;
  OpenFile* OpenedFile;

  DEBUG(dbgAddr, ("Close, initiated by user program.\n"));
  DEBUG(dbgAddr, ("%d\n", kernel->machine->ReadRegister(4)));
	  
  if (kernel->machine->ReadRegister(4) == NULL)
    {
      printf("NULL fileid, file not opened\n");
      //do not return anything, no write to R(2)
      return;
    }
  
  if (kernel->machine->ReadRegister(4)<2)
    {
      //cannot close standard output or input
      printf("Error: Cannot close standard input/output device.\n");
      return;
    }
  else {
    OpenFileId FileToClose = OpenFileId(kernel->machine->ReadRegister(4));
    // printf("OpenFileID FileToClose: %d\n", FileToClose);
    for (i=0; i <  kernel->currentThread->openedFileList->NumInList(); i++)
      {
	OpenedFile = kernel->currentThread->openedFileList->RemoveFront();
	if (FileToClose != OpenedFile->file)
	  kernel->currentThread->openedFileList->Append(OpenedFile);
      }
  }
}


void HandleRead(void){

  unsigned int i;
  char *filename;   
  int virtualAddr;
  unsigned int size;
  OpenFileId MyFileID;
  char* MyBuffer;
  int writeopenFileId;
  filename = new char[60];
  OpenFile* OpenedFile;

  int bytesRead = 0;  
  
  virtualAddr = kernel->machine->ReadRegister(4);
  size        = kernel->machine->ReadRegister(5);
  writeopenFileId  = kernel->machine->ReadRegister(6);

  MyBuffer = new char[size];
  MyFileID = writeopenFileId;
  MyBuffer[0] = '\0';
  //test for the following error conditions, check to see buffer 
  // exists, and check to see size is greater than 1, and 
  // check to see there is a valid open file ID.
  
  //check string length of filename in argument1 and 
  //check to see the buffer exists

  if (MyFileID == 0)
    {
      kernel->consoleInputLock->Acquire();
      for ( i=0; i<size; i++)
	{
	  MyBuffer[i] = kernel->consoleInput->GetChar();
	}
      kernel->consoleInputLock->Release();
      // kernel->consoleInputLock->Release;
    }
  else
    {
      for (i=0; i <  kernel->currentThread->openedFileList->NumInList(); i++)
	{
	  OpenedFile = kernel->currentThread->openedFileList->RemoveFront();
	  if (MyFileID == OpenedFile->file)
	    bytesRead=OpenedFile->Read(MyBuffer,size);
	  kernel->currentThread->openedFileList->Append(OpenedFile);
	}
    }
  kernel->memoryManager->KernelToUser(MyBuffer, virtualAddr,size);
  //add byte count return//
  kernel->machine->WriteRegister(2,bytesRead);
}

void HandleWrite(void){
  
  unsigned int i;
  char *filename;   
  int virtualAddr;
  unsigned int size;
  OpenFileId MyFileID;
  char* MyBuffer;
  int writeopenFileId;
  OpenFile* OpenedFile;

  virtualAddr = kernel->machine->ReadRegister(4);
  size        = kernel->machine->ReadRegister(5);
  writeopenFileId  = kernel->machine->ReadRegister(6);

  filename = new char[60];
  MyBuffer = new char[size];

  MyFileID = writeopenFileId;
  kernel->memoryManager->UserToKernel(MyBuffer, virtualAddr, size);

  if (MyFileID == 1)
    {
      kernel->consoleOutputLock->Acquire();
      for ( i=0; i<size; i++)
	{
	  kernel->consoleOutput->PutChar(MyBuffer[i]);
	}
      kernel->consoleOutputLock->Release();
    }
  else
    {
      for (i=0; i <  kernel->currentThread->openedFileList->NumInList(); i++)
	{
	  OpenedFile = kernel->currentThread->openedFileList->RemoveFront();
	  if (MyFileID == OpenedFile->file)
	    OpenedFile->Write(MyBuffer, size);
	  kernel->currentThread->openedFileList->Append(OpenedFile);	  
	}
    }
}



#endif // PP2_1

#ifdef PP3
void HandlePageFault(void) {

  kernel->stats->numPageFaults++;
  //add local variables here to make it easy to port to a function call later, for now it is easier to debug and make changes in this format without having to scroll down every time.
  

  int virtualAddress;
  int key;
  PTEntry *ptEntry;
  IPTEntry *iptEntry = new IPTEntry;
  IPTEntry *foundEntry;
  //printf("entering page fault exception handler\n");
  
  virtualAddress = kernel->machine->ReadRegister(BadVAddrReg);

  // make and ipt entry so we can use it to search the ipt
  iptEntry->virtualPage = (unsigned) virtualAddress / PageSize;
  iptEntry->pid = kernel->currentThread->space->ASID();
  
  
  //TLB miss causes a lookup in the IPT. If found and valid bit is true,
  //load the TLB, replacing an existing entry using random replacement.  
  //If valid bit not true, go to the swap table, replace an entry in the 
  //IPT using the clock replacement and write the replaced entry back to
  //the swap table.

  // Setup lookup iptEntry

  key = iptHashKey(iptEntry);
  // does this ipt entry exist ?
  if (kernel->memoryManager->ipt->Find(key, &foundEntry) )
    {
      // It does! Well put it in the TLB
      //  kernel->memoryManager->IPTUpdate();
      kernel->memoryManager->TLBUpdate(foundEntry);
      
    }
  else { // Its not in ipt therefore not in phys mem, so page in that page
    // printf("VPN entry is not in the IPT\n");

    // get the pt entry of the process so we can find out where it lives
    // in the swap space
    ptEntry = kernel->currentThread->space->GetPTEntry(iptEntry->virtualPage);

    if (ptEntry != NULL) // do we even have an entry
      {
	kernel->memoryManager->IPTUpdate(); // sync tlb with ipt

	// move the page from swap to phys mem.
	iptEntry->physicalPage = kernel->memoryManager->PageIn(ptEntry); 

	// make an entry for the new page and stick it into ipt and tlb
	iptEntry->valid = TRUE;
	iptEntry->readOnly = ptEntry->readOnly;
	iptEntry->use = ptEntry->use;
	iptEntry->dirty = ptEntry->dirty;

	//kernel->memoryManager->pageTableSetupLock->Acquire();
	kernel->memoryManager->ipt->Insert(iptEntry);
	//kernel->memoryManager->pageTableSetupLock->Release();
	kernel->memoryManager->TLBUpdate(iptEntry);
      }    
    else {
      // This might be a stack growth
      int lastStackPage=NumVMPages -
	kernel->currentThread->space->stackPages;
      //printf("VP: %d lastStack: %d\n", iptEntry->virtualPage ,lastStackPage);
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
	    //kernel->memoryManager->pageTableSetupLock->Acquire();
	    kernel->memoryManager->ipt->Insert(iptEntry);
	    //kernel->memoryManager->pageTableSetupLock->Release();
	    kernel->memoryManager->TLBUpdate(iptEntry);
	  } 
      }
      else {
	// We have a seg fault or we need more stack.  Goodbye process
	printf("\nProcess [%s] has segmenation faulted trying to get VA=%d\n", kernel->currentThread->getName(), virtualAddress);
	//kernel->machine->DumpState();
	HandleUnexpected();
      }
    }
  }
}
  
void HandleLength(void)
{
  int bytesRead = 0;
  char MyBuffer[1];
  int size = 1;
  OpenFileId OpenFileID;
  OpenFile* OpenedFile;
  
  OpenFileID = (int)kernel->machine->ReadRegister(4);
  if (OpenFileID == 0)
    {
      printf("Error can't map console input.");
      bytesRead = -1;
      kernel->machine->WriteRegister(2,bytesRead);
      return;
      /*kernel->consoleInputLock->Acquire();
      for ( i=0; i<size; i++)
	{
	  MyBuffer[i] = kernel->consoleInput->GetChar();
	}
      kernel->consoleInputLock->Release();*/
      // kernel->consoleInputLock->Release;
    }
  else
    {
      for (int i=0; i <  kernel->currentThread->openedFileList->NumInList(); i++)
	{
	  OpenedFile = kernel->currentThread->openedFileList->RemoveFront();
	  if (OpenFileID == OpenedFile->file){
	    while (OpenedFile->ReadAt(MyBuffer,size,bytesRead) != 0){
	      //bytesRead=OpenedFile->Read(MyBuffer,size);
	      bytesRead++;
	    }
	    kernel->machine->WriteRegister(2,bytesRead);
	    kernel->currentThread->openedFileList->Append(OpenedFile);
	    return;
	  }
	  kernel->currentThread->openedFileList->Append(OpenedFile);
	}
    }

  /*while(bytesRead != EOF)
    {
      bytesRead += pointerToOpenFile->Read(MyBuffer,size); // to verify
    }*/
  bytesRead = 0;
  kernel->machine->WriteRegister(2,bytesRead);
  
}

void HandleMmap(void)
{
  int Length, i;
  int Address;

  OpenFileId OpenFileID;
  OpenFile* OpenedFile=NULL;
  
  OpenFileID = (int)kernel->machine->ReadRegister(4);
  Length = kernel->machine->ReadRegister(6);

  // get command line
  Address = (int) kernel->machine->ReadRegister(5);

  // copy command line in the kernel space
  //kernel->memoryManager->UserToKernel(Address, AddressPtr, CMDLINEMAX);
  //read teh filename out and store it in the segment object.

  if (OpenFileID < 0)
    {
      printf("Invalid OpenFileID, file not opened\n");
      kernel->machine->WriteRegister(2, -1);
      return;
    }
  //Getting the file handle, and storing the OpenFileID in the FileTable List per process, we want to return an OpenFile pointer for the swap copy.

  for (i=0; i <  kernel->currentThread->openedFileList->NumInList(); i++)
      {
	OpenedFile = kernel->currentThread->openedFileList->RemoveFront();
	kernel->currentThread->openedFileList->Append(OpenedFile);
	if (OpenFileID == OpenedFile->file){
	  //printf( "reached OpenFileID\n");
	  break;}
      }

  ASSERT(OpenFileID == OpenedFile->file); 
  //pageoffset = (Length % PageSize);

  if (Length > 0) {
    
      int startPage = ((Address))/PageSize;
      int numSegPages=divRoundUp(Length, PageSize);

      // Check if we have enough contiguous pages in the page table of the
      // process.
      for (int i = 0; i < numSegPages; i++) {
	// the function Find(startpage+i, &processPTEntry) == TRUE
	// there is a conflict
	if (kernel->currentThread->space->pageTable->IsInTable(startPage+i)) {
	  kernel->machine->WriteRegister(2, -1);
	  return;
	}
      }

      //printf("passed check for allocated vpn per process\n");
      //check process bitmap to see if we have enough pages for the allocation
      if (numSegPages > 
	  (unsigned)kernel->memoryManager->swapMan->swapMap->NumClear()) {
      	printf("Not enough pages in  the swap table to support this memory map\n");
      	kernel->machine->WriteRegister(2, -1);
	return;
      }

      // new seg - one MySegment object per process.
      MySegment *mySeg = new MySegment;
      mySeg->theSegment = new Segment;
      mySeg->theSegment->virtualAddr = Address;
      mySeg->theSegment->inFileAddr = 0;
      mySeg->theSegment->size = Length;
      mySeg->SegmentID = kernel->currentThread->space->segmentIdCounter;
      mySeg->PID = kernel->currentThread->space->ASID();
      mySeg->OpenFileID = OpenFileID;
      //inserting 

      for (int i=0; i<numSegPages; i++) {
	kernel->memoryManager->VMAllocate(startPage+i);
      }
     
      //copy the file to the memory 
      kernel->memoryManager->swapMan->FileToSwap(OpenedFile,
						 mySeg->theSegment, 
						 kernel->currentThread->space);
      // return SegmentID
      kernel->currentThread->space->segmentIdCounter++;
      kernel->currentThread->space->segmentIdTable->Insert(mySeg);
      //printf("finished SegmentId Insert\n");
      kernel->machine->WriteRegister(2, mySeg->SegmentID);
      return;
  } else {
    printf ("Error condition, Length is not > 0 - no bytes transferred\n");
    kernel->machine->WriteRegister(2,-1);
    return;
  }
}

void HandleMunmap(void)
{

  //remove the ptEntries from the process page table
  //remove the entries from the segmentID table
  //remove any entries from the IPT
  //invalidate the process virtual addresses in the TLB

  SegmentId segmentID;
  MySegment *MySegment;
  IPTEntry *iptEntry = new IPTEntry;
  IPTEntry *iptToRemove;
  PTEntry *ptEntry;
  OpenFile *OpenedFile;
  char PageBuffer[PageSize];
  int i;

  segmentID = (SegmentId)kernel->machine->ReadRegister(4);

 
  //this should return NULL or a pointer to the segment with the segmentID - test this
  
  //kernel->currentThread->space->segmentIdTable->Find(segmentID,
  // &MySegment);
  
  if (kernel->currentThread->space->segmentIdTable->Find(segmentID,
							 &MySegment)){
    printf("SegmentID is found in segmentID table\n");   
    
    //Creating an empty buffer
    for (i = 0; i < PageSize; i++) {
      PageBuffer[i] = NULL;
    }
    
    int startPage = (MySegment->theSegment->virtualAddr)/PageSize;
    printf("\nStartPage: %d\n", startPage);
    int numSegPages = divRoundUp(MySegment->theSegment->size, PageSize);
    printf("numSegPages: %d\n", numSegPages);
    int sspn = kernel->currentThread->space->vpn2ppn(startPage);
    printf("sspn: %d\n", sspn);
    //int ASID = MySegment->PID;
    
    // Find the open file
    for (int j=0; j <  kernel->currentThread->openedFileList->NumInList(); j++)
      {
	OpenedFile = kernel->currentThread->openedFileList->RemoveFront();
	kernel->currentThread->openedFileList->Append(OpenedFile);	  
	if (MySegment->OpenFileID == OpenedFile->file)
	  break;
      }
      
    printf("Openfile Id is: %d", OpenedFile->file);
    // deleting the page table
    for (i = 0; i < numSegPages; i++) {
      //nullifying the swap file

      //emptying the swap bitmap map.
      iptEntry->virtualPage = startPage+i;
      iptEntry->pid=kernel->currentThread->space->ASID();
      sspn = kernel->currentThread->space->vpn2ppn(startPage + i);

      if (kernel->memoryManager->ipt->IsInTable(iptHashKey(iptEntry))){
	IPTEntry *temp;
	kernel->memoryManager->ipt->Find(iptHashKey(iptEntry), &temp);
	kernel->memoryManager->swapMan->CopyMemToSwap(temp->physicalPage,
						 sspn);
      }

      kernel->memoryManager->swapMan->swapFile->
	ReadAt(PageBuffer, PageSize, sspn * PageSize);

      // Write out to disk
      if (i!=numSegPages-1) { //write an  entire page
	OpenedFile->WriteAt(PageBuffer, PageSize, PageSize* i );
      } else {
	int amtToWrite=MySegment->theSegment->size % PageSize;
	OpenedFile->WriteAt(PageBuffer, amtToWrite , PageSize* i );
      }
      //Removing from swap table of the process
      ptEntry = kernel->currentThread->space->pageTable->Remove(startPage +
								i);


      if (kernel->memoryManager->ipt->IsInTable(iptHashKey(iptEntry))) {
	  iptToRemove = 
	    kernel->memoryManager->ipt->Remove(iptHashKey(iptEntry));
	  //removing from the physical memory
	  kernel->memoryManager->memoryMap->Clear(iptToRemove->physicalPage);
      }
      
      //flush the TLB instead. This will reload by itself
      kernel->memoryManager->TLBFlush();

    }
    
    // deleting the segmentID table entry
    kernel->currentThread->space->segmentIdTable->Remove(segmentID);
    
    delete MySegment;
    return;
  } else {
    printf("SegmentID is not found in segmentID table\n");
    kernel->machine->WriteRegister(2,-1);
  }
}



#endif //PP3
