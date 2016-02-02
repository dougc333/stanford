// memman.h
//
//  Data structure to keep track and manage the main memory for
//  multiprogramming.
//  
//

#ifndef MEMMAN_H
#define MEMMAN_H

#include "list.h"
#include "bitmap.h"
#include "synch.h"

class AddrSpace;

// The following class manages memory in a multiprogram environment



class MemoryManager {
 public:
  
  MemoryManager();			// Create a manager
  ~MemoryManager();			// Destroy the manager

  // Allocates memory for an address space
  bool SetupAddressSpace(AddrSpace *space, unsigned int sizePages);
  void RemoveAddressSpace(AddrSpace *space);  // deallocate address space

  // Copy stuff from user space to kernel space
  int UserToKernel(char *kernelBuf, int userVA, int bytesToCopy);
  // Copy stuff from kernel space to user space
  int KernelToUser(char *kernelBuf, int userVA, int bytesToCopy);
  int va2pa(int va, AddrSpace *space); // Translates from va to pa
 private:
  Bitmap *memoryMap; // marks the pages that in use and those that are free
  List<AddrSpace *>  *addrSpaceList; // List of address spaces we are managing
  Lock *pageTableSetupLock; // lock to setup critical section of memoryman
};



#endif // MEMMAN_H


