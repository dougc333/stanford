#include "dirman.h"
#include "synch.h" 

DirEntry::DirEntry(OpenFile *dirFile) {
  sector = dirFile->fileHeaderSector;
  lock = new Lock("DirEntryLock");

  lock->Acquire();
  tableSize = kernel->fileSystem->GetNumDirEntries(dirFile);
  realTableSize += NumSpareEntries;

  table = new DirectoryEntry[realTableSize];
  (void) dirFile->ReadAt((char *)table, tableSize *
		      sizeof(DirectoryEntry), 0);
  lock->Release();

  refCount = 0;
}

DirEntry::~DirEntry() {
  lock->Acquire();
  delete [] table;
  lock->Release();
  delete lock;
}

//******************************************************************
//******************************************************************
DirectoryManager::DirectoryManager() {
  lock = new Lock("DirectoryManagerLock");
  registry = new HashTable<int, DirEntry *>(DirKey, DirHashFunc);
}

DirectoryManager::~DirectoryManager() {
  lock->Acquire();

  registry->SanityCheck();

  // empty registry

  HashIterator<int, DirEntry *> iter(registry); 
  DirEntry *entry;

  for (; !iter.IsDone(); iter.Next()) {
    entry = iter.Item();
    if (entry == NULL) break;
    entry = registry->Remove(entry->GetSector());
    delete entry;
  }

  delete registry;
  lock->Release();
  delete lock;
}

DirectoryEntry *DirectoryManager::Subscribe(int **tableSize, int **realTableSize,
				      Lock **dirLock, OpenFile *dirFile) {
  DirEntry *foundEntry;
  
  lock->Acquire();
  registry->Find(dirFile->fileHeaderSector, &foundEntry);
  
  if (foundEntry == NULL) {
    foundEntry = new DirEntry(dirFile);
    registry->Insert(foundEntry);
  }
   
    
  foundEntry->IncRefCount();
  *dirLock = foundEntry->GetLock();
  *tableSize = foundEntry->GetTableSize();
  *realTableSize = foundEntry->GetRealTableSize();
  lock->Release();
  return foundEntry->GetDirectoryTable();
}

void DirectoryManager::UnSubscribe(int sector) {
  DirEntry *foundEntry;
  DirEntry *removedEntry;
  lock->Acquire();

  registry->Find(sector, &foundEntry);
  ASSERT(foundEntry != NULL);
  
  if (foundEntry->DecRefCount() == 0) {
    removedEntry = registry->Remove(sector);
    delete removedEntry;
  }

  lock->Release();
}

void DirectoryManager::UpdateEntry(int sector, DirectoryEntry *table) {
  DirEntry *foundEntry;
  lock->Acquire();

  registry->Find(sector, &foundEntry);
  ASSERT(foundEntry != NULL);

  foundEntry->SetDirectoryTable(table);

  lock->Release();
}

//******************************************************************
//******************************************************************
int DirKey(DirEntry *dirEntry) {
  return dirEntry->GetSector();
}
  
unsigned int DirHashFunc(int key) {
  return key;
}
