// memman.h
//
//  Data structure to keep track and manage the main memory for
//  multiprogramming.
//  
//

#ifndef MEMMAN_H
#define MEMMAN_H

#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include "synch.h"
#include "swapman.h"
class AddrSpace;
class SwapManager;
#ifdef PP3_1
class IPTEntry : public TranslationEntry {
 public:
  int pid; // process id
// physical page number already exist in TranslationEntry
  //int ppn; 
// virtual page number already exist in TranslationEntry
  //int vpn; 
};

int iptHashKey(IPTEntry *iptEntry); // Generates a Key
unsigned int iptHashFunc(int key);          // Does Hashing function on key
#define ERASETIME 5
#define LRU
//#define RANDOM

#endif // PP3_1


// The following class manages memory in a multiprogram environment



class MemoryManager {
 public:
  
  MemoryManager();			// Create a manager
  ~MemoryManager();			// Destroy the manager

  // Allocates memory for an address space
  bool SetupAddressSpace(AddrSpace *space, unsigned int sizePages);
  void RemoveAddressSpace(AddrSpace *space);  // deallocate address space

  // Copy stuff from user space to kernel space
  int UserToKernel(char *kernelBuf, unsigned long userVA, int bytesToCopy);
  // Copy stuff from kernel space to user space
  int KernelToUser(char *kernelBuf, unsigned long userVA, int bytesToCopy);
  int va2pa(int va, AddrSpace *space); // Translates from va to pa

#ifdef PP3
  HashTable<int, IPTEntry *> *ipt;  // Inverted page table is a hash
  SwapManager *swapMan;  // swap space 

  bool Locate (IPTEntry **iptEntry); // find an IPT entry
  void TLBUpdate(IPTEntry *iptEntry); // Second chance algo. to Update
  void TLBFlush(void); // clears TLB
  void IPTUpdate(void); // keeps TLB entries and flags concerrent with IPT
  int GetAndPlaceVA(unsigned long va, AddrSpace *space); // return pa while making
  // sure that pa is in physical memory
  void IPTClearUse(void);


  int PageIn(PTEntry *ptEntry);

  PTEntry *VMAllocate(int vpn);  // allocates a new page in vm for current
  // address space

  Bitmap *memoryMap; // marks the pages that in use and those that are free
#endif // PP3_1

  Lock *pageTableSetupLock; // lock to setup critical section of memoryman

#ifdef PP4
  void CreateSwap(); //create the sap file
#endif //PP4

 private:
  List<AddrSpace *>  *addrSpaceList; // List of address spaces we are managing

#ifdef PP3
  Lock *tlbLock; // locks all activity involving the tlb
  int curTLBEntry; // indicates the next TLB entry to be victimized
#endif // PP3
};


#endif // MEMMAN_H


