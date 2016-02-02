// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "constants.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();		// Initialize list of ready threads 
    ~Scheduler();		// De-allocate ready list

    void ReadyToRun(Thread* thread);	
    				// Thread can be dispatched.
    Thread* FindNextToRun();	// Dequeue first thread on the ready 
				// list, if any, and return thread.
    void Run(Thread* nextThread, bool finishing);
    				// Cause nextThread to start running
    void CheckToBeDestroyed();// Check if thread that had been
    				// running needs to be deleted
    void Print();		// Print contents of ready list
#ifdef PP1_3
    List<Thread *> *destroyList;
#endif // PP1_3

#ifdef PP1_5
    int IsInList(Thread *t) {return readyList->IsInList(t);}
    void RemoveFromList(Thread *t) {readyList->Remove(t);}
    Thread *Front();
#endif //PP1_5

#ifdef PP1_6
    void DoUpdate(void); // Applies the Update function to each thread on
    // the readyList
    List<Thread *> *updateList; // keeps a list of threads that need to be
    // updated. The list is created after Update has scanned the ready
    // list
#endif //PP1_6
#ifdef PP1_6_T
    void SelfTestScheduling();
#endif //PP1_6_T

#ifdef PP1_7	
    int genThreadID(); // returns an ID for a new thread and increments
    // the ThreadID counter.

    int genLockID(); // returns an ID for a new thread and increments the
    // LockID counter.
#endif
    
    // SelfTest for scheduler is implemented in class Thread
    
  private:
#ifdef PP1_5
    SortedList<Thread *> *readyList;
#else
    List<Thread *> *readyList;  // queue of threads that are ready to run,
				// but not running
#endif // PP1_5

#ifdef PP1_7	
    int ThreadID; // the counter of the IDs given to threads
    int LockID; // the counter of the IDs given to locks
#endif

    Thread *toBeDestroyed;	// finishing thread to be destroyed
    				// by the next thread that runs
#ifdef PP3
    int eraseTime;              // times how long last IPT clear was
				// performed
#endif // PP3
};

#ifdef PP1_3
void Cleanup(Thread *t);  //Garbage collector for deleted threads
#endif // PP1_3

#ifdef PP1_6
void Update(Thread *t);  // performs aging on threads and adjusts their
// priority accordingly
#endif // PP1_6
#ifdef PP1_6_T
void ComputeThread(int which);
void IOThread(int which);
#endif // PP1_6_T

#ifdef PP1_5
//Comparison routine to sort the priorities in the ready lis
int PriorityComp(Thread *x, Thread *y); 
#endif // PP1_5

#endif // SCHEDULER_H
