// thread.cc 
//	Routines to manage threads.  These are the main operations:
//
//	Fork -- create a thread to run a procedure concurrently
//		with the caller (this is done in two steps -- first
//		allocate the Thread object, then call Fork on it)
//	Begin -- called when the forked procedure starts up, to turn
//		interrupts on and clean up after last thread
//	Finish -- called when the forked procedure finishes, to clean up
//	Yield -- relinquish control over the CPU to another ready thread
//	Sleep -- relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly 
//		put back on the ready queue.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "sysdep.h"
#include "constants.h"
#ifdef PP1_2
#include "mailbox.h"
#endif // PP1_2
#ifdef PP1_6
#include "dptbl.h"
#endif // PP1_6
#ifdef PP2_1
#include "list.h"
#include "openfile.h"
#endif

// this is put at the top of the execution stack, for detecting stack overflows
const int STACK_FENCEPOST = 0xDEADBEEF;

//----------------------------------------------------------------------
// Thread::Thread
// 	Initialize a thread control block, so that we can then call
//	Thread::Fork.
//
//	"threadName" is an arbitrary string, useful for debugging.
//----------------------------------------------------------------------

#ifdef PP2_1
Thread::Thread(char* threadName, bool joinMayBeCalled)
{
    name = threadName;
    stackTop = NULL;
    stack = NULL;
    status = JUST_CREATED;
    for (int i = 0; i < MachineStateSize; i++) {
	machineState[i] = NULL;		// not strictly necessary, since
					// new thread ignores contents 
					// of machine registers
    }
    joinable = joinMayBeCalled;
    deletable = !joinable;
    finished = false;
    childList = new List<Thread *>;
    if (joinable) {
      joinSem = new Semaphore("Join Semaphore", 1);
    }
#ifdef USER_PROGRAM
    space = NULL;
#endif
#ifdef PP2_1
    openedFileList = new List<OpenFile *>;
#endif // PP2_1 
}
#else //use non Join constructor
Thread::Thread(char* threadName)
{
    name = threadName;
#ifdef PP1_7

    ID = kernel->scheduler->genThreadID();  //Get an ID number from the
                                            // scheduler 
#endif
    stackTop = NULL;
    stack = NULL;
    status = JUST_CREATED;
    for (int i = 0; i < MachineStateSize; i++) {
	machineState[i] = NULL;		// not strictly necessary, since
					// new thread ignores contents 
					// of machine registers
    }
#ifdef USER_PROGRAM
    space = NULL;
#endif
#ifdef PP1_3
    semFinished = new Semaphore("finished", 0);  // Init. to not finished
    OkToKill=1;  // you can kill the thread before it has been forked
    Finished=0;
    childrenList = new List<Thread *>;
    parent=NULL;  // init. to no parent (this could be the main thread);
#endif //PP1_3

#ifdef PP1_5
    priority=29;  // Set the priority Solaris default
#endif // PP1_5   
}
#endif // PP 2_1

//----------------------------------------------------------------------
// Thread::~Thread
// 	De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      NOTE: if this is the main thread, we can't delete the stack
//      because we didn't allocate it -- we got it automatically
//      as part of starting up Nachos.
//----------------------------------------------------------------------

Thread::~Thread()
{
    DEBUG(dbgThread,("Deleting thread: %s\n", name));

    ASSERT(this != kernel->currentThread);
#ifdef PP2_1
    // Close all open files by this thread
    while(!openedFileList->IsEmpty()) {
      delete openedFileList->RemoveFront();
    }
    delete openedFileList;
#endif // PP2_1    
#ifdef PP4
    delete curDirFile;
#endif
    if (stack != NULL)
	DeallocBoundedArray((char *) stack, StackSize * sizeof(int));
}

//----------------------------------------------------------------------
// Thread::Fork
// 	Invoke (*func)(arg), allowing caller and callee to execute 
//	concurrently.
//
//	NOTE: although our definition allows only a single argument
//	to be passed to the procedure, it is possible to pass multiple
//	arguments by making them fields of a structure, and passing a pointer
//	to the structure as "arg".
//
// 	Implemented as the following steps:
//		1. Allocate a stack
//		2. Initialize the stack so that a call to SWITCH will
//		cause it to run the procedure
//		3. Put the thread on the ready queue
// 	
//	"func" is the procedure to run concurrently.
//	"arg" is a single argument to be passed to the procedure.
//----------------------------------------------------------------------

