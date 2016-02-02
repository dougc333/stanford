/*
 * File: sound.h
 * Header file for Microsoft Visual C
 * Version: 4.0
 * Last modified on 1/31/97 by magi
 * ------------------------------------------------------
 * This interface defines a function for playing a named
 * sound resource.
 */

#ifndef _sound_h
#define _sound_h

#include "genlib.h"


/*
 * Function: PlayNamedSound
 * Usage: PlayNamedSound("name");
 * ------------------------------
 * This function looks for a sound file with the given name
 * (and extension .WAV, in the current directory)
 * and plays the corresponding sound if the file is found.
 * The function generates an error if the file cannot be
 * found or the sound facility is unimplemented for the platform.
 */

void PlayNamedSound(string name);

#endif
