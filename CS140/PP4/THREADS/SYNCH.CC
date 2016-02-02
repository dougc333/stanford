// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables.
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Once we'e implemented one set of higher level atomic operations,
// we can implement others using that implementation.  We illustrate
// this by implementing locks and condition variables on top of 
// semaphores, instead of directly enabling and disabling interrupts.
//
// Locks are implemented using a semaphore to keep track of
// whether the lock is held or not -- a semaphore value of 0 means
// the lock is busy; a semaphore value of 1 means the lock is free.
//
// The implementation of condition variables using semaphores is
// a bit trickier, as explained below under Condition::Wait.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "synch.h"
#include "main.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
#ifdef PP1_5
    queue = new SortedList<Thread *>(PriorityComp);
#else
    queue = new List<Thread *>;
#endif
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    Interrupt *interrupt = kernel->interrupt;
    Thread *currentThread = kernel->currentThread;
    
    // disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	
    
    while (value == 0) { 		// semaphore not available
#ifdef PP1_5
      queue->Insert(currentThread);	// so go to sleep
#else
	queue->Append(currentThread);	// so go to sleep
#endif // PP1_5
	currentThread->Sleep(FALSE);
    } 
    value--; 			// semaphore available, consume its value
   
    // re-enable interrupts
    (void) interrupt->SetLevel(oldLevel);	
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that interrupts
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Interrupt *interrupt = kernel->interrupt;
    
    // disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	
 
#ifdef PP1_5
    value++;
#endif // PP1_5
  
    if (!queue->IsEmpty()) {  // make thread ready.
	kernel->scheduler->ReadyToRun(queue->RemoveFront());
    }
#ifndef PP1_5
    value++;