void 
Thread::Fork(VoidFunctionPtr func, void *arg)
{
    Interrupt *interrupt = kernel->interrupt;
    Scheduler *scheduler = kernel->scheduler;
    IntStatus oldLevel;
    
    DEBUG(dbgThread,("Forking thread: %s f(a): 0x%x 0x%x\n", name, (int) func, (int) arg));
 
#ifdef PP1_3    
    OkToKill=0;  // Now that you have fork you must wait for parent to
		// to die before you can die 
    
    parent = kernel->currentThread;	// The parent is the one that
    // forked

    parent->AddChild(this); 

#endif // PP1_3 

    StackAllocate(func, arg);

#ifdef PP2_1
 if (joinable) {
      joinSem->P();
      if (kernel->currentThread)
	kernel->currentThread->childList->Append(this);
    }
#endif // PP2_1



    oldLevel = interrupt->SetLevel(IntOff);
    kernel->numThreads++;               

    scheduler->ReadyToRun(this);	// ReadyToRun assumes that interrupts 
					// are disabled!
    (void) interrupt->SetLevel(oldLevel);
}    

//----------------------------------------------------------------------
// Thread::CheckOverflow
// 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
//----------------------------------------------------------------------

void
Thread::CheckOverflow()
{
    if (stack != NULL) {
#ifdef HPUX			// Stacks grow upward on the Snakes
	ASSERT(stack[StackSize - 1] == STACK_FENCEPOST);
#else
	ASSERT(*stack == STACK_FENCEPOST);
#endif
   }
}

//----------------------------------------------------------------------
// Thread::Begin
// 	Called by ThreadRoot when a thread is about to begin
//	executing the forked procedure.
//
// 	It's main responsibilities are:
//	1. deallocate the previously running thread if it finished 
//		(see Thread::Finish())
//	2. enable interrupts (so we can get time-sliced)
//----------------------------------------------------------------------

void
Thread::Begin ()
{
    ASSERT(this == kernel->currentThread);
    DEBUG(dbgThread,("Beginning thread: %s\n", name));
    
    kernel->scheduler->CheckToBeDestroyed();
    kernel->interrupt->Enable();
}

//----------------------------------------------------------------------
// Thread::Finish
// 	Called by ThreadRoot when a thread is done executing the 
//	forked procedure.
//
// 	NOTE: we can't immediately de-allocate the thread data structure 
//	or the execution stack, because we're still running in the thread 
//	and we're still on the stack!  Instead, we tell the scheduler
//	to call the destructor, once it is running in the context of a different thread.
//
// 	NOTE: we disable interrupts, because Sleep() assumes interrupts
//	are disabled.
//----------------------------------------------------------------------

//
#ifdef PP2_1
void
Thread::Finish ()
{
  if (joinable) {
      joinSem->V();
  }

    Thread *joinableChild = NULL;
    while (!childList->IsEmpty()) {
      joinableChild = childList->RemoveFront();
      joinableChild->Detach();
    }
    delete childList;
    
    (void) kernel->interrupt->SetLevel(IntOff);		
    ASSERT(this == kernel->currentThread);
    
    DEBUG(dbgThread,("Finishing thread: %s\n", name));
   
    Sleep(TRUE);				// invokes SWITCH
    // not reached
}
#else //not using join in PP2_1
void
Thread::Finish ()
{
    (void) kernel->interrupt->SetLevel(IntOff);		
    ASSERT(this == kernel->currentThread);
    
    DEBUG(dbgThread,("Finishing thread: %s\n", name));
#ifdef PP1_3
    Thread *disOwnedThread;

    Finished=1; // Make all following Joins pass through
    semFinished->V();  // Signal to join to allow parent to continue

    // Let all the children kill themselves.
    while (!childrenList->IsEmpty()) {
      disOwnedThread = childrenList->RemoveFront();
      disOwnedThread -> CanBeKilled();
    }
    

#endif // PP1_3
 
    Sleep(TRUE);				// invokes SWITCH
    // not reached
}
#endif //PP2_1

//----------------------------------------------------------------------
// Thread::Yield
// 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled. 
//
// 	Similar to Thread::Sleep(), but a little different.
//----------------------------------------------------------------------

