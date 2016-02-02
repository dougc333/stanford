//---------------------------------------------------------------80 columns---|

/* MazeRunner class
 * ---------------
 * This is the game class that controls reading the maze files, 
 * setting up the board, managing its contents, and starting and stopping
 * the creature threads through the levels.  A lot of the code for this 
 * class is given to you to start with, but you will need to make changes 
 * to support the features of your game as you go.  You are free to 
 * modify this class in any way which you need to to get the job done.
 */

import java.io.*;
import java.util.Vector;

public class MazeRunner {
    
    /* This is the "main" class for the application. When we start the java 
     * interpreter, we give the name of the main class, and execution starts 
     * at the static main method in that class. The args array is a list of 
     * the command-line arguments specified (in this case, we expect none).
     * All we do here is create a new instance of the MazeRunner
     * game class and then tell it to play.
     */
    
    public static void main(String args[]) 
    {
	MazeRunner game = new MazeRunner();
	game.play();
    }
    
    
    /* Instance variables for the MazeRunner class. All are declared private
     * since no outside client should be directly messing with our internal 
     * state. Where needed, accessors can be provided to obtain their values.
     */
    private Grid squares;   
    private Vector allCreatures;
    private boolean levelInProgress, userStillAlive;
    private static int NumLevels = 5;	
       // there are 5 different mazes we give you
    
    /* MazeRunner constructor
     * ----------------------
     * This constructor just sets the initial values for the instance 
     * variables and not much else. All variables not set are initialized 
     * to zero (null).
     */
    
    public MazeRunner() 
    {
	userStillAlive = true;
	levelInProgress = false;
	allCreatures = new Vector();
    }
    
    /* 
     * isRunning()
     * -----------
     * Can be used by clients (such as the creatures) to tell whether there 
     * is an active game in progress.  This will return true when play is
     * active, false otherwise, so a creature may want to stop moving/acting 
     * when play is no longerin session.
     */
    
    boolean isRunning() 
    {
	return levelInProgress;
    }
	
    /* play()
     * ------
     * Runs the game to completion.  Starts at level 1, reads in the maze
     * file, activates the level, waits for level to end (either because
     * user won or was tagged) and then stops the level. If user is still
     * alive, keeps advancing to next level.
     */
    
    public synchronized void play()
    {
	int level = 0;
	
	while (userStillAlive && level < NumLevels) {	// if user hasn't yet been tagged
	    readMazeFileForLevel(level);// read maze from file, setup board
	    startLevel(level);			// start all creatures going
	    try {
		wait();	// wait for signal from a creature that level is over
	    } catch (InterruptedException ie) {}
	    stopLevel();				// stops all creature threads
	    level++;					// advances to next level
	}
    }
    
    
    /* levelOver()
     * -----------
     * This method is called when an action occurred that should end
     * the current level. That may be that the user made it to the
     * goal square, or the user was tagged, or the user may have chosen
     * to quit the game or skip this level. The boolean passed as the
     * parameter indicates whether the game should stop right here or 
     * keep going and advance to next level.
     */
    public synchronized void levelOver(boolean advanceToNextLevel) 
    {
	levelInProgress = false;
	userStillAlive = advanceToNextLevel;	// indicates whether to continue or stop here
	Display.drawStatusMessage(advanceToNextLevel ? "On to next level!" :"Game over!");
	notify();		// send signal to game thread that level is over
    }
    
    
    
    /* startLevel()
     * ------------
     * Helper method to activate a new level. It displays a status
     * message that tells the user to hit a key, and then waits for
     * that keystroke. When given, it will mark the level in progress
     * and start all the creature threads so that the level is underway
     */
    private void startLevel(int levelNumber)
    {
	Display.drawStatusMessage("Level " + levelNumber + ": Hit space bar to begin.");
	while (Display.getKeyFromUser() != ' ') // wait for user to type space
	    ;
    	levelInProgress = true;
	for (int i = 0; i < allCreatures.size(); i++) // iterate over all creatures, starts their threads
	    ((Thread)allCreatures.elementAt(i)).start();
	Display.drawStatusMessage("Playing level " + levelNumber + "...");
    }
    
    
    /* stopLevel()
     * -----------
     * Helper method to end the current level. It just iterates
     * through all creature threads and stops them in their tracks.
     * There are a couple of strange sleep calls that are just an
     * artifact of some weird thread behavior we are working around.
     */
    private void stopLevel()
    {
	try { Thread.sleep(200);} 
	catch (InterruptedException ie) {} // allow signal thread to leave
	
	for (int i = 0; i < allCreatures.size(); i++)
	    ((Creature)allCreatures.elementAt(i)).kill();
	
	try { Thread.sleep(200);} // give other threads a chance to get stopped
	catch (InterruptedException ie) {}
    }
    
    
    
