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

#ifdef PP3_1
#include memman.h
#endif

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
    default:
      printf("Unexpected system call %d\n", type);
      break;
    }
    break;
    
  default:
    printf("Unexpected user mode exception %d\n", which);
    HandleUnexpected();
    break;
    
  }
  
#ifdef PP2_1
  kernel->machine->IncrementPC(); 
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

  //printf("Exiting thread [%s]\n", kernel->currentThread->getName());

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
  
  Thread *newThread;
  int cmdLinePtr; // ptr to command line in user space
  int threadID;   // return value for exec
  char *cmdLine; // buffer to hold command line
  
  cmdLine = new char[CMDLINEMAX];
  
  // get command line
  cmdLinePtr = kernel->machine->ReadRegister(4);

  // copy command line in ther kernel space
  kernel->memoryManager->UserToKernel(cmdLine, cmdLinePtr, 60);
  
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
  
  // printf("SC_Exec returning threadID: %d\n", threadID);
  kernel->machine->WriteRegister(2,threadID);
  
}

//------------------------------------------------------------------------
// HandleJoin
//   Exception handler for SC_Exec
//------------------------------------------------------------------------
void HandleJoin(void) {
  DEBUG(dbgAddr, ("Join called\n"));
  
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
    //	   desiredChild->getName(), desiredChild->GetExitCode());
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
  
  kernel->memoryManager->UserToKernel(filename, pointerToOpenFile, 60);
  
  
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
  
  kernel->memoryManager->UserToKernel(filename, pointerToOpenFile, 60);
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
      for ( i=0; i<size; i++)
	{
	  MyBuffer[i] = kernel->consoleInput->GetChar();
	}
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
      for ( i=0; i<size; i++)
	{
	  kernel->consoleOutput->PutChar(MyBuffer[i]);
	}
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
