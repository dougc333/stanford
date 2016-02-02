// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include "synch.h"

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
#ifdef PP4
  sector = -1;
  realTableSize = new int;
  tableSize = new int;
  lock = new Lock("DirectoryLockNotFetched");

  *realTableSize = size + NumSpareEntries;
  table = new DirectoryEntry[*realTableSize];
#else
    table = new DirectoryEntry[size];
#endif
    *tableSize = size;
#ifdef PP4
    for (int i = 0; i < *realTableSize; i++) {
#else
    for (int i = 0; i < *tableSize; i++) {
#endif
	table[i].inUse = FALSE;
	table[i].isDir = FALSE;
    }
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
#ifdef PP4
  if (sector != -1) {
    kernel->dirManager->UnSubscribe(sector);
  } else {
    delete [] table;
    delete tableSize;
    delete realTableSize;
    delete lock;
  }

#else
    delete [] table;
#endif
} 

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
#ifdef PP4
  sector = file->fileHeaderSector;
  delete [] table;
  delete tableSize;
  delete realTableSize;
  delete lock;
  table = kernel->dirManager->Subscribe(&tableSize, &realTableSize, &lock,
					file);
#else
    (void) file->ReadAt((char *)table, *tableSize *
			sizeof(DirectoryEntry), 0);
#endif
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void
Directory::WriteBack(OpenFile *file)
{
#ifdef PP4
  lock->Acquire();
#endif
  (void) file->WriteAt((char *)table, *tableSize *
		       sizeof(DirectoryEntry), 0);
#ifdef PP4
  lock->Release();
#endif
}

//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::FindIndex(char *name)
{
  for (int i = 0; i < *tableSize; i++)
    if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
      return i;

  return -1;		// name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int
Directory::Find(char *name)
{
#ifdef PP4
  lock->Acquire();
#endif
    int i = FindIndex(name);

    if (i != -1) {
#ifdef PP4
      lock->Release();
#endif
      return table[i].sector;
    }
#ifdef PP4
    lock->Release();
#endif
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name, int newSector)
{ 
#ifdef PP4
  lock->Acquire();
#endif
  if (FindIndex(name) != -1) {
#ifdef PP4
    lock->Release();
#endif
    return FALSE;
  }
  for (int i = 0; i < *tableSize; i++)
    if (!table[i].inUse) {
      table[i].inUse = TRUE;
      strncpy(table[i].name, name, FileNameMaxLen); 
      table[i].sector = newSector;
#ifdef PP4
      table[i].isDir = FALSE;
      lock->Release();
#endif
      return TRUE;
    }

#ifdef PP4
    if (*tableSize == *realTableSize) GrowTable(); // see if we need to grow
    // the table
    
    // use one of the spare entries to hold the new entry
    table[*tableSize].inUse = TRUE;
    strncpy(table[*tableSize].name, name, FileNameMaxLen); 
    table[*tableSize].sector = newSector;
    table[*tableSize].isDir = FALSE;
    (*tableSize)++;
    lock->Release();
    return TRUE;
#else
    return FALSE;	// no space.  Fix when we have extensible files.
#endif
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name)
{ 
#ifdef PP4
  lock->Acquire();
#endif
  int i = FindIndex(name);
  
  if (i == -1) {
#ifdef PP4
    lock->Release();
#endif
    return FALSE; 		// name not in directory
  }
  table[i].inUse = FALSE;
#ifdef PP4
  lock->Release();
#endif
  return TRUE;	
}

//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
#ifdef PP4
  lock->Acquire();
#endif
  int countInUse=0;
  for (int i = 0; i < *tableSize; i++)
    if (table[i].inUse) {
      countInUse++;
      printf("%s", table[i].name);
      if (table[i].isDir) 
	printf("/\n");
      else
	printf("\n");
    }
  printf("\nTotal files and directories: %d\n", countInUse);
#ifdef PP4
  lock->Release();
#endif
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void
Directory::Print()
{
#ifdef PP4
  lock->Acquire();
#endif 
  FileHeader *hdr = new FileHeader;
  
  printf("Directory contents:\n");
  for (int i = 0; i < *tableSize; i++)
    if (table[i].inUse) {
      printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
      hdr->FetchFrom(table[i].sector);
      hdr->Print();
    }
  printf("\n");
  delete hdr;
#ifdef PP4
  lock->Release();
#endif
}
#ifdef PP4
//----------------------------------------------------------------------
// Directory::FindDir
// 	Look up directory name in directory, and return the disk sector number
//	where the directories's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the directory name to look up
//----------------------------------------------------------------------

int Directory::FindDir(char *name)
{
  lock->Acquire();
  int i = FindIndex(name);
  
  if ((i != -1) && (table[i].isDir)) {
    lock->Release();
    return table[i].sector;
  }
  lock->Release();
  return -1;
}

//----------------------------------------------------------------------
// Directory::MakeDir
// 	Add adirectory into the directory.  Return TRUE if successful;
//	return FALSE if the directory name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional directory names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::MakeDirEntry(char *name, int newSector)
{ 
  lock->Acquire();
  if (FindIndex(name) != -1) {
    lock->Release();
    return FALSE;
  }
  
  for (int i = 0; i < *tableSize; i++)
    if (!table[i].inUse) {
      table[i].inUse = TRUE;
      strncpy(table[i].name, name, FileNameMaxLen); 
      table[i].sector = newSector;
      table[i].isDir = TRUE;
      lock->Release();
      return TRUE;
    }
  
  if (*tableSize == *realTableSize) GrowTable(); // see if we need to grow
  // the table
  
  // use one of the spare entries to hold the new entry
  table[*tableSize].inUse = TRUE;
  strncpy(table[*tableSize].name, name, FileNameMaxLen); 
  table[*tableSize].sector = newSector;
  table[*tableSize].isDir = TRUE;
  (*tableSize)++;
  lock->Release();
  return TRUE;	// no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::RemoveDir
// 	Remove a dire name from the directory.  Return TRUE if successful;
//	return FALSE if the directory isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::RemoveDirEntry(char *name)
{ 
  lock->Acquire();
  int i = FindIndex(name);
  
  if (i == -1 && !table[i].isDir) {
    lock->Release();
    return FALSE; 		// name not in directory
  }
  table[i].inUse = FALSE;
  lock->Release();
  return TRUE;	
}

//----------------------------------------------------------------------
// Directory::GrowTable
// 	Makes a new table that is NumSpareEntries larger than *realTableSize
//      Copies the old table into the new table
//      Resets *realTableSize+=NumSpareEntries
//      Removes oldTable
//----------------------------------------------------------------------

void Directory::GrowTable(void)
{ 

  *realTableSize += NumSpareEntries;
  DirectoryEntry *newTable = new DirectoryEntry[*realTableSize];

  // copy over old stuff
  for (int i = 0; i < *tableSize; i++) {
	newTable[i].inUse = table[i].inUse;
	newTable[i].isDir = table[i].isDir;
	strncpy(newTable[i].name, table[i].name, FileNameMaxLen); 
  }
  // initialize new stuff
  for (int i = *tableSize; i < *realTableSize; i++) {
    	newTable[i].inUse = FALSE;
	newTable[i].isDir = FALSE;
  }

  delete [] table; // get rid of old table

  table = newTable; // assign table to new table
  kernel->dirManager->UpdateEntry(sector, table);
  
}
//----------------------------------------------------------------------
// Directory::GetNumEntries()
//        Returns the number of directory entries
//----------------------------------------------------------------------

int Directory::GetNumEntries(void)
{ 
  return *tableSize;
}

#endif
