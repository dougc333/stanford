/*
 * File: battleship.c
 * -------------------
 * Given below is a fully working compiling version of the Battleship game!
 * Unfortunately, it's missing key features like the ability to actually 
 * shoot missiles, read files, track ships etc, but we leave such minor
 * details up to you to implement =) Good luck!
 */

#include "genlib.h"
#include "strlib.h"
#include "simpio.h"
#include "gbattle.h"
#include "sound.h"
#include "netbattle.h"

#define SHIP_FILE "ShipData"
#define NUM_SHIP_FILES 4

/* This details the kind of opponent you will be playing */
typedef enum {None = 0, Computer = 1, Hosting, ConnectToOther } Opponent;


static void Welcome(void);
static Opponent SetUpConnection(void);
static Opponent ChooseOpponent(void);

main()
{	
	InitBattleGraphics();
	Welcome();
	if (SetUpConnection() != None) {
	    printf("Play game\n");
		CloseConnection();
	} else {
		printf("Unable to establish network connection.\n");
	}
	printf("Hit <RETURN> to exit: ");
	GetLine();
}



static void Welcome(void)
{
	printf("Welcome to the all-new networked Battleship!\n"
		"I, the masterful computer, will randomly place your ships for you.\n"
		"Your opponent is hiding their ships as we speak. You specify a location to \n"
		"fire a missile by clicking the mouse, and your opponent will tell you whether\n"
		"it hit a ship or not. Your opponent then gets to fire a missile at your ships.\n"
		"The winner is the first one to sink all of their opponent's ships.  Ready?  ");
	GetLine();
	PlayNamedSound("Fanfare");
}

/*
 * Function: ChooseOpponent
 * Usage: ChooseOpponent();
 * ------------------------------------------
 * This just gives a prompt then asks the user what kind of opponent
 * he wants to play and returns the response using the Opponent enumerated
 * type
 */
static Opponent ChooseOpponent(void)
{ 	 
	printf("\nYou have these choices for your opponent: \n"
		"\t1) play against the (very dense) computer player\n"
		"\t2) host a new game for another to join\n"
		"\t3) connect to a game hosted by someone else\n");
	while (TRUE) {
		int response;
		printf("Enter choice: ");
		response = GetInteger();
		if (response >= 1 && response <= 3) return (Opponent)response;
		printf("Try again. Valid responses are 1, 2, or 3\n");
	}
}

/*
 * Function: SetUpConnection
 * Usage: opponent = SetUpConnection(game);
 * ------------------------------------------
 * This function first calles the ChooseOpponent function to find out 
 * what kind of foe the user wishes to play. 
 * We then query the user again to see whether he wishes to be the host
 * or the client in the battleship game. If we're the server we wait
 * for an incoming connection, otherwise we connection to a remote
 * computer.
 *
 * RETURN VALUE : The opponent type. Be sure to check if it's None which
 *                means something went wrong!
 */
static Opponent SetUpConnection(void)
{
 	 int type = ChooseOpponent();

     if (type == Computer && ConnectToComputerPlayer()) {
     	return Computer;
     } else if (type == Hosting && HostGame() && WaitForOtherToConnect()) {
     	return Hosting;
 	} else if (type == ConnectToOther) {
 	 	string host;
 		printf("Okay, connecting to a game hosted by someone else.\n");
 		while (TRUE) {
	 		printf("Enter hostname or IP address to connect to (RETURN for this computer): ");
 	 		host = GetLine();
 	 		if (StringLength(host) == 0) host = "localhost";
 	 		if (ConnectToHost(host))
 	 			return ConnectToOther;
	 		printf("Unable to connect to \"%s\". Please try again.\n", host);
 		}
	}
	return None;
}


