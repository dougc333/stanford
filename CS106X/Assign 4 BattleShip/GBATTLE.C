/*
 * File: gbattle.c
 * Last modified on Thu Apr 20 10:34:10 PDT 2000 by jzelenski
 * ----------------------------------------------------------
 * Implementation of the gbattle module:  a set of drawing routines 
 * to support the graphics for the Battleship game.   It is fairly sparsely
 * commented, so peruse at your own risk...
 */
 
#include "gbattle.h"
#include "extgraph.h"
#include "stdarg.h"	
#include "simpio.h"	

#define MARGIN .50				// border around edges

#define LABEL_OFFSET .15		// distance between labels and grid
#define FONT "Helvetica"		// font and size used for labels
#define FONT_SIZE 10 

#define BUTTON_FONT "Charcoal"
#define BUTTON_FONT_SIZE 12
	
#define MESSAGE_Y (MARGIN)   

#define SOLID_FILL 1.0					
#define SUNK_DENSITY .50		// density to fill sunk diamonds
#define HIT_DENSITY 1.0			// density to fill hit diamonds

#define NUM_MESSAGES 3
#define MAX_MESSAGE_SIZE 128

typedef enum {LeftAligned, Centered} alignment;	// for text alignments

typedef struct {double x, y, width, height;} rect;

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static void DrawStartingBoard(board whichBoard, string name, string bgColor, double density);
static void LabelAxes(double x, double y);
static bool ValidLocation(coord location);
static rect RectForBoardCell(board whichBoard, coord location);
static void MarkSquare(board whichBoard, coord location, string color, double fill);
static void DrawCircle(rect r, string color);
static void FillCircle(rect r, string color, double density);
static void DrawRect(rect r, string color);
static void FillRect(rect r, string color, double density);
static void FillDiamond(rect r, string color, double density);
static void DrawX(rect r, string color);
static void DrawCenteredText(double cx, double cy, string str);
static void DrawFormattedText(double x, double y, alignment align, string format,...);
static void DrawGrid(board whichBoard);
void ReportFatalError(char *format, ...);


static double squareSize;
static double boardX[2], upperY;
static char messages[2][NUM_MESSAGES][MAX_MESSAGE_SIZE];
static int nextMessage[2];
static rect quitButton;

void InitBattleGraphics(void)
{
	SetWindowTitle("Head-to-head Battleship!");
	InitGraphics();
	squareSize = MIN((GetWindowWidth() - 3*MARGIN)/(2*NUM_COLS), (GetWindowHeight() - 3*MARGIN)/NUM_ROWS);
	boardX[Left] = MARGIN;
	boardX[Right] = boardX[Left] +squareSize*NUM_COLS+MARGIN;
	upperY = GetWindowHeight() - MARGIN;
	DrawStartingBoard(Left, "Opponent's ships", "Cyan", .5);
	DrawStartingBoard(Right, "Your ships", "White", 1.0);
	SetFont(BUTTON_FONT);
	SetPointSize(BUTTON_FONT_SIZE);
	quitButton.width = TextStringWidth("Force quit")*1.2;
	quitButton.height = GetFontHeight()*1.2;
	quitButton.x = GetWindowWidth()-quitButton.width;
	quitButton.y = 0;
}

static void DrawStartingBoard(board whichBoard, string name, string bgColor, double density)
{    
    rect r;
    
    r.x = boardX[whichBoard];
    r. y = upperY - NUM_ROWS*squareSize;
    r.width = squareSize*NUM_COLS;
    r.height = squareSize*NUM_ROWS;
    FillRect(r, bgColor, density);
   	DrawGrid(whichBoard);
	LabelAxes(r.x, upperY); 
	SetStyle(Bold);
	DrawFormattedText(r.x + NUM_COLS/2*squareSize, upperY + MARGIN - GetFontHeight(), Centered, name);
}


// Predicate value if a coord is valid for this board 
static bool ValidLocation(coord location)
{
	return (location.row >= 0) && (location.row < NUM_ROWS)
		&& (location.col >= 0) && (location.col < NUM_COLS);
}


// Used to verify coord and halt with error if coord is out of bounds 
static void CheckBounds(coord location)
{
	if (!ValidLocation(location))
		ReportFatalError("(%c, %d) is out of bounds for this board!", location.row + 'A', location.col);
}




