
/*
 * File: Creature.java
 * -------------------
 * Provides the abstract class from which all Creatures 
 * extend.  Very little is known about the Creatures at 
 * this level, except that each of them runs as a Thread
 * should therefore extend the Thread class.
 */

public abstract class Creature extends Thread {
    

    /*
     * Method: freeze
     * --------------
     * Any mobile creature (and that's all of them)
     * need to stop moving whenever they enter a
     * Freezer square.  Freezeing a creature is
     * essentially implemented by freezing the
     * thread, and freezing is basically the
     * same as sleeping.
     *
     * Recall that the Thread class exports
     * a sleep method.  We just implement freeze 
     * as a wrapper for Thread.sleep.  Don't
     * worry about this exception business.  It's
     * very handy feature of Java, but they're 
     * hardly representative of the paradig, so 
     * They aren't covered.
     */

    public void freeze(int milliseconds) 
    {
	try {
	    sleep(milliseconds);
	} catch (InterruptedException ie) {
	    System.err.println("Creature thread interrupted.");
	    System.err.println(ie.getMessage());
	}
    }

    /*
     * Methods: kill, creatureIsAlive
     * ------------------------------
     * Sets the Creature to understand that
     * is has just been killed.  The 
     * threadIsAlive boolean is constantly
     * polled with every iteration of the
     * Thread's run method, so that
     * the Creature thread knows when to
     * properly dispose of itself.
     *
     * Feel free to change these methods if you'd
     * like.  Just understand that all Creatures, be
     * be they Humans, friends of Humans, or foes,
     * need to be told to stop looping in their
     * run method.  This is here so that you know
     * that has to be taken care of. If you decide
     * to do something differently, then you'll
     * need to change the Mazerunner.java file, which
     * currently makes use of the kill method.
     */

    public synchronized void kill() { threadIsAlive = false; }
    public synchronized boolean creatureIsAlive() { return threadIsAlive; }
    
    private boolean threadIsAlive = true;
}
