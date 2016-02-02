// diskcache.h

#ifndef DISKCACHE_H
#define DISKCACHE_H

#include "hash.h"
#include "disk.h"
#include "circlist.h"

class Semaphore;

extern "C" void bcopy(const void *, void *, unsigned int);

#define MaxCacheSize 64   // can only hold 64 disk sectors
class CacheEntry {
 public:
  CacheEntry(int newSector, char *newData, 
	     bool newDirty, bool newUsed);	// Create New Entry
  ~CacheEntry();				// Destroy Entry
  
  void Update(char *newData);      // Updates sector data
  void SetDirty(bool d);
  bool IsDirty();
  int GetSector();
  char *GetData();
  bool IsUsed();
  void SetUsed(bool u);
 private:
  int sector;
  bool dirty;					// Indicates if the cache
  bool used;					// If sector has been used;
  char *data;					// Actual data bits of
						// cache sector
};


int cacheKey(CacheEntry *cacheEntry);		// Generates a Key
unsigned int cacheHashFunc(int key);		// Does Hashing function on key


class DiskCache {
 public:
  DiskCache( Disk *newDisk, Semaphore *newSemaphore);	// Create a disk cache
  ~DiskCache();					// Destroy the disk cache
  

  void ReadRequest(int sectorNum, char *into);
  void WriteRequest(int sectorNum, char *from);
  void Flush(void);  
  bool  Exists(int s);
  void  LoadFromDisk(int s);
 private:
  void LoadRequest(int newSector, char *newData, bool dirty, bool used);
  void MakeSpace(void);				// does LRU on cache
  // entries

  // request can be sent to cache at a time
  int curSize;					// current # of sectors in
						// cache	
  HashTable<int, CacheEntry *> *cache;		// Cache of sectors

  CircularList *cl;				// used to implement clock
						// algorithm;
  int cacheMiss;
  int cacheHit;
  
  Disk *disk;
  Semaphore *semaphore;
};


#endif // DISKCACHE_H