// Called by both MarkHit & MarkSunk (with different color & fill)
static void MarkSquare(board whichBoard, coord location, string color, double density)
{
	rect r = RectForBoardCell(whichBoard, location);

	CheckBounds(location);			// will halt with error if invalid location
	FillRect(r, "White", SOLID_FILL);
	if (color != NULL) FillDiamond(r, color, density);
	DrawRect(r, "Black");
}

void MarkShip(board whichBoard, coord location, string color)
{
	MarkSquare(whichBoard, location, color, HIT_DENSITY);
	UpdateDisplay();
}



void MarkHit(board whichBoard, coord location, string color)
{
	MarkSquare(whichBoard, location, color, HIT_DENSITY);
	FillCircle(RectForBoardCell(whichBoard, location), "Red", HIT_DENSITY);
	UpdateDisplay();
}



void MarkMiss(board whichBoard, coord location)
{
	MarkSquare(whichBoard, location, NULL, SOLID_FILL);
	DrawX(RectForBoardCell(whichBoard, location), "Black");
	UpdateDisplay();
}


// Marks all squares between start and stop.  Assumes that start is
// "smaller" than end, note the bounds on the two for loops.  It can
// deal with horizontal and vertical orientations (actually diagonal, too
// but the interface doesn't quote this)
void MarkLineAsSunk(board whichBoard, coord start, coord end, string color)
{
	coord current;
	
	for (current.row = start.row; current.row <= end.row; current.row++)
		for (current.col = start.col; current.col <= end.col; current.col++) {
			MarkSquare(whichBoard, current, color, SUNK_DENSITY);
			FillCircle(RectForBoardCell(whichBoard, current), "Red", SUNK_DENSITY);
	}
	UpdateDisplay();
}


// Waits for user to click the mouse (mouse up followed by mouse down)
// Then determines where the mouse position falls on the board.  If
// it is a valid cell, returns the coord.  Otherwise returns {-1,-1}
// as cell location
coord GetLocationChosenByUser(board whichBoard)
{
	coord hit;
	double clickX, clickY;
	
	DrawRect(quitButton, "Black");
	SetFont(BUTTON_FONT);
	SetPointSize(BUTTON_FONT_SIZE);
	SetStyle(Normal);
	DrawCenteredText(quitButton.x+quitButton.width/2, quitButton.y+quitButton.height/2, "Force quit");
	WaitForMouseDown();
	WaitForMouseUp();
	clickX = GetMouseX();
	clickY = GetMouseY();
	if (clickX >= quitButton.x && clickX <= quitButton.x + quitButton.width &&
	    clickY >= quitButton.y && clickY <= quitButton.y + quitButton.height) { 		
		printf("\nForcing game to quit.  Hit RETURN to exit: ");
		GetLine();
		exit(0);
	}
	hit.row = (int)((upperY - clickY)/squareSize);
	hit.col = (int)((clickX - boardX[whichBoard])/squareSize);
	if (!ValidLocation(hit)) hit.row = hit.col = -1;
	FillRect(quitButton, "White", SOLID_FILL);
	return hit;
}


// Fills a big white square first in order to erase any previous text.
// Re-draws "scrolling" list of messages. We put the last-most message 
// added at the bottom in bold to distinguish it from the older ones that
// will "scroll" upwards as new messages are added.
// Draws the text in black using standard font/size.
static void DrawAllMessages(board toBold)
{
	rect r;
	int i, j, index;
	
	SetFont(FONT);
	SetPointSize(FONT_SIZE);
	SetStyle(Bold);
	r.x = boardX[Left] - LABEL_OFFSET;
	r.y = MESSAGE_Y-GetFontDescent();
	r.width = GetWindowWidth();
	r.height = GetFontHeight()*NUM_MESSAGES;
	FillRect(r, "White", SOLID_FILL); // erase all previous messages with white box
	SetPenColor("Black");
	for (j = 0; j < 2; j++) {
		board whichBoard = (board)j;
		for (i = 0; i < NUM_MESSAGES; i++) {
			MovePen(boardX[whichBoard] - LABEL_OFFSET, MESSAGE_Y + (NUM_MESSAGES-i-1)*GetFontHeight());
			index = (i + nextMessage[whichBoard]) % NUM_MESSAGES;
			SetStyle((whichBoard == toBold) && ((index + 1) % NUM_MESSAGES) == (nextMessage[whichBoard]) ? Bold :Normal);
			DrawTextString(messages[whichBoard][index]);
		}
	}
}


