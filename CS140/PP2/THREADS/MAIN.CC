// main.cc 
//	Driver code to initialize, selftest, and run the 
//	operating system kernel.  
//
// Usage: nachos -u -z -d <debugflags> ...
//   -u prints entire set of legal flags
//   -z prints copyright string
//   -d causes certain debugging messages to be printed (cf. debug.h)
//
//  NOTE: Other flags are defined for each assignment, and
//  incorrect flag usage is not caught.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//

#define MAIN
#include "copyright.h"
#undef MAIN

#include "main.h"

// global variables
Debug *debug;
KernelType *kernel;


//----------------------------------------------------------------------
// Cleanup
//	Delete kernel data structures; called when user hits "ctl-C".
//----------------------------------------------------------------------

static void 
Cleanup(int x) 
{     
    printf("\nCleaning up after signal %d\n", x);
    delete kernel; 
}




//----------------------------------------------------------------------
// RootThread
// 	This is the original thread that will be run once the kernel is
//	bootstrapped.
//	
//	Right now, a kernel self-test is just invoked.  You may remove
//	this.  This thread as well as threads forked by this thread (and
//	others recursively forked) is run by the kernel, and your test
//	code should be entered here.
//	
//----------------------------------------------------------------------
void
RootThread(void *arg)
{
    printf("Root Thread: Hello\n\n");
    kernel->SelfTest();
}



//----------------------------------------------------------------------
// main
// 	Bootstrap the operating system kernel.  
//	
//	Initialize kernel data structures
//	Call selftest procedure
//	Run the kernel
//
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int
main(int argc, char **argv)
{
    int i;

#ifdef FILESYS
    bool fsList = FALSE;
    bool fsPrint = FALSE;
    char *copySrc = NULL;
    char *copyDst = NULL;
    char *printArg = NULL;
    char *removeArg = NULL;
#endif

    char *debugArg = "";

    // before anything else, initialize the debugging system
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
	    ASSERT(i + 1 < argc);   // next argument is debug string
            debugArg = argv[i + 1];
	    i++;
	} else if (strcmp(argv[i], "-u") == 0) {
            printf("Partial usage: nachos [-z -d debugFlags]\n");
	} else if (strcmp(argv[i], "-z") == 0) {
            printf("%s\n",copyright);
#ifdef FILESYS
        } else if (strcmp(argv[i], "-cp") == 0) {
          copySrc = argv[++i];
          copyDst = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0) {
          printArg = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0) {
          removeArg = argv[++i];
        } else if (strcmp(argv[i], "-ls") == 0) {
          fsList = TRUE;
        } else if (strcmp(argv[i], "-D") == 0) {
          fsPrint = TRUE;
#endif
	}

    }
    debug = new Debug(debugArg);
    
    DEBUG(dbgThread,("Entering main\n"));

    kernel = new KernelType(argc, argv);
    kernel->Initialize();
    
    CallOnUserAbort(Cleanup);		// if user hits ctl-C

#ifdef FILESYS
    extern void Copy(char *from, char *to);
    extern void Print(char *name);

    if ((copySrc != NULL) && (copyDst != NULL)) {
      Copy(copySrc, copyDst);
    }
    if (printArg != NULL) {
      Print(printArg);
    }
    if (removeArg != NULL) {
      kernel->fileSystem->Remove(removeArg);
    }
    if (fsList) {
      kernel->fileSystem->List();
    }
    if (fsPrint) {
      kernel->fileSystem->Print();
    }
#endif

    //    Thread *rootThread = new Thread("Root Thread");
    // rootThread->Fork((VoidFunctionPtr) RootThread, NULL);

    kernel->Run();

    // this point is never reached
    fprintf(stderr, "Error: kernel returned.\n");

    return 0;
}