void
Thread::Yield ()
{
    Thread *nextThread;
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    
    ASSERT(this == kernel->currentThread);

#ifdef PP1_6

    // if you ran out of quantum you priority is set to TQEXP
    if (timeleft == 0) 
      priority = dptbl[priority][TQEXP];
    else // you block your priority is set to slpret
      priority = dptbl[priority][SLPRET];

    // reset time left after a yeild
    timeleft = dptbl[priority][QUANTUM];
#endif //PP1_6

#ifdef PP1_5
    if (kernel->scheduler->Front() == this)
      return;
#endif //PP1_5

    // Cannot yield if we're in the interrupt handler, so call YieldOnReturn
    if (kernel->interrupt->InHandler()) {
      kernel->interrupt->YieldOnReturn();
      return;
    }

    DEBUG(dbgThread,("Yielding thread: %s\n", name));
    
    nextThread = kernel->scheduler->FindNextToRun();
    if (nextThread != NULL) {
	kernel->scheduler->ReadyToRun(this);
	kernel->scheduler->Run(nextThread, FALSE);
    }
    (void) kernel->interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Thread::Sleep
// 	Relinquish the CPU, because the current thread has either
//	finished or is blocked waiting on a synchronization 
//	variable (Semaphore, Lock, or Condition).  In the latter case,
//	eventually some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off 
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
//----------------------------------------------------------------------
void
Thread::Sleep (bool finishing)
{
    Thread *nextThread;
    
    ASSERT(this == kernel->currentThread);
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    
    DEBUG(dbgThread,("Sleeping thread: %s\n", name));

    if (finishing) {
      kernel->numThreads--;
      // If this was the last kernel thread, halt the system 
      if (kernel->numThreads == 0) {
        kernel->interrupt->Halt();
      }
    }

    status = BLOCKED;
    while ((nextThread = kernel->scheduler->FindNextToRun()) == NULL)
	kernel->interrupt->Idle();	// no one to run, wait for an interrupt
    
    // returns when it's time for us to run
    kernel->scheduler->Run(nextThread,deletable ? finishing: false); 
}

void Thread::Join() {
  if (joinable) {
    joinSem->P();
    joinSem->V();
  }
}

void Thread::Detach() {
  deletable = true;
  if (finished)
    delete this;
}

//----------------------------------------------------------------------
// ThreadBegin, ThreadFinish,  ThreadPrint
//	Dummy functions because C++ does not (easily) allow pointers to member
//	functions.  So we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the 
//	member function.
//----------------------------------------------------------------------

static void ThreadFinish()    { kernel->currentThread->Finish(); }
static void ThreadBegin() { kernel->currentThread->Begin(); }
void ThreadPrint(Thread *t) { t->Print(); }

#ifdef PARISC

//----------------------------------------------------------------------
// PLabelToAddr
//	On HPUX, function pointers don't always directly point to code,
//	so we need to do the conversion.
//----------------------------------------------------------------------

static void *
PLabelToAddr(void *plabel)
{
    int funcPtr = (int) plabel;

    if (funcPtr & 0x02) {
        // L-Field is set.  This is a PLT pointer
        funcPtr -= 2;	// Get rid of the L bit
        return (*(void **)funcPtr);
    } else {
        // L-field not set.
        return plabel;
    }
}
#endif

//----------------------------------------------------------------------
// Thread::StackAllocate
//	Allocate and initialize an execution stack.  The stack is
//	initialized with an initial stack frame for ThreadRoot, which:
//		enables interrupts
//		calls (*func)(arg)
//		calls Thread::Finish
//
//	"func" is the procedure to be forked
//	"arg" is the parameter to be passed to the procedure
//----------------------------------------------------------------------

void
Thread::StackAllocate (VoidFunctionPtr func, void *arg)
{
    stack = (int *) AllocBoundedArray(StackSize * sizeof(int));

#ifdef PARISC
    // HP stack works from low addresses to high addresses
    // everyone else works the other way: from high addresses to low addresses
    stackTop = stack + 16;	// HP requires 64-byte frame marker
    stack[StackSize - 1] = STACK_FENCEPOST;
#endif

#ifdef SPARC
    stackTop = stack + StackSize - 96; 	// SPARC stack must contains at 
					// least 1 activation record 
					// to start with.
    *stack = STACK_FENCEPOST;
#endif 

#ifdef PowerPC // RS6000
    stackTop = stack + StackSize - 16; 	// RS6000 requires 64-byte frame marker
    *stack = STACK_FENCEPOST;
#endif 

#ifdef DECMIPS
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif

#ifdef SGIMIPS
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif

#ifdef ALPHA
    stackTop = stack + StackSize - 8;	// -8 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif


#ifdef x86
    // the x86 passes the return address on the stack.  In order for SWITCH() 
    // to go to ThreadRoot when we switch to this thread, the return addres 
    // used in SWITCH() must be the starting address of ThreadRoot.
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *(--stackTop) = (int) ThreadRoot;
    *stack = STACK_FENCEPOST;
#endif
    
#ifdef PARISC
    machineState[PCState] = PLabelToAddr(ThreadRoot);
    machineState[StartupPCState] = PLabelToAddr(ThreadBegin);
    machineState[InitialPCState] = PLabelToAddr(func);
    machineState[InitialArgState] = arg;
    machineState[WhenDonePCState] = PLabelToAddr(ThreadFinish);
#else
    machineState[PCState] = ThreadRoot;
    machineState[StartupPCState] = ThreadBegin;
    machineState[InitialPCState] = func;
    machineState[InitialArgState] = arg;
    machineState[WhenDonePCState] = ThreadFinish;
#endif
}

#ifdef USER_PROGRAM
#include "machine.h"

//----------------------------------------------------------------------
// Thread::SaveUserState
//	Save the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine saves the former.
//----------------------------------------------------------------------

void
Thread::SaveUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
	userRegisters[i] = kernel->machine->ReadRegister(i);
}

