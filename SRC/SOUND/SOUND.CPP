#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "../STL/SNDBLSTR.HPP"
#include "../STL/SA2.HPP"

volatile bool adlib_playSong = false;
SBL_POINTER adlib_sampleBuffer;
unsigned char *adlib_songData;
unsigned int adlib_songDataSize;

// No timer interrupt speed manipulations, so it doesn't interfere with the DOS/BIOS harddrive stuff at all..

// the pragma off and on is needed by watcomc, it may throw a (malicious) StackOverflow error without it
#pragma off(check_stack)
void soundBlasterCallback(unsigned char *buffer, unsigned int bufferLength, unsigned int playPos) {
  for (int i = 0; i < bufferLength; i++) SBL_WriteSample8(buffer, i, 0);
  if (adlib_playSong)
    play_line();
}
#pragma on(check_stack)

/**
* This function loads a .sa2 song from !SurpriseProductions Adlib Tracker 2.0 and plays it
*
* @param name The file name of the *.sa2 to load and play.
* @example playSong("DATA/SOUND/TEST.SA2")
* @return if the soundblaster was there and the song is playing
*/
bool playSong(const char *fileName) {

  adlib_playSong = false;
  if (!SBL_Init()) return false;

  FILE *in = fopen(fileName,"rb");
  if (in == NULL) {
    printf("File not found:<%s>\n",fileName);
    exit(0);
  }
  fseek(in,0,SEEK_END);
  adlib_songDataSize = ftell(in);
  fseek(in,0,SEEK_SET);
  adlib_songData = (unsigned char*)malloc(adlib_songDataSize);
  fread(adlib_songData,1,adlib_songDataSize,in);
  fclose(in);

  SBL_LockMemory((void*)&adlib_playSong, 4);

  adlibTrackerLock(adlib_songData,adlib_songDataSize); 
  prepare_song(adlib_songData);
  SBL_SampleRate = 22222;
  unsigned int bufferParts = 2;
  unsigned int bufferSize = SBL_SampleRate*60/(current_bpm*24)*bufferParts;

  adlib_sampleBuffer = SBL_Malloc(bufferSize);
  SBL_StartPlayback( &adlib_sampleBuffer, bufferSize,bufferParts,soundBlasterCallback);
  SBL_SetSampleVolume(0x44); // we need to reduce sample volume on SBEMU to hear adlib

  adlib_playSong = true;
  return true;
}

/**
* This function stops the currently playing .sa2 song from !SurpriseProductions Adlib Tracker 2.0
*
* @example stopSong()
* @return if the song was playing and thus is now stopped
*/
bool stopSong() {
  if (!adlib_playSong) return false;
  adlib_playSong = false;
  stop_all();
  SBL_Done();
  SBL_Free(&adlib_sampleBuffer);
  adlibTrackerUnlock(adlib_songData,adlib_songDataSize); free(adlib_songData);
  SBL_UnlockMemory((void*)&adlib_playSong, 4);
  return true;
}
