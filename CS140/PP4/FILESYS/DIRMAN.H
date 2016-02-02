// dirman.h
//   dirman manages the open file dirs in the system

#ifndef DIRMAN_H
#define DIRMAN_H

#include "hash.h"
#include "stdio.h"
#include "directory.h"
#include "openfile.h"

class Lock;
class DirectoryEntry;
class OpenFile;
class DirEntry {
 public:
  DirEntry(OpenFile *dirFile);
  ~DirEntry();

  int GetSector() {return sector;}
  Lock *GetLock() {return lock;}
  DirectoryEntry *GetDirectoryTable() {return table;}
  void SetDirectoryTable(DirectoryEntry *t) {table = t;}

  int *GetTableSize() {return &tableSize;};
  int *GetRealTableSize() {return &realTableSize;}

  int GetRefCount() {return refCount;}
  void IncRefCount() {refCount++;}
  int DecRefCount() {return  --refCount;}
 private:
  int sector;
  int tableSize;
  int realTableSize;
  Lock *lock;
  DirectoryEntry *table;

  int refCount;
};

int DirKey(DirEntry *dirEntry);		// Generates a Key
unsigned int DirHashFunc(int key);		// Does Hashing function on key



class DirectoryManager {
 public:
  DirectoryManager();
  ~DirectoryManager();

  DirectoryEntry *Subscribe(int **tableSize, int **realTableSize, Lock **dirLock, OpenFile *dirFile);
  void UnSubscribe(int sector);
  void UpdateEntry(int sector, DirectoryEntry *table);

 private:
  HashTable<int, DirEntry *> *registry;
  Lock *lock;
};

#endif // DIRMAN_H  