//----------------------------------------------------------------------
// Thread::RestoreUserState
//	Restore the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine restores the former.
//----------------------------------------------------------------------

void
Thread::RestoreUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
	kernel->machine->WriteRegister(i, userRegisters[i]);
}

#endif

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

static void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        kernel->currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// Thread::SelfTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
Thread::SelfTest()
{
    DEBUG(dbgThread,("Entering Thread::SelfTest\n"));

    Thread *t = new Thread("forked thread");

    t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    SimpleThread(0);
}

#ifdef PP1_3  
//----------------------------------------------------------------------
// Thread::Join
//	This routine causes the currently running thread to block until
// the joined thread has been completed.
//----------------------------------------------------------------------
/*void Thread::Join(void)
{
  ASSERT(kernel->currentThread == parent); // can only call join from parent
  if (!Finished)
    semFinished->P();		// Wait until the thread is finished
}*/

//----------------------------------------------------------------------
// Thread::AddChild
//	This routine adds a child to the list of children.  This list will
// be used to signal to each child that it can go ahead and kill itself
// when this thread dies
//----------------------------------------------------------------------
void Thread::AddChild(Thread *child)
{
  childrenList->Append(child);
}

//----------------------------------------------------------------------
// Thread::CanBeKilled
//	This routine is used to release a thread to  kill itself
//----------------------------------------------------------------------
void Thread::CanBeKilled(void)
{
   ASSERT(kernel->currentThread == parent); // only a parent can release a
   // child
   OkToKill=1;
}


//----------------------------------------------------------------------
// Thread::SelfTestJoin
//	This tests the join functionality.
//----------------------------------------------------------------------
void Thread::SelfTestJoin(void)
{
  

  Thread *t = new Thread("selftestjoinA");
  printf("\nSelf testing the Join method\n");
  printf("SelfTestJoin is forking off thread SelfTestJoinA.\n");
  t->Fork((VoidFunctionPtr) SelfTestJoinA, (void *) this);
  printf("SelfTestJoin is JOINing thread SelfTestJoinA.\n");
  t->Join();
  printf("SelfTestJoin has passed the JOINing point.\n");
  printf("SelfTestJoin trying to JOIN again.\n");
  t->Join();
  printf("SelfTestJoin as passed the JOINing point again.\n");
  printf("Self test of Join passed\n");
}

//----------------------------------------------------------------------
// Thread::SelfTestJoinA
//	This helps test the join functionality.
//----------------------------------------------------------------------
void Thread::SelfTestJoinA(void)
{
  printf("SelfTestJoinA is running now");
  for (int i=0; i<100; i++)
    printf(".");
  printf("\nSelfTestJoinA is now Finished.\n");
}

#endif // PP1_3

#ifdef PP1_5
//----------------------------------------------------------------------
// Thread::SetPriority
//	Sets the threads priority by taking off the ready list, setting
// the new priority and then re-inserting the thread on the ready list.
//----------------------------------------------------------------------
void Thread::SetPriority(int newPriority)
{
  if (kernel->scheduler->IsInList(this)) {
    kernel->scheduler->RemoveFromList(this); 
    priority=newPriority;
    kernel->scheduler->ReadyToRun(this);
  }
}

