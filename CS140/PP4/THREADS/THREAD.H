// thread.h 
//	Data structures for managing threads.  A thread represents
//	sequential execution of code within a program.
//	So the state of a thread includes the program counter,
//	the processor registers, and the execution stack.
//	
// 	Note that because we allocate a fixed size stack for each
//	thread, it is possible to overflow the stack -- for instance,
//	by recursing to too deep a level.  The most common reason
//	for this occuring is allocating large data structures
//	on the stack.  For instance, this will cause problems:
//
//		void foo() { int buf[1000]; ...}
//
//	Instead, you should allocate all data structures dynamically:
//
//		void foo() { int *buf = new int[1000]; ...}
//
//
// 	Bad things happen if you overflow the stack, and in the worst 
//	case, the problem may not be caught explicitly.  Instead,
//	the only symptom may be bizarre segmentation faults.  (Of course,
//	other problems can cause seg faults, so that isn't a sure sign
//	that your thread stacks are too small.)
//	
//	One thing to try if you find yourself with seg faults is to
//	increase the size of thread stack -- ThreadStackSize.
//
//  	In this interface, forking a thread takes two steps.
//	We must first allocate a data structure for it: "t = new Thread".
//	Only then can we do the fork: "t->fork(f, arg)".
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#ifndef THREAD_H
#define THREAD_H

class Thread;


#include "copyright.h"
#include "utility.h"
#include "sysdep.h"
#include "constants.h"

#include "list.h"

class Lock;
class Semaphore;

#ifdef PP1_7
class Lock;
class Condition;

struct TestStruct {
  Lock *L1;
  Lock *L2;
  Lock *L3;
  Condition *Sem;
};
#endif

template <class T> class List;

#ifdef PP2_1
#include "list.h"
#include "openfile.h"
#endif //PP2_1

#ifdef USER_PROGRAM
#include "machine.h"
#include "addrspace.h"
#endif //USER_PROGRAM


//class Thread;

#ifdef PP2_1
class Semaphore;
#endif //PP2_1


// CPU register state to be saved on context switch.  
// The x86 needs to save only a few registers, 
// SPARC and MIPS needs to save 10 registers, 
// the Snake needs 18,
// and the RS6000 needs to save 75 (!)
// For simplicity, I just take the maximum over all architectures.

#define MachineStateSize 75 


// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!
const int StackSize = (4 * 1024);	// in words


// Thread state
enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED };

#ifdef PP1_2
class Mailbox;
struct dstruct
{
  Mailbox *mbox;
  int *message;
  
}; //dstruct

#endif PP1_2

// The following class defines a "thread control block" -- which
// represents a single thread of execution.
//
//  Every thread has:
//     an execution stack for activation records ("stackTop" and "stack")
//     space to save CPU registers while not running ("machineState")
//     a "status" (running/ready/blocked)
//    
//  Some threads also belong to a user address space; threads
//  that only run in the kernel have a NULL address space.

class Thread {
  private:
    // NOTE: DO NOT CHANGE the order of these first two members.
    // THEY MUST be in this position for SWITCH to work.
    int *stackTop;			 // the current stack pointer
    void *machineState[MachineStateSize];  // all registers except for stackTop

  public:
#ifdef PP2_1
    Thread(char* debugName, bool joinMayBeCalled=TRUE);		// initialize a
#else
    Thread(char* debugName);
#endif
 
    ~Thread(); 				// deallocate a Thread
					// NOTE -- thread being deleted
					// must not be running when delete 
					// is called

    // basic thread operations

    void Fork(VoidFunctionPtr func, void *arg); 
    				// Make thread run (*func)(arg)
    void Yield();  		// Relinquish the CPU if any 
				// other thread is runnable
    void Sleep(bool finishing); // Put the thread to sleep and 
				// relinquish the processor
    void Begin();		// Startup code for the thread	
    void Finish();  		// The thread is done executing
    
#ifdef PP2_1
    void Join();
    void SetExitCode(int code); // Sets exit code
    int GetExitCode(void); // Gets exit code
    List<OpenFile *> *openedFileList;	// Maintains a list of opened
    // files
    List<Thread *> *childList;
#endif //PP2_1


