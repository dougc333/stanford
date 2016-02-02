/* sweethand.c
 * ------------
 * The starting file for HW3 CS107 Fall 2000. This program is largely
 * just for illustration purposes and mostly should be thrown away
 * when you are ready to develop your solution. It does a full sequential
 * finger of all names and all hosts which takes an immense amount of
 * time to do one by one.
 *
 */

#include "thread_107.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "scanner.h"
#include "network.h"
#include <netdb.h>   // for MAXHOSTNAMELEN 
#include <limits.h>  // for LOGNAME_MAX 

#define MAX_HOSTS 200
#define MAX_SOCKETS 64
#define MAX_TRIES 4
#define MAX_REQUESTS_PER_HOST 3
#define UNREACHABLE_THRESHHOLD 3
#define SECOND 1000000
#define PRINT_EVERY 5*SECOND

typedef char Username[LOGNAME_MAX + 1];
typedef char Hostname[MAXHOSTNAMELEN + 1];

static struct host {
    Hostname name;
    int addr;
} hosts[MAX_HOSTS];

static void PrintTimeNow(void);
static int ScanAllHosts(const char *hostfile);
static void SequentialFingerAll(const char *userfile, const char *hostfile);

/* 
 * main
 * ----
 * Not much here.  The code here picks out the debug trace
 * arguments for you and sets the flags which you might find handy.
 * It then calls the function to do the entire sequential scan.
 */
int main(int argc, char **argv) 
{
    bool networkTrace = false, threadTrace = false;
    int numHosts;
    
    if (argc >= 2 && argv[1][0] == '-') {
        threadTrace = (strchr(argv[1], 't') != NULL); 
        networkTrace = (strchr(argv[1], 'n') != NULL); 
        argv++; // advance past the flag argument
        argc--;
    }
    if (argc < 3) {
        printf("\nOOPS!  This program takes two arguments: the file of users and the file of hostnames.\nPlease try again!\n\n");
        exit(-1);
    }
    
    InitNetworkPackage(MAX_SOCKETS, networkTrace);
    InitThreadPackage(threadTrace);
    
    SequentialFingerAll(argv[1], argv[2]);
    return 0;
}


/* 
 * SequentialFingerAll
 * -------------------
 * This is a very simple sequential finger operation that helps you
 * to see how our routines work.  It scans each real name
 * from the user file, uses our Whois function to translate to
 * leland username, and then loops through all hosts (earlier
 * scanned into a global array of hostnames) finger the user on
 * that host.  For each hostname, it ensures there is a valid network
 * address and then a finger request is sent out with SendFingerRequest 
 * function. The result is printed and it moves on.  Note it doesn't try 
 * to re-send, it doesn't collate results, and all the other things that 
 * your solution will need to do.  This is only here to show the
 * usage of our functions and you should quickly throw this code out and
 * get started on designing your own solution.
 * By the way, it is somewhat interesing to note how painfully slow 
 * this version is. I started on the full CS107 class and entire leland
 * host file and got bored and cancelled it after a half hour, at which 
 * point it had only finished about a quarter of the users, so figure
 * a full run is a multi-hour affair. Don't try this at home, just take 
 * my word for it. Your snappy concurrent program should be faster by 
 * almost an order of magnitude when you're done with it.
 */
static void SequentialFingerAll(const char *userfile, const char *hostfile)
{
    Scanner names;
    int i, count, numHosts;
    char realname[1024];
    Username username;

    numHosts = ScanAllHosts(hostfile); // first read in all hostnames
    names = NewScannerFromFilename(userfile, "\n", true);
    assert (names != NULL);
    OpenSocket(0);
    while (ReadNextToken(names, realname, sizeof(realname))) {
        if (Whois(realname, username, sizeof(username))) {
            for (i = 0; i < numHosts; i++) {
                if (hosts[i].addr != NETWORK_ERROR) {
                    count = SendFingerRequest(username, hosts[i].addr, 0, false);
                    PrintTimeNow();
                    printf("Finger %s@%s = %d\n", username, hosts[i].name, count);
                }
            }
        }
    }
    CloseSocket(0);
    FreeScanner(names);
}


/*
 * ScanAllHosts
 * -------------
 * The one argument is the filename containing the list of hosts, one 
 * per line. It uses the scanner to pull them out and feeds them into 
 * the global array of hostnames. It stops when the fixed-sized array
 * is filled to capacity, or when it hits EOF in the file, whichever
 * comes first. The return value is the total number of hostnames 
 * written to the global array. As it stands, this function directly
 * looks up the hostname address while scanning, you will handle this
 * in the username threads in your program.
 */
static int ScanAllHosts(const char *hostfile)
{
    int i = 0;
    
    Scanner s = NewScannerFromFilename(hostfile, "\n", true);
    assert(s != NULL);
    while (ReadNextToken(s, hosts[i].name, sizeof(hosts[i].name))) {
        hosts[i].addr = LookupHostAddress(hosts[i].name);
        if (++i == MAX_HOSTS) break;    // fixed maximum, don't overrun array!
    }
    FreeScanner(s);
    return i;   // i is now the total number of hosts
}


/*
 * PrintTimeNow
 * -------------
 * Prints the current date/time. Uses time() to get the current
 * time and strftime() to do the date formatting.  See the man pages
 * if you want to know more.
 */
static void PrintTimeNow(void)
{
    char buf[1024];
    time_t now;
    struct tm tm;
    
    time(&now);
    strftime(buf, sizeof(buf), "%b %e %r", localtime_r(&now, &tm));
    printf("[%s] ", buf);
}
