// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "constants.h"

#ifdef PP2_2
#include "noff.h"
#endif // PP2_2
#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace();			// Create an address space.
    ~AddrSpace();			// De-allocate an address space

    void Execute(char *fileName);	// Run the the program
					// stored in the file "executable"

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
    unsigned int ASID() { return asid; }

    // Enable ExceptionHandler to access the page table
    friend void ExceptionHandler(ExceptionType which);
#ifdef PP2_1
    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
#endif // PP2_1

#ifdef PP2_3
    void PassArgs(void);                // Sets up stack with command line
    // arguments
#endif // PP2_3


#ifdef PP2_2
 public:
    TranslationEntry *pageTable;
    bool GetReady(char *fileName);	// Prepare to run the program
					// stored in the file "executable"
 private:
#else
  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
    				        // for now!
#endif // PP2_2

#ifdef PP2_2
    void LoadSegment(Segment *seg,  OpenFile *executable);     // Loads a segment into memory
    unsigned int stackOffset;		// Offset of stack after adding
    // arguments
    unsigned int argv0Addr, argvAddr[CMDLINEMAX];	// argv[0] and *argv[0] addresses
    int argc;				// # of command line args
    char *argv[CMDLINEMAX];   // With a 60 char command line can only have 30 args max.
    char comLine[CMDLINEMAX]; // command line working buffer
#endif // PP2_2


    unsigned int numPages;		// Number of pages in the virtual 
					// address space

    bool Load(char *fileName);		// Load the program into memory
					// return false if not found
#ifndef PP2_1
    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
#endif // PP2_1
    static unsigned int nextAsid;       // Need ASID for MIPS TLB
    unsigned int asid;
};

#endif // ADDRSPACE_H
