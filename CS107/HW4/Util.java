//---------------------------------------------------------------80 columns---|

/* Util class
 * --------------
 * Just a place to group general-purpose who don't have a home elsewhere.
 * We give you a few static methods that help generate random numbers.
 * You can changes these methods or add additional miscellaneous methods 
 * to this class that don't have a more sensible place to go.
 */

public class Util
{

	/* randomInteger
	 * --------------
	 * Standard bounded range random integer functions. Returns a number
	 * between low and high, inclusive on both ends.
     */
	public static int randomInteger(int low, int high)
	{
		return (int)(Math.random()*(high-low+1)) + low;
	}	
	
	/* randomChance
	 * --------------
	 * Given probability of success, computes random chance to see if
	 * it was within that probability  (returns true) or not (returns false).
     */
	public static boolean randomChance(double probability)
	{	
		return Math.random() <= probability;
	}


}