// swapman.h
//
//  Data structure to keep track and manage the swap memory for
//  multiprogramming.
//  
//


#ifndef SWAPMAN_H
#define SWAPMAN_H

#ifdef PP3

#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include "synch.h"
#include "addrspace.h"
#include "filesys.h"
#include "noff.h"

// The following class manages swap space in a multiprogram environment



class SwapManager {
 public:
  
  SwapManager();			// Create a manager
  ~SwapManager();			// Destroy the manager

  bool SwapToMem(int sspn, int ppn);
  bool CopyMemToSwap(int vpn, int ppn);
  void FileToSwap(OpenFile *ExecFile, Segment *seg, AddrSpace *space);
  Bitmap *swapMap; // marks the pages that in use and those that are free
  OpenFile* swapFile;

 private:
  int maxSize;
  int size;

};

#endif // PP3

#endif // MEMMAN_H


