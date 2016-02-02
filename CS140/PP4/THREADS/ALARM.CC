// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "alarm.h"
#include "main.h"
#ifdef PP1_2
#include "list.h"
#endif // PP1_2
//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to 
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom)
{
    timer = new Timer(doRandom, this);
}

//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice 
//      if we're currently running something (in other words, not idle).
//	Also, to keep from looping forever, we check if there's
//	nothing on the ready list, and there are no other pending
//	interrupts.  In this case, we can safely halt.
//----------------------------------------------------------------------

void 
Alarm::CallBack() 
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();
    
    if (status == IdleMode) {	// is it time to quit?
        if (!interrupt->AnyFutureInterrupts()) {
	    timer->Disable();	// turn off the timer
	}
    } else {
#ifdef PP1_6
      // decrement time remaing for thread by one quantum
      kernel->currentThread->timeleft--; 
      // if quantum has expired then preempt the thread
      if (kernel->currentThread->timeleft == 0)
	interrupt->YieldOnReturn();
      // every 10 TimerTicks run the update routine on the ready list
      if ((numTimerTicks % 10) == 0) {
	numTimerTicks = 0;
	kernel->scheduler->DoUpdate();
      }
      else numTimerTicks++;

#else 		// there's someone to preempt
	interrupt->YieldOnReturn();
#endif // PP1_6
    }
}

#ifdef PP1_4

void Alarm::WaitUntil(int x)
{
  

  printf("the Alarm::WaitUntil calling thread is: %s (%x) \n",kernel->currentThread->getName());

  //I am going to hack this - since I was stupid in not getting the sorted list to work.

  // get the current time
  printf("the current time is: %d ==\n", kernel->stats->systemTicks);
  
  //add the current time to the int x we received

  wakeuptime = kernel->stats->totalTicks + x;
  printf("the wakeup time is: %d ==\n", wakeuptime);
  //add the thread onto the list with a wakeup time
  
  
  //insert the thread intot the alarm list

  alarmlist = new List<Thread *>;
  wakeuplist = new List<int>;
  
  //do thread switch

  Interrupt *interrupt = kernel->interrupt;
  Thread *currentThread = kernel->currentThread;
  
  //disable interrupts
  
  IntStatus oldLevel = interrupt->SetLevel(IntOff);
  
  alarmlist->Append(currentThread);
  currentThread->Sleep(FALSE);
  wakeuplist->Append(wakeuptime);
  
  (void) interrupt->SetLevel(oldLevel);




  //q) how do we wake up the slumbering thread, this function is not 
  //called on all the timer ticks, we can implement a 
  // change in interrupt.cc or scheduler to look at this queue also.
  
  //1) Hack to make alarmlist public so the scheduler can schedule this
  //   we can hack the scheduler find next to run, doing the test and evaluate
  //   for future threads to be woken up. 
  //   
  //2) add a pending interrupt to the interrupt list to wake up a
  //   scheduling function for this list, insert into interrupt pending
  //   queue
  //
  //3) it is a hack to throw a thread onto a ready queue to be scheduled,
  //   we should really do a context switch sometime before the thread
  //   is to be scheduled to help guarantee the accuracy of the timer dela
 
}


bool Alarm::alarmlistempty()
{

 if (wakeuplist->IsEmpty)
   {
     return true;
   }
}



bool Alarm::timeisup()
{

  int currenttime = kernel->stats->totalTicks;
  int test;
  test = wakeuplist->RemoveFront();
  if (test < currenttime)
    {
      wakeuplist->Append(test); //insert time back 
      return FALSE;
    }
  if (test > currenttime)
    {
      return true; //queue is popped automatically 
    }
}


void Alarm::Addthread()
{

  //add thread from alarmlist to readyqueue
  kernel->scheduler->ReadyToRun(alarmlist->RemoveFront());

}
  

void Alarm::AlarmSelfTest()
{



}


#endif
