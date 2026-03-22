#ifndef __SOUND_HPP__
#define __SOUND_HPP__

/**
* This function loads a .sa2 song from !SurpriseProductions Adlib Tracker 2.0 and plays it
*
* @param name The file name of the *.sa2 to load and play.
* @example playSong("DATA/SOUND/TEST.SA2")
* @return if the soundblaster was there and the song is playing
*/
bool playSong(const char *fileName);

/**
* This function stops the currently playing .sa2 song from !SurpriseProductions Adlib Tracker 2.0
*
* @example stopSong()
* @return if the song was playing and thus is now stopped
*/
bool stopSong();

#endif //__SOUND_HPP__
