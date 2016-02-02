// alarm.h 
//	Data structures for a software alarm clock.
//
//	We make use of a hardware timer device, that generates
//	an interrupt every X time ticks (on real systems, X is
//	usually between 0.25 - 10 milliseconds).
//
//	From this, we provide the ability for a thread to be
//	woken up after a delay; we also provide time-slicing.
//
//	NOTE: this abstraction is not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#ifndef ALARM_H
#define ALARM_H

#include "copyright.h"
#include "utility.h"
#include "callback.h"
#include "timer.h"
#include "constants.h"

#ifdef PP1_2
#include "list.h"
#endif

#ifdef PP1_4
class Thread;
#endif

// The following class defines a software alarm clock. 
class Alarm : public CallBackObj {
  public:
    Alarm(bool doRandomYield);	// Initialize the timer, and callback 
				// to "CallBack" every time slice.
    ~Alarm() { delete timer; }
#ifdef PP1_4
     delete alarmlist;
#endif
    void WaitUntil(int x);	// suspend execution until time > now + x
#ifdef PP1_4
    bool timeisup();
    bool alarmlistempty();
    void Addthread();           // add thread from alarm list to readylist


    void AlarmSelfTest();
    List<Thread *> *alarmlist;  //list of threads to store
    List<int> *wakeuplist;      // list of ints, refers to threads
#endif // PP1_4
  private:
#ifdef PP1_6
    int numTimerTicks;		// counts the num of elapse timer ticks
#endif // PP1_6
    Timer *timer;		// the hardware timer device

    void CallBack();		// called when the hardware
#ifdef PP1_4
    int wakeuptime; 
#endif // PP1_4				// timer generates an interrupt
};
#ifdef PP1_2
//the following class defines the alarmlist data structure

template <class T>
class AlarmList : public SortedList<T> {
  public:
    AlarmList(int (*comp)(T x, T y)) : List<T>() {compare=comp;}; 
    ~AlarmList() {};		// base class destructor called automatically

    void Insert(T item); 	// insert an item onto the list in sorted order

    // we inherit Remove() from list, remove the 
    // front item when the count int x=0
    //
    //    void SanityCheck() const;	// has this list been corrupted?
    //void SelfTest(T *p, int numEntries);
				// verify module is working

  private:
    int (*compare)(T x, T y);	// function for sorting list elements

    SortedList<T> *alarmlist;
    int delay;

};


#endif // PP1_2
#endif // ALARM_H
