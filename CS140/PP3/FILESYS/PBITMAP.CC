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

PersistBitMap::PersistBitMap(OpenFile *f, int numItems) : Bitmap(numItems)
{
  file = f;
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
    file->ReadAt((char *)map, numWords * sizeof(unsigned), 0);
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