#endif // PP1_5
    
    // re-enable interrupts
    (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Semaphore::SelfTest, SelfTestHelper
// 	Test the semaphore implementation, by using a semaphore
//	to control two threads ping-ponging back and forth.
//----------------------------------------------------------------------

static Semaphore *ping;
static void
SelfTestHelper (Semaphore *pong) 
{
    for (int i = 0; i < 10; i++) {
        ping->P();
	pong->V();
    }
}

void
Semaphore::SelfTest()
{
    Thread *helper = new Thread("ping");

    ASSERT(value == 0);		// otherwise test won't work!
    ping = new Semaphore("ping", 0);
    helper->Fork((VoidFunctionPtr) SelfTestHelper, this);
    for (int i = 0; i < 10; i++) {
        ping->V();
	this->P();
    }
    delete ping;
}

//----------------------------------------------------------------------
// Lock::Lock
// 	Initialize a lock, so that it can be used for synchronization.
//	Initially, unlocked.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Lock::Lock(char* debugName)
{
    name = debugName;
    semaphore = new Semaphore("lock", 1);  // initially, unlocked
    lockHolder = NULL;
#ifdef PP1_5
    priorityInversion = 0;
    origPriority = 0;
#endif PP1_5
#ifdef PP1_7
    ID = kernel->scheduler->genLockID();
    LockHolderID = -1;
#endif
}

//----------------------------------------------------------------------
// Lock::~Lock
// 	Deallocate a lock
//----------------------------------------------------------------------
Lock::~Lock()
{
    delete semaphore;
}

//----------------------------------------------------------------------
// Lock::Acquire
//	Atomically wait until the lock is free, then set it to busy.
//	Equivalent to Semaphore::P(), with the semaphore value of 0
//	equal to busy, and semaphore value of 1 equal to free.
//----------------------------------------------------------------------

void Lock::Acquire()
{
#ifdef PP1_5
    Interrupt *interrupt = kernel->interrupt;
    Thread *currentThread = kernel->currentThread;
    
    // disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	

    if (lockHolder!=NULL)  // do we have lock holder
      if (kernel->scheduler->IsInList(lockHolder)) // is he in the readylist
	if(currentThread->priority > lockHolder->priority) { // does the
	  // look holder have a lower priority than the lock wanter
	  if (!priorityInversion) { // Only save the original priority once
	    origPriority=lockHolder->priority;
	    priorityInversion=1;
	  }
	  // set the lock holder new priority
	  lockHolder->SetPriority(currentThread->priority); 
	}
    
    // re-enable interrupts
    (void) interrupt->SetLevel(oldLevel);	
#endif // PP1_5
#ifdef PP1_7
if (kernel->currentThread->getID() == 2 || 
    kernel->currentThread->getID() == 3 || 
    kernel->currentThread->getID() == 4) 
{
  printf("Thread%d is acquiring Lock%d\n",
	 kernel->currentThread->getID(), ID);
}
    kernel->currentThread->setLockWaited(this);

// if the lock has no holder, a deadlock is impossible
 if (lockHolder != NULL)
   kernel->currentThread->DeadLockCheck(kernel->currentThread, 
					this, 
					lockHolder, 
					(*lockHolder).getLockWaited());
#endif
    semaphore->P();
    lockHolder = kernel->currentThread;
#ifdef PP1_7  

    // The lock is now owned by the thread.
    LockHolderID = kernel->currentThread->getID();
    if (kernel->currentThread->getID() == 2 || 
	kernel->currentThread->getID() == 3 || 
	kernel->currentThread->getID() == 4) 
      {
	printf("Thread%d has acquired Lock%d\n", 
	     kernel->currentThread->getID(), ID);
      }
    kernel->currentThread->setLockWaited(NULL); // if the thread has acquired the lock, it doesn't
    	// wait for it anymore.
#endif  
}

//----------------------------------------------------------------------
// Lock::Release
//	Atomically set lock to be free, waking up a thread waiting
//	for the lock, if any.
//	Equivalent to Semaphore::V(), with the semaphore value of 0
//	equal to busy, and semaphore value of 1 equal to free.
//
//	By convention, only the thread that acquired the lock
// 	may release it.
//---------------------------------------------------------------------

void Lock::Release()
{
  ASSERT(IsHeldByCurrentThread());
#ifdef PP1_5
    Interrupt *interrupt = kernel->interrupt;
    Thread *currentThread = kernel->currentThread;
    
    // disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	
    
    // if we had priority inversion then go back to the original priority
    // and yield the processor.
    if (priorityInversion) {
      lockHolder->priority=origPriority;
      priorityInversion=0;
      origPriority=0;
    }
    // enable interrupts
    (void) interrupt->SetLevel(oldLevel);	
#endif // PP1_5
#ifdef PP1_7
    if (kernel->currentThread->getID() == 2 || 
	kernel->currentThread->getID() == 3 || 
	kernel->currentThread->getID() == 4) 
      {
	printf("Thread%d is releasing Lock%d\n", 
	       kernel->currentThread->getID(), ID);
      }
#endif
  lockHolder = NULL;
  semaphore->V();
#ifdef PP1_7
    LockHolderID = -1;
if (kernel->currentThread->getID() == 2 || 
    kernel->currentThread->getID() == 3 || 
    kernel->currentThread->getID() == 4) 
{
  printf("Thread%d has released Lock%d\n", kernel->currentThread->getID(),
	 ID);
}
#endif 
}

//----------------------------------------------------------------------
// Lock::IsHeldByCurrentThread
//       Return true if the current thread holds this lock.
//----------------------------------------------------------------------

bool Lock::IsHeldByCurrentThread()
{
    return (lockHolder == kernel->currentThread); 
}

#ifdef PP1_7
int Lock::getLockHolderID()
{
  return (lockHolder->getID());
}

int Lock::getLockID()
{
  return ID;
}
Thread* Lock::getLockHolder()
{
  return lockHolder;
}
#endif

#ifndef PP1_1
//----------------------------------------------------------------------
// Condition::Condition
// 	Initialize a condition variable, so that it can be 
//	used for synchronization.  Initially, no one is waiting
//	on the condition.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Condition::Condition(char* debugName)
{
    name = debugName;
    waitQueue = new List<Semaphore *>;
}

//----------------------------------------------------------------------
// Condition::Condition
// 	Deallocate the data structures implementing a condition variable.
//----------------------------------------------------------------------

Condition::~Condition()
{
    delete waitQueue;
}

//----------------------------------------------------------------------
// Condition::Wait
// 	Atomically release monitor lock and go to sleep.
//	Our implementation uses semaphores to implement this, by
//	allocating a semaphore for each waiting thread.  The signaller
//	will V() this semaphore, so there is no chance the waiter
//	will miss the signal, even though the lock is released before
//	calling P().
//
//	Note: we assume Mesa-style semantics, which means that the
//	waiter must re-acquire the monitor lock when waking up.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock) 
{
     Semaphore *waiter;
    
     ASSERT(conditionLock->IsHeldByCurrentThread());

     waiter = new Semaphore("condition", 0);
     waitQueue->Append(waiter);
     conditionLock->Release();
     waiter->P();
     conditionLock->Acquire();
     delete waiter;
}

//----------------------------------------------------------------------
// Condition::Signal
// 	Wake up a thread waiting on this condition, if any.
//
//	Note: we assume Mesa-style semantics, which means that the
//	signaller doesn't give up control immediately to the thread
//	being woken up (unlike Hoare-style).
//
//	Also note: we assume the caller holds the monitor lock
//	(unlike what is described in Birrell's paper).  This allows
//	us to access waitQueue without disabling interrupts.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock)
{
    Semaphore *waiter;
    
    ASSERT(conditionLock->IsHeldByCurrentThread());
    
    if (!waitQueue->IsEmpty()) {
        waiter = waitQueue->RemoveFront();
	waiter->V();
    }
}

//----------------------------------------------------------------------
// Condition::Broadcast
// 	Wake up all threads waiting on this condition, if any.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock) 
{
    while (!waitQueue->IsEmpty()) {
        Signal(conditionLock);
    }
}

#else
// Enable code for problem one of PP1
//----------------------------------------------------------------------
// Condition::Condition
// 	Initialize a condition variable, so that it can be 
//	used for synchronization.  Initially, no one is waiting
//	on the condition.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Condition::Condition(char* debugName)
{
    name = debugName;
    waitQueue = new List<Thread *>;
}

//----------------------------------------------------------------------
// Condition::Condition
// 	Deallocate the data structures implementing a condition variable.
//----------------------------------------------------------------------

Condition::~Condition()
{
    delete waitQueue;
}

//----------------------------------------------------------------------
// Condition::Wait
// 	Atomically release monitor lock and go to sleep.
//	Semephores are not used in this implementation.  Rather threads
//	placed on a waiting list.  The when the waiting thread is
//	awakened, it proceeds to re-acquire the lock. All this is done
//	atomically. 
//
//	Note: we assume Mesa-style semantics, which means that the
//	waiter must re-acquire the monitor lock when waking up.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock) 
{
  Interrupt *interrupt = kernel->interrupt;
  Thread *currentThread = kernel->currentThread;
  
  ASSERT(conditionLock->IsHeldByCurrentThread());

  // disable interrupts
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	
  
  // the thread is queued and put to sleep while it waits for the
  // condition to be meet.  In the mean time the lock is released and is
  // later aquired after being wokenup.

  waitQueue->Append(currentThread);
  conditionLock->Release();
  currentThread->Sleep(FALSE);
  conditionLock->Acquire();

  // re-enable interrupts
  (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Condition::Signal
// 	Wake up a thread waiting on this condition, if any.
//
//	Note: we assume Mesa-style semantics, which means that the
//	signaller doesn't give up control immediately to the thread
//	being woken up (unlike Hoare-style).
//
//	All this is done atomically to insure that the queue is not
//	emptied before proceeding to wakeup the waiting thread.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock)
{
  Interrupt *interrupt = kernel->interrupt;

  // disable interrupts
  IntStatus oldLevel = interrupt->SetLevel(IntOff);	  

  ASSERT(conditionLock->IsHeldByCurrentThread());

  // Go ahead let a process on the queue proceed now that the condition
  // has been met
  if (!waitQueue->IsEmpty()) {
    kernel->scheduler->ReadyToRun(waitQueue->RemoveFront());
  }

  // re-enable interrupts
  (void) interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// Condition::Broadcast
// 	Wake up all threads waiting on this condition, if any.
//
//	"conditionLock" -- lock protecting the use of this condition
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock) 
{
  while (!waitQueue->IsEmpty()) {
    Signal(conditionLock);
  }
}

//---------------------------------------------------------------------
// Condition::SelfTest
//	Fires off 2 threads, P1 and P2, that have to run in a particular
// order.  The condition variable prevents the second thread from run
// until the first thread is complete.  Thus the condition is that P! and
// P2 have a critical sections that are mutually exclusive.  In addtion P2
// cannot run its critical section until P1 has finished its critcal and
// remainder sections.
//-----------------------------------------------------------------------
void Condition::SelfTest(void)
{
  doneTesting=0;
  
  Thread *h1 = new Thread("helper1");
  Thread *h2 = new Thread("helper2");

  doneTesting = new Semaphore("SelfTestSema",0);
  selfTestCondition = new Condition("SelfTestCondition");
  selfTestLock = new Lock("SelfTestLock");

  printf("Self testing condition.\n");
  

  h2->Fork((VoidFunctionPtr) Helper2, (void *) this); 
  doneTesting->P();
  h1->Fork((VoidFunctionPtr) Helper1, (void *) this);
  doneTesting->P();

  delete doneTesting;
  delete selfTestCondition;
  delete selfTestLock;
   
}
 
void Condition::Helper1(void)
{
  printf("Helper 1 is getting a lock.\n");
  selfTestLock->Acquire();
  printf("Helper 1 is in its critical section.\n");
  selfTestCondition->Signal(selfTestLock);
  printf("Helper 1 has told helper2 it can start.\n");
  selfTestLock->Release();
  printf("Helper 1 is in its remainder section.\n");
}

void Condition::Helper2(void)
{
  printf("Helper 2 is getting a lock.\n");
  selfTestLock->Acquire();
  printf("Helper 2 is trying to start its critical section.\n");
  doneTesting->V();
  selfTestCondition->Wait(selfTestLock);
  printf("Helper 2 is in its critical section.\n");
  selfTestLock->Release();
  printf("Helper 2 is in its remainder section.\n");
  doneTesting->V();
}
#endif
