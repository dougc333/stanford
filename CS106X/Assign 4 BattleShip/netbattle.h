/*
 * File: netbattle.h
 * Last modified on Thu Apr 20 10:34:10 PDT 2000 by jzelenski
 * ----------------------------------------------------------
 * This file is the network library written for the CS106 assignment
 * Battleship. It provides a simple mechanism for two Battleship clients
 * to communicate with each other through a standard protocol. Clients
 * of this library need only concern themselves with using the provided
 * response structure to send/receive over the network
 * 
 * Using the functions in this library is simple, however be sure to 
 * read all of the documentation contained in this header file thoroughly
 * before starting your coding. In particular, pay close attention to 
 * the return values provided by every function, they tell you what
 * exactly happened during the call. Unlike most of the functions you've
 * seen, network failures can and do occur in the middle of function calls
 * and the only way this can be reported to you is through the return value,
 * so be sure to check it to make sure everything went according to plan. If
 * you ever get an unexpected result (for example, FALSE coming back from
 * SendShotToOpponentAndWaitForResponse) all you need to do is Error()
 * out of the program, you need not write code to recover.
 */

#include "gbattle.h"
#include "genlib.h"

#ifndef _battlenet_
#define _battlenet_



/* Type : ShotResult
 * -----------------
 * The five possible results that come from firing a shot.
 */
typedef enum {AlreadyFiredOn=1, ShotMissed, ShotHit, ShipSunk, LastShipSunk} ShotResult;


/* Maximum number of characters that can be sent in a 'message' in the 
   response packet defined below */
#define MAX_CHARS 128


/* Type : response
 * ---------------
 * This structure is what is used to provide a means of communication between
 * the two players in the game. When waiting for a response, a blank structure
 * is passed in to the SendShotAndWaitForResponse function and upon return
 * the appropriate fields will have been filled in. When sending a response,
 * you first fill in the appropriate fields then call the 
 * SendResponseToOpponentShot function, passing the address of your response
 * structure as the argument. 
 * 
 * Note that because this struct is used only as a data passing mechanism
 * and does not hold any persistent data, we strongly recommend that you 
 * place it on the stack rather than dynamically allocating memory. For 
 * example, in the case of receiving a response, you could do
 *
 * void FireAShot(coord shot) 
 * {
 *    response shotresponse;
 *    if (SendShotToOpponentAndWaitForResponse(shot, &shotresponse))
 *    --- use your newly filled in shotresponse here---
 * 
 * A similar technique can be used when sending responses.
 *
 * When you have a message to send, sprint or strcpy into the message
 * buffer of the response structure.  One restriction is that you may
 * NOT use the \r or \n  characters at all in the message string being sent. 
 * Doing so will result in an error being raised in the network functions 
 * that use that struct. 
 */
typedef struct {
    ShotResult result; /* whether shot was hit/miss/sunk, etc.*/	
    char message[MAX_CHARS]; /* any message sent by player in response to shot */
    coord sunkShipStart, sunkShipEnd; /* If a ship was sunk, where are the 
										 start/end coords of that ship */
} response;



/*
 * Function: HostGame
 * Usage:if (HostGame()) {
 * -----------------------
 * This asks the network library to prepare the computer to be a network
 * game host. The actual waiting for the connection occurs in 
 * WaitForOtherToConnect but this function must be called first to set things
 * up properly. If another Battleship program is already hosting on this computer
 * a second attempt to host will fail.
 *
 * RETURN VALUE : Whether the initialisation was successful 
 */
bool HostGame(void);



/*
 * Function: WaitForOtherToConnect
 * Usage: gotClient = WaitForOtherToConnect();
 * ------------------------------------------
 * This function requires that HostGame() already have been called. It
 * asks the local computer to wait until a remote computer attempts to connect
 * over the network. If no machine has connected after 2 minutes, then this
 * function will give up and return with FALSE as the return value.
 *
 * Note that a host must be waiting in this function BEFORE another machine
 * can successfully connect to it. Otherwise the remote machine will fail
 * its ConnectToHost call (see below). Thus when testing be sure to have
 * your first instantiation of the program be the host while the second is
 * the client.
 *
 * RETURN VALUE : Whether another computer successfully connected
 */
bool WaitForOtherToConnect(void);