    void CheckOverflow();   	// Check if thread stack has overflowed
    void setStatus(ThreadStatus st) { status = st; }
    char* getName() { return (name); }
    void Print() { printf("%s",name); }
    void SelfTest();		// test whether thread impl is working
    
#ifdef PP1_2
    void MBoxSender();          // declaration of new member fxn. 
    void MBoxReceiver();        // declaration of new member fxn. 
    void SelfTestMailBoxes();		// test whether thread impl is working
    
    dstruct Mstruct;

#endif// PP1_2

#ifdef PP1_3
//    void Join(void);		// blocks parent until thread is finished
    void CanBeKilled(void);	// allow thread to kill itself
    void AddChild(Thread *child);    // adds child to list of children
    void SelfTestJoin(void);
    void SelfTestJoinA(void);
    int OkToKill;	// indicates when a thread can go
                        // ahead and kill itself
    int Finished;       // indicate when a thread has Finished runing
#endif // PP1_3

#ifdef PP1_5
    int priority;	// thread priority (0-59)
    void SetPriority(int newPriority); // Sets the original priority
    // of a thread

    void SelfTestPriorityInversion(void); // self test the priority
    // inversion
    // support functions and vars to self test priority inversion
    void PriInvHelper1(void);
    void PriInvHelper2(void);
    Lock *lockThing;
    Semaphore *semSynch;

#endif //PP1_5
   
#ifdef PP1_6
    long timeleft; // time remaining in quantum
    long dispwait; // time thread has been waiting on readyList
#endif // PP1_6
#ifdef PP1_7
    int getID();  // return the ID of the thread
    Lock* getLockWaited(); // return the lock that the thread is
    void setLockWaited(Lock *NewWaitedLock);
    // waiting for 
    void DeadLockCheck(Thread* InitThread, Lock* CurrentLock, Thread* OtherThread, Lock* OtherLockWaited);
    void DeadLockTest();
    void SelfTestDeadLock();
#endif

#ifdef PP4
    OpenFile *GetCurDirFile();
    void SetCurDirectory(OpenFile *NewDir);
#endif // PP4

  private:
    // some of the private data for this class is listed above

#ifdef PP1_3
    Thread *parent;			// indicates this threads parent
    Semaphore *semFinished;		// Indicating when thread is finished

    List<Thread *> *childrenList;	// List of childs of this thread
#endif //PP1_3
    
#ifdef PP1_7
    int ID;      // Unique integer identifying this thread
    int LockWaitedID; // The ID of the lock the thread is waiting for
    Lock *LockWaited;
    TestStruct MyStruct;
#endif

#ifdef PP2_1
    int exitCode;		// Exit code for this thread
    bool joinable;
    bool deletable;
    bool finished;
    Semaphore *joinSem;

    void Detach();
    

#endif // PP2_1

#ifdef PP4
    OpenFile* curDirFile;
#endif // PP4
    
    int *stack; 	 	// Bottom of the stack 
				// NULL if this is the main thread
				// (If NULL, don't deallocate stack)
    ThreadStatus status;	// ready, running or blocked
    char* name;

    void StackAllocate(VoidFunctionPtr func, void *arg);
    				// Allocate a stack for thread.
				// Used internally by Fork()

#ifdef USER_PROGRAM
// A thread running a user program actually has *two* sets of CPU registers -- 
// one for its state while executing user code, one for its state 
// while executing kernel code.

    int userRegisters[NumTotalRegs];	// user-level CPU register state

  public:
    void SaveUserState();		// save user-level register state
    void RestoreUserState();		// restore user-level register state

    AddrSpace *space;			// User code this thread is running.
#endif //USER_PROGRAM
};

// external function, dummy routine whose sole job is to call Thread::Print
extern void ThreadPrint(Thread *thread);	 

// Magical machine-dependent routines, defined in switch.s

extern "C" {
// First frame on thread execution stack; 
//   	call ThreadBegin
//	call "func"
//	(when func returns, if ever) call ThreadFinish()
void ThreadRoot();

// Stop running oldThread and start running newThread
void SWITCH(Thread *oldThread, Thread *newThread);
}

#endif // THREAD_H
