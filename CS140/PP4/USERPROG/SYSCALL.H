/* syscalls.h 
 * 	Nachos system call interface.  These are Nachos kernel operations
 * 	that can be invoked from user programs, by trapping to the kernel
 *	via the "syscall" instruction.
 *
 *	This file is included by user programs and by the Nachos kernel. 
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation 
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt		1000
#define SC_Exit		1001
#define SC_Exec		1002
#define SC_Join		1003
#define SC_Create	1004
#define SC_Open		1005
#define SC_Read		1006
#define SC_Write	1007
#define SC_Close	1008
#define SC_ThreadFork	1009
#define SC_ThreadYield	1010
#define SC_Length       1011
#define SC_Mmap         1012
#define SC_Munmap       1013
#define SC_Sbrk         1014
#define SC_Chdir        1015
#define SC_Lsdir        1016
#define SC_Mkdir        1017
#define SC_Remove	1018

#ifndef IN_ASM

/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the 
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking, 
 * from the system call entry point in exception.cc.
 */

/* Stop Nachos, and print out performance stats */
void Halt();		
 

/* Address space control operations: Exit, Exec, and Join */

/* This user program is done (status = 0 means exited normally). */
void Exit(int status);	

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;	
 
/* Run the executable, stored in the Nachos file "name", and return the 
 * address space identifier
 */
SpaceId Exec(char *name);
 
/* Only return once the the user program "id" has finished.  
 * Return the exit status.
 */
int Join(SpaceId id); 	
 

/* File system operations: Create, Open, Read, Write, Close
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 */
 
/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;	

/* when an address space starts up, it has two open files, representing 
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */

#define StandardInput	0  
#define StandardOutput	1  

/* A unique identifier for each segment associated with a Nachos
 * process: code, data, stack, and memory mapped files
 */
typedef int SegmentId;

/* Reserve SegmentId values for the code, data, and stack segments */
#define CodeSegment  0
#define DataSegment  1
#define StackSegment 2
 
/* Create a Nachos file, with "name" */
void Create(char *name);

/* Remove the Nachos file "name" */
void Remove(char *name);

/* Open the Nachos file "name", and return an "OpenFileId" that can 
 * be used to read and write to the file.
 */
OpenFileId Open(char *name);

/* Write "size" bytes from "buffer" to the open file. */
void Write(char *buffer, int size, OpenFileId id);

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int Read(char *buffer, int size, OpenFileId id);

/* Close the file, we're done reading and writing to it. */
void Close(OpenFileId id);



/* User-level thread operations: Fork and Yield.  To allow multiple
 * threads to run within a user program. 
 *
 * Could define other operations, such as LockAcquire, LockRelease, etc.
 */

/* Fork a thread to run a procedure ("func") in the *same* address space 
 * as the current thread.
 */
void ThreadFork(void (*func)());

/* Yield the CPU to another runnable thread, whether in this address space 
 * or not. 
 */
void ThreadYield();		

/* Return the length in bytes of the open file */
int Length(OpenFileId id);

/* Map the open file into the process address space at
 * virtual address "addr", extending for "length" bytes
 */
SegmentId Mmap(OpenFileId id, char *addr, int length);

/* Unmap a memory mapped file, removing it from the address space
 * and writing any outstanding modifications back to the file.
 * This call cannot be used to unmap the code, data, or stack segments.
 */
void Munmap(SegmentId segment);

/* Extend "segment" by "incr" bytes if possible (i.e. if this does not
 * overlap with another segment).  Return the number of bytes by which
 * the segment was actually extended.
 */
int Sbrk(SegmentId segment, int incr);



/* Routines for creating and listing directories and changing the 
 * current working directory of a process.
 */

/* Change the current working directory of the process to "dirname" */
int Chdir(char *dirname);

/* List the contents of the current working directory to the console */
void Lsdir(void);

/* Create the directory with pathname "dirname" */
int Mkdir(char *dirname);

#endif /* IN_ASM */

#endif /* SYSCALL_H */
