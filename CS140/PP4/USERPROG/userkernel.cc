// userkernel.cc 
//	Initialization and cleanup routines for the version of the
//	Nachos kernel that supports running user programs.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "synchconsole.h"
#include "userkernel.h"
#include "constants.h"
#ifdef PP2_2
void CallMachineRun(int junk);
#include "memman.h"
#include "synch.h"
//class MemoryManager;
#endif // PP2_2

//----------------------------------------------------------------------
// UserProgKernel::UserProgKernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

UserProgKernel::UserProgKernel(int argc, char **argv) 
		: ThreadedKernel(argc, argv)
{
    debugUserProg = FALSE;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
	    debugUserProg = TRUE;
        } else if (strcmp(argv[i], "-u") == 0) {
            printf("Partial usage: nachos [-s]\n");
	}
    }
}

//----------------------------------------------------------------------
// UserProgKernel::Initialize
// 	Initialize Nachos global data structures.
//----------------------------------------------------------------------

void
UserProgKernel::Initialize()
{
#ifdef PP4

    machine = new Machine(debugUserProg);
    headManager = new HeadManager();
    dirManager =  new DirectoryManager();
    memoryManager = new MemoryManager;

#endif //PP4

    ThreadedKernel::Initialize();	// init multithreading

    consoleInput = new SynchConsoleInput(NULL);
    consoleOutput = new SynchConsoleOutput(NULL);
#ifndef PP4
    machine = new Machine(debugUserProg);
#ifdef PP2_2
    memoryManager = new MemoryManager;
#endif // PP2_2
#endif //PP4
#ifdef PP2_1

    consoleInputLock = new Lock("console input");
    consoleOutputLock = new Lock("console output");

#endif //PP2_1

}

//----------------------------------------------------------------------
// UserProgKernel::~UserProgKernel
// 	Nachos is halting.  De-allocate global data structures.
//	Automatically calls destructor on base class.
//----------------------------------------------------------------------

UserProgKernel::~UserProgKernel()
{
    delete machine;

#ifdef PP2_2
   delete  memoryManager;
#endif // PP2_2

#ifdef PP4
   delete dirManager;
   delete headManager;
#endif

#ifdef PP2_1
    delete consoleInputLock;
    delete consoleOutputLock;
#endif //PP2_1
}

//----------------------------------------------------------------------
// UserProgKernel::Run
// 	Run the Nachos kernel.  For now, just run the "halt" program. 
//----------------------------------------------------------------------

void
UserProgKernel::Run()
{
#ifdef FILESYS
  Thread *shell;
  shell = new Thread("shell");
  shell->space = new AddrSpace();
  shell->space->GetReady("/shell");
#ifdef PP4
  shell->SetCurDirectory(kernel->fileSystem->GetDirectoryFile());
# endif //PP4
  //shell->space->GetReady("../test/SUN/echo hello world");
  shell->Fork((VoidFunctionPtr) CallMachineRun,  (void *) 0);

    //halt->Execute("halt");
#else
#ifdef SIMOS
    halt->Execute("../test/SIMOS/halt");
#else

#ifdef PP2_1
    Thread *shell;
    shell = new Thread("shell");
    shell->space = new AddrSpace();
    shell->space->GetReady("../test/SUN/shell");
    //shell->space->GetReady("../test/SUN/echo hello world");
    shell->Fork((VoidFunctionPtr) CallMachineRun,  (void *) 0);

    //halt->Execute("../test/SUN/shell");
#else
    halt->Execute("../test/SUN/halt");
#endif // PP2_1


#endif
#endif // FILESYS

    ThreadedKernel::Run();
}

//----------------------------------------------------------------------
// UserProgKernel::SelfTest
//      Test whether this module is working.
//----------------------------------------------------------------------

void
UserProgKernel::SelfTest() {
    char ch;

    ThreadedKernel::SelfTest();

    // test out the console device

    printf("Testing the console device.\n" 
           "Typed characters will be echoed, until q is typed.\n"
           "Note newlines are needed to flush the input line.\n");
#ifndef SIMOS
    fflush(stdout);
#endif

    do {
    	ch = consoleInput->GetChar();
    	consoleOutput->PutChar(ch);   // echo it!
    } while (ch != 'q');

    printf("\n");

    // self test for running user programs is to run the halt program above
}

