/*
 * File: gbattle.h
 * Last modified on Thu Apr 20 10:34:10 PDT 2000 by jzelenski
 * ----------------------------------------------------------
 * Defines a set of drawing routines to support the graphics for the 
 * Battleship game.  Includes functions to draw the starting grid and mark 
 * cells as hit, missed, or sunk.  Another function will allow the user to 
 * click on a cell and return the coordinates.  The last function will 
 * display a general printf-style message on the graphics window.
 */

#ifndef _gbattle_h
#define _gbattle_h

#include "genlib.h"

/*
 * Constants
 * ---------
 * NUM_ROWS and NUM_COLS identify the size of the board that will be drawn
 * NUM_ROWS should not be changed to something larger than 26 (it uses the
 * alphabet to identify the rows).
 */
#define NUM_ROWS 10
#define NUM_COLS 12


/*
 * Type: coord
 * -----------
 * The coord struct type is used to identify a location on the board by
 * its row-column position.  Both rows and columns are integer indexed
 * starting from zero at the upper-left corner.
 */
typedef struct {
	int row, col;
} coord;


/*
 * Type: board
 * -----------
 * The board enum is used to identify which of the two boards you are
 * referring to when marking cells, etc. Left and Right refer to the
 * boards by position in the window. The Left board is used to hold
 * the opponent's ship (and your moves), the Right board shows your 
 * ships and opponent's moves.
 */
typedef enum { Left, Right } board;


/*
 * Function: InitBattleGraphics
 * Usage: InitBattleGraphics();
 * ---------------------------
 * Sets up the initial graphics window with the two boards drawn as empty
 * grids. This function should be called once at the beginning of the game.
 */
void InitBattleGraphics(void);

/*
 * Function: MarkShip
 * Usage: MarkShip(Left, location, "Magenta");
 * ------------------------------------------
 * Marks the cell on identified by the location coord with a filled diamond that
 * represents a ship. The diamond is filled with the color specified by the 
 * last parameter. Uses the first parameter to determine whether this update
 * is intended for the Left or Right board. If the location is out of bounds for
 * the board, an error message is generated and the program quits.
 */
void MarkShip(board whichBoard, coord location, string color);


/*
 * Function: MarkHit
 * Usage: MarkHit(Right, location, "Magenta");
 * ------------------------------------------
 * Marks the cell identified by the location coord with a filled red circle that
 * represents a hit. If the color parameter is non-NULL, it also fills a
 * diamond underneath with the given color. For the user's ships, you will know
 * this color, for the opponent's you will not. Passing NULL will cause it to
 * skip that step.  Uses the first parameter to determine whether this update
 * is intended for the Left or Right board. If the location is out of bounds for 
 * the board, an error message is generated and the program quits.
 */
void MarkHit(board whichBoard, coord location, string color);



/*
 * Function: MarkMiss
 * Usage: MarkMiss(Left, location);
 * -------------------------------
 * Marks the cell identified by the location coord with the 'X' that 
 * represents a miss. Al misses are drawn in black.  Uses the first parameter 
 * to determine whether this update is intended for the Left or Right board.
 * If the location is out  of bounds for the board, an error message is printed 
 * and program quits.
 */
void MarkMiss(board whichBoard, coord location);



/*
 * Function: MarkLineAsSunk
 * Usage: MarkLineAsSunk(Left, start, end, "Green");
 * ------------------------------------------------
 * Marks the cells between start and end location (inclusive) with the dimmed
 * status that represents a sunk ship. Each cell has a dimmed red circle on
 * top.  If the color parameter is non-NULL, it also fills a diamond underneath
 * with the given color. For the user's ships, you will know this color, for the 
 * opponent's you will not. Passing NULL will cause it to skip that step.  Uses 
 * the first parameter to determine whether this update is intended for the
 * Left or Right board. This function assumes the start and end are 
 * either in a horizontal or vertical line.  The start location must be 
 * "less" than the end location (ie either the row or col is smaller for 
 * start location than end location, depending on the orientation of the 
 * line.)  If any of the locations between start and end are out of bounds 
 * for the board, an error message is generated and the program quits.
 */
void MarkLineAsSunk(board whichBoard, coord start, coord end, string color);


/*
 * Function: GetLocationChosenByUser
 * Usage: location = GetLocationChosenByUser(Left);
 * -----------------------------------------------
 * The function will wait until the user clicks the mouse somewhere in the 
 * graphics window and will return the coordinates of the cell that was 
 * clicked by the user.  The function returns the coordinates {-1, -1} if 
 * the mouse click was not on a valid board cell for the specified board
 * You may need to click and briefly hold the mouse for this function to properly 
 * register the mouse click, because sometimes this function can miss a mouse-click if is 
 * happens too fast. (It will ignore it and keep waiting for another click).
 * In addition to allowing the user to click a cell to fire, this function 
 * also puts up a "Force Quit" button in the lower-right corner that allows
 * the user to force a quick exit in the middle of a game as a convenience.
 */
coord GetLocationChosenByUser(board whichBoard);


/*
 * Function: DrawPrintfMessage
 * Usage: DrawPrintfMessage(Left, "Location %c-%d was a hit!", 'E', 5);
 * -------------------------------------------------------------------
 * This function has semantics just like printf-you supply the printf-style 
 * format string including % directives and matching arguments.  It will
 * appropriately include the formatted information to construct the message 
 * string.  Instead of writing the message into the text window as printf 
 * does, it will write the message at the bottom of the graphics window, 
 * It keeps a small backlog of messages and scrolls them up and away as 
 * new messages are added.  Each message is assumed to be one line long
 * (i.e. the string should not contain any new-lines!) 
 */
void DrawPrintfMessage(board whichBoard, string format,...);

#endif