void Thread::SelfTestPriorityInversion(void) {
  semSynch = new Semaphore("SynchingSemaphore",0);
  lockThing = new Lock("SelfTestPriorityLock");

  Thread *thread1 = new Thread("SelfTestPriorityThread1");
  Thread *thread2 = new Thread("SelfTestPriorityThread2");

  printf("\n Self testing priority inversion\n");
  thread1->Fork( (VoidFunctionPtr) PriInvHelper1, (void *) this);
  semSynch->P(); 
  thread2->priority=55;
  thread2->Fork( (VoidFunctionPtr) PriInvHelper2, (void *) this);
  semSynch->P(); 
}

void Thread::PriInvHelper1(void) {
  Lock *mylock= new Lock("MyPrivateLock");
  
  printf("Inside priority inverstion helper1 (PIH1)\n");
  lockThing->Acquire();
  semSynch->V();
  for (int i=0; i < 20; i++) {
    printf("(PIH1)");
    mylock->Acquire();
    mylock->Release();
  }
  lockThing->Release();
  printf("Leaving priority inverstion helper1 (PIH1)\n");
}

void Thread::PriInvHelper2(void) {
  Lock *mylock= new Lock("MyPrivateLock");

  
  printf("Inside priority inversion helper2 (PIH2)\n");
  printf("(PIH2) trying to get lock that (PHI1) has.\n");
  lockThing->Acquire();
  for (int i=0; i < 20; i++) {
    printf("(PIH2)");
    mylock->Acquire();
    mylock->Release();
  }
  lockThing->Release();
  printf("Leaving priority inversion helper2 (PIH2)\n");
  semSynch->V();
}

#endif // PP1_5

//----------------------------------------------------------------------
// Thread::DeadLockTest
// 	It is used with three threads acquiring the same locks.  
//	thread2 acquire lock2 and waits on the condition variable.
//      thread3 acquire lock3 and waits on the condition variable.
//      thread4 acquire lock4, broadcast on the condition variable and
//      try to acquire Lock2.
//      thread2 and thread3 start again trying to acquire respectively 
//      lock3 and lock4
//      the deadlock may happen differently according to the yielding
//      between  the threads. This variations are out of our control.
//----------------------------------------------------------------------

#ifdef PP1_7
void
Thread::DeadLockTest()
{
  this->MyStruct.L1->Acquire();
  if (this->getID() != 4){
    this->MyStruct.L3->Acquire();
    this->MyStruct.Sem->Wait(this->MyStruct.L3);
    this->MyStruct.L3->Release();
  }
  if (this->getID() == 4 ) {
    this->MyStruct.L3->Acquire();
    this->MyStruct.Sem->Broadcast( this->MyStruct.L3);
    this->MyStruct.L3->Release();
    this->Yield();
  }
  this->MyStruct.L2->Acquire();
}

void
Thread::SelfTestDeadLock()
{
    DEBUG(dbgThread,("Entering Thread::SelfTest\n"));


    // three threads that will acquire some locks
    Thread *t2 = new Thread("forked thread2");
    printf("thread %d is born (t2)\n", t2->getID());
    Thread *t3 = new Thread("forked thread3");
    printf("thread %d is born (t3)\n", t3->getID());
    Thread *t4 = new Thread("forked thread4");
    printf("thread %d is born (t4)\n", t4->getID());

    // the locks to create deadlocks
    Lock *LT2 = new Lock("L1");
    Lock *LT3 = new Lock("L2");
    Lock *LT4 = new Lock("L3");

    // the lock needed by the condition variable
    Lock *LTC = new Lock("LC");

    // the condition variable used to block some of the threads
    Condition *MySem = new Condition("MySemaphore");
    
    // the threads have a member structure storing pointers to the lock
    // and the condition variable, so we can manipulate them after the fork. 
    t2->MyStruct.L1 = LT2;
    t2->MyStruct.L2 = LT3;
    t2->MyStruct.L3 = LTC;
    t2->MyStruct.Sem = MySem;

    t3->MyStruct.L1 = LT3;
    t3->MyStruct.L2 = LT4;
    t3->MyStruct.L3 = LTC;
    t3->MyStruct.Sem = MySem;

    t4->MyStruct.L1 = LT4;
    t4->MyStruct.L2 = LT2;
    t4->MyStruct.L3 = LTC;
    t4->MyStruct.Sem = MySem;

    // forking the threads to start the deadlock
    t2->Fork((VoidFunctionPtr) DeadLockTest, (void *) t2);
    t3->Fork((VoidFunctionPtr) DeadLockTest, (void *) t3);
    t4->Fork((VoidFunctionPtr) DeadLockTest, (void *) t4);
    
    // The main thread must continue to run otherwise the lock that are
    // declared as local variables would disapear.
    SimpleThread(0);

}


