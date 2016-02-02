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
#include "hash.h"
#ifdef PP2_2
#include "noff.h"
#endif // PP2_2
#define UserStackSize		1024 	// increase this as necessary!
#define TopVM                   0x80000000
#define NumVMPages              TopVM/PageSize

#ifdef PP3_1
typedef int OpenFileId;
typedef TranslationEntry PTEntry;
int PTHashKey(PTEntry *ptEntry);
unsigned PTHashFunc(int key);

class MySegment;

int SegmentHashKey(MySegment *segment);
unsigned int SegmentHashFunc(int key);

class MySegment {
 public:
  segment *theSegment;
  int SegmentID;
  int PID;
  OpenFileId OpenFileID;
};

#endif // PP3_1

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
#ifdef PP3_1
    HashTable<int, PTEntry *> *pageTable; // make the page table a hash
    int vpn2ppn(int vpn);                   // returns the ppn of vpn
    PTEntry *GetPTEntry(int vpn);         // returns page entry pointer
    int stackPages;                        // number of stack pages

    HashTable<int, MySegment *> *segmentIdTable;
    int segmentIdCounter;
#else
    TranslationEntry *pageTable;
#endif // PP3_1
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
    void SetReadOnly(unsigned startPage, unsigned int pages, bool readOnly);
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
