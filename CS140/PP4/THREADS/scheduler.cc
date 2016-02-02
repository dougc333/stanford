// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

#ifdef PP1_6
#include "dptbl.h"
#endif // PP1_6_T
#ifdef PP1_6_T
#include "synch.h"
#endif // PP1_6

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{ 
#ifdef PP1_5
  readyList = new SortedList<Thread *>(PriorityComp); 
#else
  readyList = new List<Thread *>; 
#endif //PP1_5

#ifdef PP1_3
    destroyList=new List<Thread *>; 
#endif // PP1_3

#ifdef PP1_6	
    updateList=new List<Thread *>;
#endif

#ifdef PP1_7	
    ThreadID = 0;
    LockID = 0;
#endif

    toBeDestroyed = NULL;
#ifdef PP3
    int eraseTime = 0;
#endif // PP3
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList;
#ifdef PP1_3
    while (!destroyList->IsEmpty())
      destroyList->RemoveFront();
    delete destroyList;
#endif // PP1_3
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread,("Putting thread on ready list: %s\n", thread->getName()));

    thread->setStatus(READY);
#ifdef PP1_5
    // check if the currently running thread is of lower priority.  If so
    // then premept it with recently added thread.
    readyList->Insert(thread);
    if ((thread!=kernel->currentThread) && (thread->priority >
					     kernel->currentThread->priority))
      kernel->currentThread->Yield();
#else
    readyList->Append(thread);
#endif //PP1_5

#ifdef PP1_6
    // initialize the waiting time

    thread->dispwait=0;
#endif // PP1_6
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

#ifdef PP1_4
     Thread *thread;
    // if alarm list not empty test for threads to insert into ready list from alarm list
    while (!(kernel->alarm->alarmlistempty())) 
      {
	//read and test an item from the list to see if it should be 
	// added to the ready queue
	// test for inequalities only
	//declare test to be an alarm object
	bool testtime; //can be a bool
	testtime = kernel->alarm->timeisup;
	if (testtime > kernel->stats->totalTicks)
	  {
	    //add corresponding alarmlist thread to the ready queue
	    kernel->alarm->Addthread();
	    
	  }
      }
#endif

    if (readyList->IsEmpty()) {
	return NULL;
    } else {
    	return readyList->RemoveFront();
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

#ifdef SIMOS
extern "C" void SetASID(int);
#endif

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
    
    ASSERT(kernel->interrupt->getLevel() == IntOff);
#ifdef PP3
/*    if (eraseTime==ERASETIME) {
      kernel->memoryManager->IPTClearUse();
      eraseTime=0;
    } else
      eraseTime++;

    if (nextThread==oldThread) {
      //       ASSERT(kernel->interrupt->getLevel() == IntOff);
       return;
       }*/
#endif

#ifdef PP1_6
    nextThread->timeleft=dptbl[nextThread->priority][QUANTUM];
#endif // PP1_6

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	 toBeDestroyed = oldThread;
    }
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	oldThread->space->SaveState();
    }
#endif

    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread,("Switching from: %s (%x) to: %s (%x)\n", oldThread->getName(), (int)oldThread, nextThread->getName(), (int)nextThread));

    // printf("Switching from: %s (%x) to: %s (%x)\n", oldThread->getName(), (int)oldThread, nextThread->getName(), (int)nextThread);
#ifdef PP3
    kernel->memoryManager->TLBFlush(); // make sure TLB is clean 
#endif // PP3
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".


    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread,("Now in thread: %s (%x)\n", oldThread->getName(), (int)oldThread));

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
#ifdef PP1_3
      destroyList->Append(toBeDestroyed); // keep a list of who needs to
      // be deleted
#else
        delete toBeDestroyed;
#endif
	toBeDestroyed = NULL;
    }
#ifdef PP1_3
    // Do garbage clean up by seeing which Threads can be deleted
    if (!destroyList->IsEmpty())
      destroyList->Apply((void (*)(Thread *))Cleanup);
#endif //PP1_3
}
 
#ifdef PP1_3

//----------------------------------------------------------------------
// Cleanup
// 	Does garbage clean up of threads that are ready to be deleted.
//  A thread that is ready to be deleted has been told by its parent that
//  it is becoming an orphan by setting its OkToKill flag in the child
//----------------------------------------------------------------------
void Cleanup(Thread *t)
{
  if (t->OkToKill==1) {
    kernel->scheduler->destroyList->Remove(t);
    delete t;

  }
}
#endif //PP1_3

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    printf("Ready list contents:\n");
    readyList->Apply(ThreadPrint);
}

