#include "headman.h"
#include "synch.h" 
HeadEntry::HeadEntry(int newSector) {

  lock = new Lock("HeadEntryLock");
  
  lock->Acquire();
  sector = newSector;
  refCount = 0;
  alreadyFetched = FALSE;
  hdr = new FileHeader;
  lock->Release();
}

HeadEntry::~HeadEntry() {
  lock->Acquire();
  //hdr->WriteBack(sector);
  delete hdr;
  lock->Release();
  delete lock;
}

//******************************************************************
//******************************************************************
HeadManager::HeadManager() {
  lock = new Lock("HeadManagerLock");
  registry = new HashTable<int, HeadEntry *>(headKey, headHashFunc);
}

HeadManager::~HeadManager() {
  lock->Acquire();

  registry->SanityCheck();

  // empty registry

  HashIterator<int, HeadEntry *> iter(registry); 
  HeadEntry *entry;
  HeadEntry *removed;
  int s;
/*
  for (; !iter.IsDone(); iter.Next()) {
    entry = iter.Item();
    ASSERT(entry != NULL);
    s =entry->GetSector();
    removed = registry->Remove(s);
    delete removed;
  }

  delete registry;*/
  lock->Release(); 
  delete lock;
}

FileHeader *HeadManager::Subscribe(int sector, Lock **hdrLock) {
  HeadEntry *foundEntry;
  
  lock->Acquire();
  registry->Find(sector, &foundEntry);
  
  if (foundEntry == NULL) {
    foundEntry = new HeadEntry(sector);
    ASSERT(foundEntry != NULL);
    registry->Insert(foundEntry);
  }

  foundEntry->IncRefCount();
  *hdrLock = foundEntry->GetLock();
  lock->Release();
  return foundEntry->GetHeader();
}

void HeadManager::UnSubscribe(int sector) {
  HeadEntry *foundEntry;
  HeadEntry *removedEntry;
  lock->Acquire();

  registry->Find(sector, &foundEntry);
  ASSERT(foundEntry != NULL);
  
  foundEntry->DecRefCount();
  if (foundEntry->DecRefCount() == 0) {
    removedEntry = registry->Remove(sector);
    delete removedEntry;
  }

  lock->Release();
}


bool HeadManager::IsAlreadyFetched(int sector) {
  HeadEntry *foundEntry;
  lock->Acquire();
  registry->Find(sector, &foundEntry);
  ASSERT(foundEntry != NULL);
  lock->Release();
  return foundEntry->IsAlreadyFetched();
}

void HeadManager::SetAlreadyFetched(int sector, bool a)
{
  HeadEntry *foundEntry;
  lock->Acquire();
  registry->Find(sector, &foundEntry);
  ASSERT(foundEntry != NULL);

  foundEntry->SetAlreadyFetched(a);
  lock->Release();

}
//******************************************************************
//******************************************************************
int headKey(HeadEntry *headEntry) {
  return headEntry->GetSector();
}
  
unsigned int headHashFunc(int key) {
  return key;
}