    /* readMazeFileForLevel()
     * ----------------------
     * Reads the configuration for one maze level from a file. The
     * files are assumed to be stored in a subdirectory Mazes of the
     * current directory. The maze filenames should be "Level1.maze",
     * "Level2.maze" and so on. The first two lines of the file identify
     * how many rows and columns are in the particular maze, the
     * grid and display are reconfigured to match that. Then we process
     * all of the individual squares one by one from the maze file. If
     * at any point we run into a problem (can't find file, file improperly
     * formatted), it just gives up, that's enough error-checking for this
     * assignment. :-)
     */
    
    private void readMazeFileForLevel(int level)
    {
	String mazeDirectory = System.getProperty("user.dir") + java.io.File.separator + "Mazes" + java.io.File.separator;
	String filename = mazeDirectory + "Level" + level + ".maze";
	BufferedReader in; 
	
	try {
	    in = new BufferedReader(new FileReader(filename)); 
     	} 
     	catch (FileNotFoundException e) {	
	    System.out.println("Cannot find file \"" + filename + "\".");
	    return;
	}
	
	try {
	    int numRows = Integer.valueOf(in.readLine().trim()).intValue(); 
	    int numCols = Integer.valueOf(in.readLine().trim()).intValue(); 
	    
	    squares = new Grid(numRows, numCols);	// create new grid (no need to throw away old one!
	    Display.configureForSize(numRows,numCols);
	    Display.drawStatusMessage("Loading level " + level +"...");
	    allCreatures.removeAllElements();	// empty vector of any previous creatures
	    
	    for (int row = 0; row < numRows; row++) {
		for (int col = 0; col < numCols; col++)
		    readOneSquare(new Location(row, col), (char)in.read());
		in.readLine();	// skip over newline at end of row
	    }
	}
	catch (IOException e) { 
	    System.out.println("File improperly formatted, quitting"); 
	    return;
	}	
    }
    
    
    /* readOneSquare()
     * ---------------
     * Given the character just read from the maze file and that
     * location that it represents, this method is supposed to 
     * configure that square in the grid to have the proper contents
     * be it a wall square, a ladder square, etc. and potentially to
     * have a creature creating and occupying that square as well.
     * Any creature that is allocated should be added to the
     * Vector the game keeps of all creatures so that it is properly
     * started and stopped during this level. Right now, this doesn't
     * do at all what it is supposed to, it just draws the contents of
     * file with the iamges, so you get the basic idea. You'll need to
     * change it to instantiate your objects and set up the maze world.
     */
    private void readOneSquare(Location location, char ch)
    {
	switch (ch) {
	case 'H': 
	    Display.drawAtLocation("Empty", location); // draw background
	    Display.drawAtLocation("Human", location); break;
	case 'R': 
	    Display.drawAtLocation("Empty", location);
	    Display.drawAtLocation("Rover", location); break;
	case 'J': 
	    Display.drawAtLocation("Empty", location); 
	    Display.drawAtLocation("Jumper", location); break;
	case 'P': 
	    Display.drawAtLocation("Empty", location); 
	    Display.drawAtLocation("PacerEast", location); break;
	case 'E':
	    Display.drawAtLocation("Empty", location); 
	    Display.drawAtLocation("Energy", location); break;
	case ' ' : 
	    Display.drawAtLocation("Empty", location); break;
	case '%' : 
	    Display.drawAtLocation("Brick", location); break;
	case '!' : 
	    Display.drawAtLocation("Goal", location); break;
	case '#' :
	    Display.drawAtLocation("Ladder", location); break;
	case '*' : 
	    Display.drawAtLocation("Freezer",location); break;
	case '0' : case '1' : case '2': case '3' : case '4': 
	case '5' : case '6' : case '7': case '8' : case '9': 
	    Display.drawAtLocation("Hyper", ch, location); break;
	}
    }
}