#ifdef PP1_5
//----------------------------------------------------------------------
// PriorityComp
// 	Sorts the ready list based on priority of a thread.  Threads with 
// highest priority are placed at the front of the list
// returns -1 if x>y
// returns 0 if x==y
// return 1 if x<y
//----------------------------------------------------------------------
int PriorityComp(Thread *x, Thread *y)
{
  if (x->priority > y->priority)
    return -1;
  else if (x->priority == y->priority)
    return 0;
  else return 1;
}

//----------------------------------------------------------------------
// Scheduler::Front
//  Encapsulates readyList->Front()
//----------------------------------------------------------------------
Thread *
Scheduler::Front()
{
  ASSERT(kernel->interrupt->getLevel() == IntOff);
  
  if (readyList->IsEmpty()) {
    return NULL;
  } else {
    return readyList->Front();
  }
}

#endif //PP1_5

#ifdef PP1_6
//----------------------------------------------------------------------
// Scheduler::Updates
//  Applies aging function on threads in readyList.
//----------------------------------------------------------------------
void Scheduler::DoUpdate(void) {
  Interrupt *interrupt = kernel->interrupt;
  Thread *currentThread = kernel->currentThread;
  Thread *uThread; // thread to update
  // disable interrupts
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	
  
  if (!readyList->IsEmpty())
    readyList->Apply((void (*)(Thread *))Update);

  while (!updateList->IsEmpty()) {
    uThread=updateList->RemoveFront();
    //uThread->SetPriority(dptbl[uThread->priority][LWAIT]);
    readyList->Remove(uThread);
    readyList->Insert(uThread);
  }
    
  // re-enable interrupts
    (void) interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------
// Update
//   Ages threads according to their waiting time. If they have waited too
// long then their priority is adjusted
//----------------------------------------------------------------------
void Update(Thread *t)
{
  // increament wait time
  t->dispwait++;
  // see if wait time has exceed maximum waiting time
  if (t->dispwait > dptbl[t->priority][MAXWAIT]) {
    t->dispwait=0;
    kernel->scheduler->updateList->Append(t);
  }
}
#endif //PP1_6

#ifdef PP1_6_T
//----------------------------------------------------------------------
// Scheduler::SelfTestScheduling
//  Fires off 4 compute intensive and 4 IO intenstive threads the IO
// threads should finish up with higher priority and the compute intensive
// threads should finish with lower priority.
//----------------------------------------------------------------------
void Scheduler::SelfTestScheduling() {
  // compute intensive threads
  Thread *c1=new Thread("Compute1");
  Thread *c2=new Thread("Compute2");
  Thread *c3=new Thread("Compute3");
  Thread *c4=new Thread("Compute4");
 // io intensive threads
  Thread *i1=new Thread("IO1");
  Thread *i2=new Thread("IO2");
  Thread *i3=new Thread("IO3");
  Thread *i4=new Thread("IO4");

  printf("\nStarting multilevel feedback scheduling test.\n");

  c1->Fork((VoidFunctionPtr) ComputeThread, (void *) 1);
  i1->Fork((VoidFunctionPtr) IOThread, (void *) 1);
  c2->Fork((VoidFunctionPtr) ComputeThread, (void *) 2);
  i2->Fork((VoidFunctionPtr) IOThread, (void *) 2);
  c3->Fork((VoidFunctionPtr) ComputeThread, (void *) 3);
  i3->Fork((VoidFunctionPtr) IOThread, (void *) 3);
  c4->Fork((VoidFunctionPtr) ComputeThread, (void *) 4);
  i4->Fork((VoidFunctionPtr) IOThread, (void *) 4);
}



void ComputeThread(int which) {
  Lock *mylock= new Lock("MyPrivateLock");
  printf("Starting compute intensive thread %d at priority %d.\n",which,
	   kernel->currentThread->priority);
  for (int i=0; i < 200; i++) {
    mylock->Acquire();
    //    printf("Compute thread %d priority is %d.\n",which,
    //	   kernel->currentThread->priority);
    mylock->Release();
  }
  printf("Compute thread %d priority is %d.\n",which,
	 kernel->currentThread->priority);
}

void IOThread(int which) {
  printf("Starting IO intensive thread %d at priority %d.\n",which,
	   kernel->currentThread->priority);
  for (int i=0; i < 200; i++) {
    //  printf("IO thread %d priority is %d.\n",which,
    //   kernel->currentThread->priority);
    kernel->currentThread->Yield();
  }
   printf("IO thread %d priority is %d.\n",which,
	   kernel->currentThread->priority);
}
#endif //PP1_6_T

#ifdef PP1_7	
int 
Scheduler::genThreadID() 
{
	//Thread_LockWaited.resize(Thread_LockWaited.size() + 1);
	ThreadID++;
	return (ThreadID - 1);
}

int 
Scheduler::genLockID()
{
	//Lock_ThreadOwner.resize(Lock_ThreadOwner.size() + 2*sizeof(int));
	LockID++;
	return (LockID - 1);
}
#endif