// Using vsprintf to process the var args, builds up a formatting string
// and writes to saved message buffer and re-draws array of messages,
// scrolling upwards by one (see DrawAllMessages above)
void DrawPrintfMessage(board whichBoard, string format,...)
{
	va_list args;
	
	va_start(args, format);
	vsprintf(messages[whichBoard][nextMessage[whichBoard]], format, args);
	va_end(format);
	nextMessage[whichBoard] = (nextMessage[whichBoard] +1) % NUM_MESSAGES;
	DrawAllMessages(whichBoard);
	UpdateDisplay();
}


// Using vsprintf to process the var args, builds up a formatting string
// and writes to specified location on the graphics window.
// Draws the text in black, using the current font.
static void DrawFormattedText(double x, double y, alignment align, string format,...)
{
	va_list args;
	char buffer[1024];
	
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(format);
	SetPenColor("Black");
	if (align == Centered) {
		DrawCenteredText(x,y,buffer);
	} else {
		MovePen(x, y);
		DrawTextString(buffer);
	}
}


static void FillDiamond(rect r, string color, double density)
{
	SetPenColor(color);
	MovePen(r.x, r.y + r.width/2); // move to left side of diamond
	StartFilledRegion(density);
	DrawLine(r.width/2, r.height/2);	// draw up to top point
	DrawLine(r.width/2, -r.height/2);	// draw down to right side
	DrawLine(-r.width/2, -r.height/2);	// draw down to bottom point
	DrawLine(-r.width/2, r.height/2);	// draw back to left side
	EndFilledRegion();
}


static void DrawX(rect r, string color)
{
	SetPenColor(color);
	MovePen(r.x,r.y);					// move to lower-left corner
	DrawLine(r.width, r.height);		// draw to upper-right corner
	MovePen(r.x, r.y+r.height);			// move to upper-left corner
	DrawLine(r.width, -r.height);		// draw down to lower-right corner
}


static void DrawRect(rect r, string color)
{
	SetPenColor(color);
	MovePen(r.x, r.y);
	DrawLine(r.width,0);
	DrawLine(0,r.height);
	DrawLine(-r.width,0);
	DrawLine(0,-r.height);
}


static void FillRect(rect r, string color, double density)
{
	StartFilledRegion(density);
	DrawRect(r, color);
	EndFilledRegion();
}

static void DrawCircle(rect r, string color)
{
	r.x += squareSize/4;
	r.y += squareSize/4;
	r.width -= squareSize/2;
	r.height -= squareSize/2;
	SetPenColor(color);
	MovePen(r.x + r.width, r.y + r.height/2);
	DrawEllipticalArc(r.width/2, r.height/2, 0, 360);
}


static void FillCircle(rect r, string color, double density)
{
	StartFilledRegion(density);
	DrawCircle(r, color);
	EndFilledRegion();
}



static void DrawGrid(board whichBoard)
{
	int row, col;
	
	for (row = 0; row < NUM_ROWS; row++) {
		for (col = 0; col < NUM_COLS; col++) {
			coord position;
			position.row = row;
			position.col = col;
			DrawRect(RectForBoardCell(whichBoard, position), "Black");
		}
	}
}



// Horizontally AND vertically centers the text around the specified point
// given the metrics of the current font setting.
static void DrawCenteredText(double centerX, double centerY, string str)
{
	MovePen(centerX - TextStringWidth(str)/2, centerY - GetFontAscent()/2);
	DrawTextString(str);
}


// Labels the grid axes.  Columns are numbered from 0 to NUM_COLS -1 along the
// top edge of the grid.  Rows are lettered from 'A' down along the left edge
// of the grid.  Uses the DrawFormattedText function to aesthetically center
// each label.
static void LabelAxes(double x, double y)
{
	int row, col;
	
	SetFont(FONT);
	SetPointSize(FONT_SIZE);
	SetStyle(Normal);
	for (row = 0; row < NUM_ROWS; row++)
		DrawFormattedText(x - LABEL_OFFSET, y - (row +.5)*squareSize, 
					Centered, "%c", 'A' + row);
	for (col = 0; col < NUM_COLS; col++)
		DrawFormattedText(x + (col+.5)*squareSize, y + LABEL_OFFSET,
					Centered, "%d", col);
}

// Helper to get the rectangle enclosing a particular board cell
static rect RectForBoardCell(board whichBoard, coord location)
{
	rect r;
	r.x = boardX[whichBoard] + location.col*squareSize;
	r.y = upperY - (location.row+1)*squareSize;
	r.width = squareSize;
	r.height = squareSize;
	return r;
}

