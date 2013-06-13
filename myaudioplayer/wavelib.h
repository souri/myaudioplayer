/***********************************************************************
 * testaudio.h
 *  
 *    Audio Library
 *
 *
 * Toby Opferman Copyright (c) 2003
 *
 ***********************************************************************/


#ifndef __WAVELIB_H__
#define __WAVELIB_H__


typedef PVOID HWAVELIB;

//#ifdef __cplusplus
//extern "C" {
//#endif


extern HWAVELIB WaveLib_Init(PCHAR pAudioFile, BOOL bPause);
extern void WaveLib_UnInit(HWAVELIB hWaveLib);
extern void WaveLib_Pause(HWAVELIB hWaveLib, BOOL bPause);
extern int GetSize();
extern int GetLength();
extern float GetPlayTime(HWAVELIB hWaveLib);
extern float WaveLib_PlaybackTime(HWAVEOUT hWaveOut);
extern void WaveTest_Play(char *pszFileName);
extern int PlaybackTimePercentage(int FileLength, int PlaybackTime);

//#ifdef __cplusplus
//}
//#endif



#endif

