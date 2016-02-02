// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files
//
//	Each file in the file system has the following:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"

#include "disk.h"
#include "pbitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"
#include "debug.h"

#ifdef PP4
#include "main.h"
#endif // PP4

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		0
#define DirectorySector 	1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries)

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
    DEBUG(dbgFile,("Initializing the file system.\n"));
#ifdef PP4
    freeMapLock = new Lock("FreeMapLock");
    freeMapRefCount = 0;
    freeMapGlobal = NULL;
#endif

    if (format) {

        Directory *directory = new Directory(NumDirEntries);
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

        DEBUG(dbgFile,("Formatting the file system.\n"));

    // Since we haven't created the FreeMap file yet, pass NULL for now
        PersistBitMap *freeMap =new PersistBitMap(NULL, NumSectors);
     
    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG(dbgFile,("Writing headers back to disk.\n"));
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
/*#ifdef PP4
	freeMapFile->fileHeaderSector=FreeMapSector;
	directoryFile->fileHeaderSector=DirectorySector;
#endif */
    // Match up FreeMap file and the PerstistFreeMap
        freeMap->SetFile(freeMapFile);

    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG(dbgFile,("Writing bitmap and directory back to disk.\n"));
	freeMap->WriteBack();	 // flush changes to disk
	directory->WriteBack(directoryFile);

	if (debug->IsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();
        }
        delete freeMap; 
	delete directory; 
	delete mapHdr; 
	delete dirHdr;
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);

    }
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(char *name, int initialSize)
{
    Directory *directory;
    PersistBitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG(dbgFile,("Creating file %s size %d\n", name, initialSize));

#ifdef PP4
    directory = new Directory(GetNumDirEntries(directoryFile));
#else
    directory = new Directory(NumDirEntries);
#endif
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
      freeMap = GetFreeMap(); //new PersistBitMap(freeMapFile, NumSectors);
        freeMap->FetchFrom();
        sector = freeMap->FindAndSet(); // find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
        else if (!directory->Add(name, sector))
            success = FALSE;	// no space in directory
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, initialSize))
            	success = FALSE;	// no space on disk for data
	    else {	
	    	success = TRUE;
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 
		//freeMap->WriteBack();
    	    	directory->WriteBack(directoryFile);
    	    	freeMap->WriteBack();
	    }
            delete hdr;
	}
	ReleaseFreeMap();//  delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(char *name)
{ 
#ifdef PP4
    Directory *directory = new Directory(GetNumDirEntries(directoryFile));
#else
    Directory *directory = new Directory(NumDirEntries);
#endif
    OpenFile *openFile = NULL;
    int sector;

    DEBUG(dbgFile,("Opening file %s\n", name));
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
#ifdef PP4
    if (sector >= 0) {		
      openFile = new OpenFile(sector);
      openFile->fileHeaderSector = sector;
    }
#else
    if (sector >= 0) 		
	openFile = new OpenFile(sector);	// name was found in
    // directory 
#endif 
    delete directory;

    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(char *name)
{ 
    Directory *directory;
    PersistBitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
#ifdef PP4
    directory = new Directory(GetNumDirEntries(directoryFile));
#else    
    directory = new Directory(NumDirEntries);
#endif
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = GetFreeMap(); // new PersistBitMap(freeMapFile, NumSectors);
    freeMap->FetchFrom();

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack();		// flush to disk
    directory->WriteBack(directoryFile);        // flush to disk
    delete fileHdr;
    delete directory;
    ReleaseFreeMap(); //delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------

void
FileSystem::List()
{
#ifdef PP4
    Directory *directory = new Directory(GetNumDirEntries(directoryFile));
#else
    Directory *directory = new Directory(NumDirEntries);
#endif

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    PersistBitMap *freeMap = GetFreeMap(); // new PersistBitMap(freeMapFile, NumSectors);
#ifdef PP4
    Directory *directory = new Directory(GetNumDirEntries(directoryFile));
#else
    Directory *directory = new Directory(NumDirEntries);
#endif
    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom();
    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    ReleaseFreeMap(); //delete freeMap;
    delete directory;
} 

#ifdef PP4
//----------------------------------------------------------------------
// FileSystem::GetDirectoryFile
// 	returns the root directory file
//----------------------------------------------------------------------
OpenFile* FileSystem::GetDirectoryFile(){
  return directoryFile;
}
#endif // PP4

#ifdef PP4
//----------------------------------------------------------------------
// FileSystem::GetNumDirEntries
// 	Calculates # of directory entries from the file size
//----------------------------------------------------------------------

unsigned FileSystem::GetNumDirEntries(OpenFile *dirFile) {
  ASSERT(dirFile->Length() >= 0 );
  return (unsigned)( dirFile->Length() / (unsigned)
		     sizeof(DirectoryEntry));
}

//----------------------------------------------------------------------
// FileSystem::CreateInDir
// 	Create a file in the Nachos file system (similar to UNIX create).
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::CreateInDir(char *name,  OpenFile *dirFile)
{
    Directory *directory;
    PersistBitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG(dbgFile,("Creating file %s size %d\n", name, 0));


    directory = new Directory(GetNumDirEntries(dirFile));

    directory->FetchFrom(dirFile);

    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
      freeMap = GetFreeMap(); //new PersistBitMap(freeMapFile, NumSectors);
        freeMap->FetchFrom();
        sector = freeMap->FindAndSet(); // find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
        else if (!directory->Add(name, sector))
            success = FALSE;	// no space in directory
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, 0))
            	success = FALSE;	// no space on disk for data
	    else {	
	    	success = TRUE;
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
    	    	directory->WriteBack(dirFile);
    	    	freeMap->WriteBack();
	    }
            delete hdr;
	}
        ReleaseFreeMap(); //delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::OpenInDir
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::OpenInDir(char *name, OpenFile *dirFile)
{ 
    Directory *directory = new Directory(GetNumDirEntries(dirFile));

    OpenFile *openFile = NULL;
    int sector;

    DEBUG(dbgFile,("Opening file %s\n", name));
    directory->FetchFrom(dirFile);
    sector = directory->Find(name); 
    if (sector >= 0) {		
      openFile = new OpenFile(sector);	// name was found in
      // directory 
    openFile->fileHeaderSector = sector;
    } else {
      return NULL;			 // file not found 
    }
    delete directory;
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::RemoveInDir
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::RemoveInDir(char *name, OpenFile *dirFile)
{ 
    Directory *directory;
    PersistBitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    Lock *rmLock;
    directory = new Directory(GetNumDirEntries(dirFile));

    directory->FetchFrom(dirFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       return FALSE;			 // file not found 
    }

    fileHdr = kernel->headManager->Subscribe(sector, &rmLock);
    rmLock->Acquire();
    if (!kernel->headManager->IsAlreadyFetched(sector)) {
      fileHdr->FetchFrom(sector);
      kernel->headManager->SetAlreadyFetched(sector, TRUE);
    }
    rmLock->Release();

    freeMap = GetFreeMap(); // new PersistBitMap(freeMapFile, NumSectors);
    freeMap->FetchFrom();

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack();		// flush to disk
    directory->WriteBack(dirFile);        // flush to disk
    kernel->headManager->UnSubscribe(sector);
    //    delete fileHdr;
    delete directory;
    ReleaseFreeMap(); //delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::ListInDir
// 	List all the files in the current file system directory.
//----------------------------------------------------------------------

void
FileSystem::ListInDir(OpenFile *dirFile)
{
    Directory *directory = new Directory(GetNumDirEntries(dirFile));

    directory->FetchFrom(dirFile);
    directory->List();
    delete directory;
}

//----------------------------------------------------------------------
// FileSystem::CreateDirectoryInDir
// 	Creates a directory in the Nachos file system 
//----------------------------------------------------------------------

bool
FileSystem::CreateDirectoryInDir(char *name, OpenFile *dirFile)
{
    Directory *directory;
    Directory *newDir;
    OpenFile *newDirFile;
    PersistBitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG(dbgFile,("Creating directory %s size %d\n", name, 0));


    directory = new Directory(GetNumDirEntries(dirFile));

    directory->FetchFrom(dirFile);

    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
      freeMap = GetFreeMap();//new PersistBitMap(freeMapFile, NumSectors);
        freeMap->FetchFrom();
        sector = freeMap->FindAndSet(); // find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for new directory header 
        else if (!directory->MakeDirEntry(name, sector))
            success = FALSE;	// no space in directory
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, 0))
            	success = FALSE;	// no space on disk for data
	    else {	
	    	success = TRUE;
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
    	    	directory->WriteBack(dirFile);
    	    	freeMap->WriteBack();
		// add . and ..
		newDirFile = new OpenFile(sector);
		newDir = new Directory(0);
		newDir->FetchFrom(newDirFile);
		newDir->MakeDirEntry(".", sector);
		newDir->MakeDirEntry("..", dirFile->fileHeaderSector);
		newDir->WriteBack(newDirFile);
		delete newDirFile;
		delete newDir;
	    }
            delete hdr;
	}
        ReleaseFreeMap();//delete freeMap;
    }
    delete directory;
    return success;
}

