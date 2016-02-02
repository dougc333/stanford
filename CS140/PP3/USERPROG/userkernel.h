// userkernel.h
//	Global variables for the Nachos kernel, for the assignment
//	supporting running user programs.
//
//	The kernel supporting user programs is a version of the 
//	basic multithreaded kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#ifndef USERKERNEL_H  
#define USERKERNEL_H

class UserProgKernel;

#include "kernel.h"
#include "machine.h"
#include "synchconsole.h"
#include "constants.h"

#ifdef PP2_2
#include "memman.h"
class MemoryManager;
#endif // PP2_2


class UserProgKernel : public ThreadedKernel {
  public:
    UserProgKernel(int argc, char **argv);
				// Interpret command line arguments
    ~UserProgKernel();		// deallocate the kernel

    void Initialize();		// initialize the kernel 

    void Run();			// do kernel stuff 

    void SelfTest();		// test whether kernel is working

// These are public for notational convenience.
    Machine *machine;
    SynchConsoleOutput *consoleOutput;
    SynchConsoleInput *consoleInput;

#ifdef PP2_2
    MemoryManager *memoryManager;

#ifdef PP2_1
    Lock *consoleInputLock;
    Lock *consoleOutputLock;
#endif

#endif // PP2_2

  private:

    bool debugUserProg;		// single step user program
};

#endif



