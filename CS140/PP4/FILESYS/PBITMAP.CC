// pbitmap.c 
//	Routines to manage a persistent bitmap -- a bitmap that is
//	stored on disk.
//
// Copyright (c) 1992,1993,1995 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "pbitmap.h"

// These aren't needed until the FILESYS assignment

PersistBitMap::PersistBitMap(OpenFile *f, int numItems) : SBitmap(numItems)
{
  file = f;
#ifdef PP4
  fetched = FALSE;
#endif  
}

PersistBitMap::~PersistBitMap(void)
{
}

//----------------------------------------------------------------------
// BitMap::ToCanonical
// 	Initialize the contents of a bitmap from a Nachos file.
//
//	"file" is the place to read the bitmap from
//----------------------------------------------------------------------

void
PersistBitMap::FetchFrom(void) 
{
  if (!fetched) {
    file->ReadAt((char *)map, numWords * sizeof(unsigned), 0);
    fetched = TRUE;
  }
}

//----------------------------------------------------------------------
// PersistBitMap::WriteBack
// 	Store the contents of a bitmap to a Nachos file.
//
//	"file" is the place to write the bitmap to
//----------------------------------------------------------------------

void
PersistBitMap::WriteBack(void)
{
   file->WriteAt((char *)map, numWords * sizeof(unsigned), 0);
}

#ifdef PP4

#include "synch.h"

//----------------------------------------------------------------------
// BitMap::BitMap
// 	Initialize a bitmap with "numItems" bits, so that every bit is clear.
//	it can be added somewhere on a list.
//
//	"numItems" is the number of bits in the bitmap.
//----------------------------------------------------------------------

SBitmap::SBitmap(int numItems) 
{ 
  lock = new Lock("ASBitmapLock");
    int i;

    ASSERT(numItems > 0);

    numBits = numItems;
    numWords = divRoundUp(numBits, BitsInWord);
    map = new unsigned int[numWords];
    for (i = 0; i < numWords; i++) {
	map[i] = 0;		// initialize map to keep Purify happy
    }
    for (i = 0; i < numBits; i++) {
        Clear(i);
    }
    
}

//----------------------------------------------------------------------
// SBitmap::~SBitmap
// 	De-allocate a bitmap.
//----------------------------------------------------------------------

SBitmap::~SBitmap()
{ 

  delete lock ;

    delete map;
}

//----------------------------------------------------------------------
// SBitmap::Set
// 	Set the "nth" bit in a bitmap.
//
//	"which" is the number of the bit to be set.
//----------------------------------------------------------------------

void
SBitmap::Mark(int which) 
{ 

  //  lock->Acquire();

    ASSERT(which >= 0 && which < numBits);

    map[which / BitsInWord] |= 1 << (which % BitsInWord);

    ASSERT(Test(which));


    //lock->Release();


}
    
//----------------------------------------------------------------------
// SBitmap::Clear
// 	Clear the "nth" bit in a bitmap.
//
//	"which" is the number of the bit to be cleared.
//----------------------------------------------------------------------

void 
SBitmap::Clear(int which) 
{

  lock->Acquire();

    ASSERT(which >= 0 && which < numBits);

    map[which / BitsInWord] &= ~(1 << (which % BitsInWord));

    ASSERT(!Test(which));

  lock->Release();

}

//----------------------------------------------------------------------
// SBitmap::Test
// 	Return TRUE if the "nth" bit is set.
//
//	"which" is the number of the bit to be tested.
//----------------------------------------------------------------------

bool 
SBitmap::Test(int which) const
{

  // lock->Acquire();

    ASSERT(which >= 0 && which < numBits);
    
    if (map[which / BitsInWord] & (1 << (which % BitsInWord))) {

      // lock->Release();

	return TRUE;
    } else {

      //  lock->Release();

	return FALSE;
    }
}

//----------------------------------------------------------------------
// SBitmap::FindAndSet
// 	Return the number of the first bit which is clear.
//	As a side effect, set the bit (mark it as in use).
//	(In other words, find and allocate a bit.)
//
//	If no bits are clear, return -1.
//----------------------------------------------------------------------

int 
SBitmap::FindAndSet() 
{

  lock->Acquire();

    for (int i = 0; i < numBits; i++) {
	if (!Test(i)) {
	    Mark(i);

	    lock->Release();

	    return i;
	}
    }

    lock->Release();

    return -1;
}

//----------------------------------------------------------------------
// SBitmap::NumClear
// 	Return the number of clear bits in the bitmap.
//	(In other words, how many bits are unallocated?)
//----------------------------------------------------------------------

int 
SBitmap::NumClear() const
{

  lock->Acquire();

    int count = 0;

    for (int i = 0; i < numBits; i++) {
	if (!Test(i)) {
	    count++;
	}
    }

    lock->Release();

    return count;
}

//----------------------------------------------------------------------
// SBitmap::Print
// 	Print the contents of the bitmap, for debugging.
//
//	Could be done in a number of ways, but we just print the #'s of
//	all the bits that are set in the bitmap.
//----------------------------------------------------------------------

void
SBitmap::Print() const
{

  lock->Acquire();

    printf("SBitmap set:\n"); 
    for (int i = 0; i < numBits; i++) {
	if (Test(i)) {
            printf("%d, ", i);
	}
    }
    printf("\n"); 

    lock->Release();

}


//----------------------------------------------------------------------
// SBitmap::SelfTest
// 	Test whether this module is working.
//----------------------------------------------------------------------

void
SBitmap::SelfTest() 
{

  lock->Acquire();

    int i;
    
    ASSERT(numBits >= BitsInWord);	// bitmap must be big enough

    ASSERT(NumClear() == numBits);	// bitmap must be empty
    ASSERT(FindAndSet() == 0);
    Mark(31);
    ASSERT(Test(0) && Test(31));

    ASSERT(FindAndSet() == 1);
    Clear(0);
    Clear(1);
    Clear(31);

    for (i = 0; i < numBits; i++) {
        Mark(i);
    }
    ASSERT(FindAndSet() == -1);		// bitmap should be full!
    for (i = 0; i < numBits; i++) {
        Clear(i);
    }

    lock->Release();

}


#endif