//----------------------------------------------------------------------
// FileSystem::ParseFile
// 	
//----------------------------------------------------------------------
char* FileSystem::ParseFile(char *filepath, OpenFile** MyOpenFile) {

 int NewDirSector;
 OpenFile *CurDirOpenFile=NULL;
 char *buffer, *buffer2;
 int startSect;

 if (filepath == NULL) {
   printf("Wrong path name.\n");
   return NULL;
 }
 if (filepath[0] == '/') { // absolute path
   startSect =
     (kernel->fileSystem->GetDirectoryFile())->fileHeaderSector;
   ASSERT(startSect != -1);
   if(startSect < 0) {
     printf("Can't find root directory.\n");
     return NULL;
   }
   CurDirOpenFile = new OpenFile(startSect);
   ASSERT(CurDirOpenFile != NULL);
   if(CurDirOpenFile == NULL) {
     printf("Can't find root directory.\n");
     delete CurDirOpenFile; // e
     return NULL;
   }
 }
 else {
   startSect =
     (kernel->currentThread->GetCurDirFile())->fileHeaderSector;
   ASSERT(startSect != -1);
   if(startSect < 0) {
     printf("Can't find current directory.\n");
     return NULL;
   }
   CurDirOpenFile = new OpenFile(startSect);
   ASSERT(CurDirOpenFile != NULL);
   if(CurDirOpenFile == NULL) {
     printf("Can't find root directory.\n");
     delete CurDirOpenFile; // e
     return NULL;
   }
 }
 
 Directory *CurDirectory = new Directory(GetNumDirEntries(CurDirOpenFile));
 CurDirectory->FetchFrom(CurDirOpenFile);
 
 buffer = strtok(filepath, "/");
 
 
 do {
   if (buffer == NULL) {
     startSect = (kernel->fileSystem->GetDirectoryFile())->fileHeaderSector;
     *MyOpenFile = new OpenFile(startSect);
     delete CurDirectory;
     delete CurDirOpenFile;
     return buffer;
   } else {
     NewDirSector = CurDirectory->FindDir(buffer);
     if (NewDirSector != -1) {
       delete CurDirOpenFile;
       CurDirOpenFile = new OpenFile(NewDirSector);
       delete CurDirectory;
       CurDirectory = new Directory(GetNumDirEntries(CurDirOpenFile));
       CurDirectory->FetchFrom(CurDirOpenFile);
     } else if (buffer == NULL) {
       printf("File path is invalid.\n");
       delete CurDirectory;
       delete CurDirOpenFile;
       return NULL;
     } else {
       buffer2 = buffer;
       if ((buffer = strtok(NULL, "/")) == NULL) {
	 *MyOpenFile = CurDirOpenFile;
	 delete CurDirectory;
	 return buffer2;
       } else {
	 *MyOpenFile = CurDirOpenFile;
	 delete CurDirectory;
	 return NULL;
       }
     }
   }
 } while((buffer = strtok(NULL, "/")) != NULL);
 *MyOpenFile = CurDirOpenFile;
 delete CurDirectory;
 return NULL;
} //ParseFile

PersistBitMap *FileSystem::GetFreeMap(void) {
  freeMapLock->Acquire();
  freeMapRefCount++;
  if (freeMapGlobal == NULL) {
    freeMapGlobal = new PersistBitMap(freeMapFile, NumSectors);
  }
  freeMapLock->Release();

  return freeMapGlobal;
}

void FileSystem::ReleaseFreeMap(void) {
  freeMapLock->Acquire();
  freeMapGlobal->WriteBack();
/*  if (freeMapRefCount > 0){
    freeMapRefCount = freeMapRefCount - 1;;
    freeMapGlobal->WriteBack();
    if (freeMapRefCount == 0) {
      delete freeMapGlobal;
    }
  }*/
  freeMapLock->Release();
}

#endif