int 
Thread::getID() 
{ 
  return (ID);
}

void 
Thread::setLockWaited(Lock *NewWaitedLock)
{
  LockWaited = NewWaitedLock;
}

Lock * 
Thread::getLockWaited()
{
  return (LockWaited);
}

void Thread::DeadLockCheck(Thread* InitThread, Lock* CurrentLock, Thread* OtherThread, Lock* OtherLockWaited)
{
  if (OtherLockWaited == NULL) {
    return;
  } else if (OtherLockWaited->getLockHolderID() == InitThread->getID())
    {	printf("Thread%d is waiting on Lock%d, held by Thread%d waiting on Lock%d, held by thread%d\n", 
	       ID, 
	       CurrentLock->getLockID(), 
	       OtherThread->getID(), 
	       OtherThread->getLockWaited()->getLockID(), 
	       InitThread->getID());
      printf("Deadlock detected\n");
      return;
    } else {
	printf("Thread%d is waiting on Lock%d, held by Thread%d waiting on Lock%d\n", 
	       InitThread->getID(), 
	       CurrentLock->getLockID(), 
	       OtherThread->getID(), 
	       OtherThread->getLockWaited()->getLockID());
	OtherThread->DeadLockCheck (InitThread, OtherLockWaited,
				    OtherLockWaited->getLockHolder(), 
				    OtherLockWaited->getLockHolder()->getLockWaited());
    }
  return;
}

#endif


#ifdef PP1_2
void
Thread::MBoxSender()
{
    int *test;
    test = this->Mstruct.message;
    this->Mstruct.mbox->Send(3);
    printf("message send\n");
}


void
Thread::MBoxReceiver()
{

  int *test;
  this->Mstruct.mbox->Receive(test);
  printf("The message is %d\n", test);
  //kernel->currentThread->Yield();
}

//#endif

//----------------------------------------------------------------------
// Thread::SelfTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
Thread::SelfTestMailBoxes()
{
    DEBUG(dbgThread,("Entering Thread::SelfTest\n"));

   

    Mailbox *m = new Mailbox();
    Thread *t1= new Thread ("Threadt1");
    Thread *t2= new Thread ("Threadt2");
    
    t1->Mstruct.mbox = m;
    t2->Mstruct.mbox = m;

    //

    printf("currentThread before fork is: %s (%x)", kernel->currentThread->getName());
    t1->Fork((VoidFunctionPtr) MBoxSender, (void *) t1);
    t2->Fork((VoidFunctionPtr) MBoxReceiver, (void *) t2);

    Thread *t = new Thread("forked thread");
    t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    SimpleThread(0);
}

#endif // PP1_2


#ifdef PP2_1
//----------------------------------------------------------------------
// Thread::SetExitCode
// 	Sets exit code to pass on to Parent upon join
//---------------------------------------------------------------------
void Thread::SetExitCode(int code)
{
  exitCode=code;
}

//----------------------------------------------------------------------
// Thread::GetExitCode
// 	Gets exit code to pass on to Parent upon join
//---------------------------------------------------------------------
int Thread::GetExitCode(void)
{
  return exitCode;
}
#endif // PP2_1

#ifdef PP4
//----------------------------------------------------------------------
// Thread::GetCurDirectory
// 	returns the current directory of the thread
//---------------------------------------------------------------------
OpenFile *Thread::GetCurDirFile(){
  OpenFile *returnValue;
  returnValue = new OpenFile(curDirFile->fileHeaderSector);
  return returnValue;
}
#endif // PP4

#ifdef PP4
//----------------------------------------------------------------------
// Thread::SetCurDirectory
// 	changes the current directory of the thread to the given 
//      directory
//---------------------------------------------------------------------
void Thread::SetCurDirectory(OpenFile *NewDir){
  if (NewDir != NULL) {
/*   if (curDirFile != NULL)
      delete curDirFile;*/
   curDirFile = NewDir;
  }
  return;
}
#endif // PP4
