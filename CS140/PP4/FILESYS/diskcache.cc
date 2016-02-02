// diskcache.cc 

#include "diskcache.h"
#include "main.h"


//----------------------------------------------------------------------
// DiskCache::DiskCache
//	Makes a new disk cache with maximum of size entries
//----------------------------------------------------------------------

DiskCache::DiskCache(Disk *newDisk, Semaphore *newSemaphore)
{ 
  DEBUG(dbgCache,("Cache is beging created\n"));
  disk = newDisk;
  semaphore = newSemaphore;
  curSize = 0;
  cacheHit = 0;
  cacheMiss = 0;
  cache = new HashTable<int, CacheEntry *>(cacheKey, cacheHashFunc);
  cl = new CircularList;
}

//----------------------------------------------------------------------
// DiskCache::~DiskCache
//----------------------------------------------------------------------

DiskCache::~DiskCache()
{
  DEBUG(dbgCache,("Cache is beging destroyed\n"));
  printf("\nDisk Cache: Hits = %d Miss = %d Closing size = %d\n",
	 cacheHit, cacheMiss, curSize);
  
  Flush();

  // empty cache
  HashIterator<int, CacheEntry *> iter(cache); 
  CacheEntry *entry;

  for (; !iter.IsDone(); iter.Next()) {
    entry = iter.Item();
    cache->Remove(entry->GetSector());
  }
  delete cache;
  delete cl;
}

//----------------------------------------------------------------------
// DiskCache::LoadRequest
//	Place new sector into cache
//----------------------------------------------------------------------

void DiskCache::LoadRequest(int newSector, char *newData, bool dirty, bool
			    used)
{ 
  CacheEntry *newEntry;
  DEBUG(dbgCache,("Cache load request loading sector %d into cache\n", newSector));

  if (curSize == MaxCacheSize) 
    MakeSpace();

  newEntry = new CacheEntry(newSector, newData, dirty, used);
  cache->Insert(newEntry);
  cl->Add(newSector);
  curSize++;
}
 
//----------------------------------------------------------------------
// DiskCache::ReadRequest
//	Place new sector into cache
//----------------------------------------------------------------------
void  DiskCache::ReadRequest(int sectorNum, char *into)
{
  CacheEntry *foundEntry;
  foundEntry = NULL;
  cache->Find(sectorNum, &foundEntry);	// do we have this sector
  
  // if we did not find the sector then we should load it up into the 
  // cache
  if (foundEntry == NULL) {		// not found then
    DEBUG(dbgCache,("Cache read request loading sector %d into cache\n", sectorNum));
    disk->ReadRequest(sectorNum, into); // get sector from disk
    semaphore->P();
    LoadRequest(sectorNum, into, FALSE, TRUE);// place sector in cache
    cacheMiss++;
  } else {
    DEBUG(dbgCache,("Cache read request found sector %d in cache\n", foundEntry->GetSector()));
    bcopy(foundEntry->GetData(), into, SectorSize); // get sector from
    // cache
    foundEntry->SetUsed(TRUE);
    cacheHit++;
  }
}


//----------------------------------------------------------------------
// DiskCache::WriteRequest
//	Place new sector into cache
//----------------------------------------------------------------------
void  DiskCache::WriteRequest(int sectorNum, char *from)
{
  CacheEntry *foundEntry;
  foundEntry = NULL;
  cache->Find(sectorNum, &foundEntry);	// do we have this sector
  
  // if we did not find the sector then we should load it up into the 
  // cache
  if (foundEntry == NULL) {		// not found then
    DEBUG(dbgCache,("Cache write request loading sector %d into cache\n", sectorNum));
    LoadRequest(sectorNum, from, TRUE, TRUE);	// place sector in cache
  } else {
    DEBUG(dbgCache,("Cache write request found sector %d in cache\n", sectorNum));
    foundEntry->Update(from); // get sector from cache
    foundEntry->SetDirty(TRUE);
    foundEntry->SetUsed(TRUE);
    cacheHit++;
  }
}

