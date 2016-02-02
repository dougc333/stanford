// headman.h
//   headman manages the open file heads in the system

#ifndef HEADMAN_H
#define HEADMAN_H

#include "hash.h"
#include "stdio.h"
#include "filehdr.h"

class Lock;

class HeadEntry {
 public:
  HeadEntry(int newSector);
  ~HeadEntry();

  int GetSector() {return sector;}
  Lock *GetLock() {return lock;}
  FileHeader *GetHeader() {return hdr;}
  bool IsAlreadyFetched() {return alreadyFetched;}
  void SetAlreadyFetched(bool a) {alreadyFetched = a;}

  int GetRefCount() {return refCount;}
  void IncRefCount() {refCount++;}
  int DecRefCount() {return  --refCount;}
 private:
  int sector;
  int refCount;
  bool alreadyFetched;
  Lock *lock;
  FileHeader *hdr;
};

int headKey(HeadEntry *headEntry);		// Generates a Key
unsigned int headHashFunc(int key);		// Does Hashing function on key



class HeadManager {
 public:
  HeadManager();
  ~HeadManager();

  FileHeader *Subscribe(int sector, Lock **hdrLock);
  void UnSubscribe(int sector);
  bool IsAlreadyFetched(int sector);
  void SetAlreadyFetched(int sector, bool a);

 private:
  HashTable<int, HeadEntry *> *registry;
  Lock *lock;
};

#endif // HEADMAN_H  
