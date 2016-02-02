// pbitmap.h 
//	Data structures defining a "persistent" bitmap -- a bitmap
//	that can be stored and fetched off of disk
//
// Copyright (c) 1992,1993,1995 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#ifndef PBITMAP_H
#define PBITMAP_H

#include "copyright.h"
#include "bitmap.h"
#include "openfile.h"

#ifdef PP4
#include "constants.h"
class Lock;

class SBitmap {
  public:
    SBitmap(int numItems);	// Initialize a bitmap, with "numItems" bits
				// initially, all bits are cleared.
    ~SBitmap();			// De-allocate bitmap
    
    void Mark(int which);   	// Set the "nth" bit
    void Clear(int which);  	// Clear the "nth" bit
    bool Test(int which) const;	// Is the "nth" bit set?
    int FindAndSet();         // Return the # of a clear bit, and as a side
				// effect, set the bit. 
				// If no bits are clear, return -1.
    int NumClear() const;	// Return the number of clear bits

    void Print() const;		// Print contents of bitmap
    void SelfTest();		// Test whether bitmap is working
    
  protected:
    int numBits;		// number of bits in the bitmap
    int numWords;		// number of words of bitmap storage
				// (rounded up if numBits is not a
				//  multiple of the number of bits in
				//  a word)
    unsigned int *map;		// bit storage


    Lock *lock;

};
#endif

// The following class defines a persistent bitmap.  It inherits all
// the behavior of a bitmap (see bitmap.h), adding the ability to
// be read from and stored to the disk.

class PersistBitMap : public SBitmap {
  public:
    PersistBitMap(OpenFile *f, int numItems); // initialize bitmap from disk

    ~PersistBitMap(); 			// deallocate bitmap
    void SetFile(OpenFile *f) { file = f; }
    void FetchFrom();
    void WriteBack(); 	// write bitmap contents to disk 
  private:
    OpenFile *file;
#ifdef PP4
    bool fetched;
#endif

};

#endif // PBITMAP_H
