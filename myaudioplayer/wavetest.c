 #include <windows.h>
 #include <stdio.h>
 #include <conio.h>
 #include "wavelib.h"


/************************************************************************
 * Prototypes 
 ************************************************************************/
 void WaveTest_Play();
 void WaveTest_PrintArgs(void);
 void WaveTest_PrintError(void);
 void readPlaylist(char *pszFileName);
 int PlaybackPercentage;

 int FileSize;
 int FileLength;
 int PlaybackTime;
 int getPlaybackTimePercentage(int FileLength, int PlabackTime);

 DWORD volume;
 
 /***********************************************************************
  * main()
  * Parameters
  *     Number Of Arguements, Arguements
  * 
  * Return Value
  *     0
  ***********************************************************************/
  
  int main(int argc, char *argv[])
  {
      if(argc == 2)
      {

          WaveTest_Play(argv[1]);
      }
      else
      {
          WaveTest_PrintArgs();
      }

      return 0;
  }
 
 //int main(int argc, char *argv[])
 // {
 //     if(argc == 2)
 //     {
 //         WaveTest_Play(argv[1]);
 //     }
 //     else
 //     {
 //         WaveTest_PrintArgs();
 //     }

 //     return 0;
 // }


 /***********************************************************************
  * WaveTest_Play
  *  
  *    Play A Wave File
  *
  * Parameters
  *     File Name
  * 
  * Return Value
  *     0
  *
  ***********************************************************************/
 void WaveTest_Play(char *pszFileName)
 {
     HWAVELIB hWaveLib = NULL;
         
     hWaveLib = WaveLib_Init(pszFileName, FALSE);
     FileSize = GetSize();
	 FileLength = GetLength();

	 if(hWaveLib)
	 {
		 do
		 {
			 PlaybackTime =GetPlayTime(hWaveLib);
			 PlaybackPercentage = getPlaybackTimePercentage(FileLength, PlaybackTime);
			 if (PlaybackPercentage >= 0 && PlaybackPercentage < 10)
			 {

			 }
			 else if (PlaybackPercentage >= 10 && PlaybackPercentage < 20)
			 {
			 }
			 else if (PlaybackPercentage >= 30 && PlaybackPercentage < 40)
			 {
			 }
			 else if (PlaybackPercentage >= 40 && PlaybackPercentage < 60)
			 {
			 }
			 else if (PlaybackPercentage >= 60 && PlaybackPercentage < 70)
			 {
			 }
			 else if (PlaybackPercentage >= 80 && PlaybackPercentage < 99)
			 {
			 }


		 }

		 while(PlaybackTime < FileLength);
		 printf("Playbacktime: %d | FileLength: %d",PlaybackTime,FileLength);
		 WaveLib_UnInit(hWaveLib);
	 }
	 else
	 {
		 WaveTest_PrintError();
	 }
   //  if(hWaveLib)
   //  {
   //      printf(" Press a key to stop> \n");
   //      _getch();
		 //WaveLib_UnInit(hWaveLib);
   //  }
   //  else
   //  {
   //      WaveTest_PrintError();
   //  }
     
 }

 void WaveTest_PrintArgs(void)
 {
     printf("WaveTest .WAV File Player!\n");

     printf("Usage:\n");
     printf("\n   WaveTest <FileName>\n\n");
 }

 void WaveTest_PrintError(void)
 {
     printf(" An Error Occured Attempting to play sound\n");
     WaveTest_PrintArgs();
 }


 void readPlaylist(char *pszFileName)
 {

 }

 int PlaybackTimePercentage(int FileLength, int PlaybackTime)
 {
	 PlaybackPercentage = (PlaybackTime/FileLength)*100;
	 return PlaybackPercentage;


 }