/*
 * Function: ConnectToHost
 * Usage: if (ConnectToHost("winky.stanford.edu")) {
 * -------------------------------------------------
 * This function asks the local computer to attempt to connect to another
 * computer over the net (though it may quite possibly be the local machine if 
 * that is the address given!). The host string should either be an IP
 * number of the form 
 *	
 *    171.64.64.28
 *
 * or a domain host name such as 
 *
 *    winky.stanford.edu
 *
 * This is probably the most likely function to fail. If there is no network
 * on the current machine or the host does not exist on the internet, then
 * this function will immediately return with the value FALSE. Similarly if
 * the machine exists and is up but there is no Battleship host currently
 * waiting for a connection on that machine, this function will immediately
 * fail. If the machine exists but is currently off or disconnected from
 * the network, then this function will wait for 2 minutes before giving
 * up and returning FALSE.
 *
 * RETURN VALUE : Whether the connection was successfully made
 */
bool ConnectToHost(string host);



/*
 * Function: ConnectToComputerPlayer
 * Usage: if (ConnectToComputerPlayer()) {
 * ---------------------------------------
 * This asks the network library to not really connect to another machine
 * but rather get turn information from the computer player instead. This
 * allows you to test easily using the computer player but then quickly
 * and easily switch to the network later!
 *
 * RETURN VALUE : Whether the computer player was successfully set up
 */
bool ConnectToComputerPlayer(void);



/*
 * Function: CloseConnection
 * Usage: CloseConnection();
 * -------------------------
 * This function shuts down any existing connection and cleans up any
 * memory it may have allocated. Call this at the end of your game to
 * have things be neat and tidy.
 */
void CloseConnection(void);



/*
 * Function: SendShotToOpponentAndWaitForResponse
 * Usage: if (SendShotToOpponentAndWaitForResponse(myCoord, &theirReponse)) {
 * ------------------------------------------------------------------------
 * This function takes a coordinate as a parameter and sends that to the
 * player at the other end of the connection as a shot. The other player
 * is responsible for sending the result of that shot back to us.
 * This function fills in the response structure whose address
 * was passed in to the function with the appropriate values. Thus be sure
 * that the response parameter has memory allocated to it (most likely 
 * on the stack).
 *
 * If the network or remote computer has crashed/ceased to work between
 * setting up the connection and this call, the function will return FALSE.
 * If the problem is a network problem then a delay of up to 2 minutes may
 * occur before this function returns, though most of the time the function
 * will return immediately if the other side of the connection is down.
 * Given the timeout, this also means that if the other player takes
 * more than 2 minutes to decide the response to a shot it will time out, 
 * and end the game early. (This shouldn't actually happen in practice).
 *
 * RETURN VALUE : Whether the shot was sent and a response received.
 */
bool SendShotToOpponentAndWaitForResponse(coord location, response *response);



/*
 * Function: WaitForOpponentShot
 * Usage: if (WaitForOpponentShot(&coord)) {
 * -----------------------------------------
 * A function which blocks and wait for shot information to come in from
 * the remote opponent calling SendShotToOpponentAndWaitForResponse. Once
 * he has called this, WaitForOpponentShot will fill in the coord structure
 * whose address was passed in with the appropriate x,y positions.
 *
 * This function will wait for up to 2 minutes for information about the
 * other turn to come in, otherwise it will assume the other computer 
 * has crashed or left the game and will close the connection. This means
 * that you should definitely make your shots in under 2 minutes of thinking
 * time! You must also ALWAYS check the return value as usual, do not
 * assume that the coord has been initialised because when this function
 * returns FALSE, the coord contents are left as the junk they started.
 *
 * RETURN VALUE : Whether a shot was successfully received.
 */
bool WaitForOpponentShot(coord *location);



/*
 * Function: SendResponseToOpponentShot
 * Usage: if (SendResponseToOpponentShot(&myresponse)) {
 * ------------------------------------------------------
 * After you receive the position of a shot via WaitForOpponentShot and 
 * have determined the results of that shot, you can inform your opponent
 * of those results via this function. SendResponseToOpponentShot takes
 * the information in the passed in response structure ( so be sure it 
 * is filled in correctly! ) and transmits those to the other player who
 * will receive it through SendShotToOpponentAndWaitForResponse.
 *
 * As state in the comment about the response packet, be SURE that your
 * message in the response packed does not contain a \r or a \n character!
 *
 * RETURN VALUE : Whether the response was properly sent to the opponent
 */
bool SendResponseToOpponentShot(response *response);

#endif