//----------------------------------------------------------------------
// DiskCache::MakeSpace
//----------------------------------------------------------------------
void DiskCache::MakeSpace(void)
{ 
  CacheEntry *foundEntry;
  bool done = FALSE;
  int sectorNum;
  DEBUG(dbgCache,("Cache is making space for a new page\n"));

  while (!done) {
    sectorNum = cl->Next();
    cache->Find(sectorNum, &foundEntry);
    ASSERT(foundEntry != NULL);
    
    if (!foundEntry->IsUsed()) {
      if (foundEntry->IsDirty()) {
	disk->WriteRequest(sectorNum, foundEntry->GetData());
	semaphore->P();
      }
      cache->Remove(sectorNum);
      cl->Remove(sectorNum);
      curSize--;
      done = TRUE;
    } else {
      foundEntry->SetUsed(FALSE);
    }
  }
}


//----------------------------------------------------------------------
// DiskCache::Flush
//	write back dirty sectors
//----------------------------------------------------------------------
void  DiskCache::Flush(void)
{
  DEBUG(dbgCache,("Cache is flushing\n"));
	
  HashIterator<int, CacheEntry *> iter(cache); 
  CacheEntry *entry;

  for (; !iter.IsDone(); iter.Next()) {
    entry = iter.Item();
    if (entry->IsDirty()) {
      disk->WriteRequest(entry->GetSector(), entry->GetData());
      semaphore->P();
      entry->SetDirty(FALSE);
    }
  }
}

//----------------------------------------------------------------------
// DiskCache::Exists
//	Indicates if a sector is in the cache or not
//----------------------------------------------------------------------
bool  DiskCache::Exists(int s)
{
  return cache->IsInTable(s);
}

//----------------------------------------------------------------------
// DiskCache::LoadFromDisk
//	Indicates if a sector is in the cache or not
//----------------------------------------------------------------------
void  DiskCache::LoadFromDisk(int s)
{
  char into[SectorSize];

  disk->ReadRequest(s, into); // get sector from disk
  semaphore->P();
  LoadRequest(s, into, FALSE, FALSE);// place sector in cache
}

//**********************************************************************
//**********************************************************************


//----------------------------------------------------------------------
// CacheEntry::CacheEntry
//	Makes a new disk cache with maximum of size entries
//----------------------------------------------------------------------

CacheEntry::CacheEntry(int newSector, char *newData, bool newDirty, bool newUsed)
{ 
  sector = newSector;
  dirty = newDirty;
  used = newUsed;
  data = new char[SectorSize];
  Update(newData);
}

//----------------------------------------------------------------------
// CacheEntry::~CacheEntry
//----------------------------------------------------------------------

CacheEntry::~CacheEntry()
{
  delete [] data;
}

//----------------------------------------------------------------------
// CacheEntry::Update
//----------------------------------------------------------------------

void CacheEntry::Update(char *newData)
{
  ASSERT(data != NULL);

  bcopy(newData, data, SectorSize);
}

//----------------------------------------------------------------------
// CacheEntry::SetDirty
//----------------------------------------------------------------------

void CacheEntry::SetDirty(bool d)
{
  dirty = d;
}
//----------------------------------------------------------------------
// CacheEntry::IsDirty
//----------------------------------------------------------------------

bool CacheEntry::IsDirty()
{
  return dirty;
}

//----------------------------------------------------------------------
// CacheEntry::GetData
//----------------------------------------------------------------------

char *CacheEntry::GetData()
{
  return data;
}

//----------------------------------------------------------------------
// CacheEntry::Sector
//----------------------------------------------------------------------

int CacheEntry::GetSector()
{
  return sector;
}

//----------------------------------------------------------------------
// CacheEntry::SetUsed
//----------------------------------------------------------------------

void CacheEntry::SetUsed(bool u)
{
  used = u;
}
//----------------------------------------------------------------------
// CacheEntry::IsUsed
//----------------------------------------------------------------------

bool CacheEntry::IsUsed()
{
  return used;
}

//**********************************************************************
//**********************************************************************
int cacheKey(CacheEntry *cacheEntry) {		// Generates a Key
  return cacheEntry->GetSector();
}

unsigned int cacheHashFunc(int key) {		// Does Hashing function on key
  return (unsigned) key;
}
