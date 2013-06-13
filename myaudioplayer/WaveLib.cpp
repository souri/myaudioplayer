 #include <windows.h>
 #include <mmddk.h>
 #include <Mmsystem.h>
 #include <stdio.h>
 #include <conio.h>
 #include "wavelib.h"
 #include <Mmdeviceapi.h>
 #include <Strsafe.h>
 #include <audiopolicy.h>
 
 
 /**********/
int total_time;
MMTIME playbacktime;
UINT playbacktime_size;
DWORD filesize;

 /***********************************************************************
  * Internal Structures
  ***********************************************************************/
typedef struct {
    
    UCHAR IdentifierString[4];
    DWORD dwLength;

} RIFF_CHUNK, *PRIFF_CHUNK;


typedef struct {
    WORD  wFormatTag;         // Format category
    WORD  wChannels;          // Number of channels
    DWORD dwSamplesPerSec;    // Sampling rate
    DWORD dwAvgBytesPerSec;   // For buffer estimation
    WORD  wBlockAlign;        // Data block size
    WORD  wBitsPerSample;
} WAVE_FILE_HEADER, *PWAVE_FILE_HEADER;

WAVE_FILE_HEADER WaveFileHeader;

typedef struct _wave_sample {
     WAVEFORMATEX WaveFormatEx;
     char *pSampleData;
     UINT Index;
     UINT Size;
     DWORD dwId;
     DWORD bPlaying;
     struct _wave_sample *pNext;
} WAVE_SAMPLE, *PWAVE_SAMPLE;
 
#define SAMPLE_SIZE    (2*2*2000) 

typedef struct {
     
     HWAVEOUT hWaveOut;
     HANDLE hEvent;
     HANDLE hThread;
     WAVE_SAMPLE WaveSample;
     BOOL bWaveShouldDie;
     WAVEHDR WaveHdr[8];
     char AudioBuffer[8][SAMPLE_SIZE];
     BOOL bPaused;
} WAVELIB, *PWAVELIB;

int FileSize;
int FileLength;
float PlaybackTimeNow;
void getPlaybackTimePercentage(int FileLength, int PlaybackTime);

 /***********************************************************************
  * Internal Functions
  ***********************************************************************/
void CALLBACK WaveLib_WaveOutputCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
BOOL WaveLib_OpenWaveSample(CHAR *pFileName, PWAVE_SAMPLE pWaveSample);
void WaveLib_WaveOpen(HWAVEOUT hWaveOut, PWAVELIB pWaveLib);
void WaveLib_WaveDone(HWAVEOUT hWaveOut, PWAVELIB pWaveLib);
DWORD WINAPI WaveLib_AudioThread(PVOID pDataInput);
void WaveLib_CreateThread(PWAVELIB pWaveLib);
void WaveLib_SetupAudio(PWAVELIB pWaveLib);
void WaveLib_WaveClose(HWAVEOUT hWaveOut, PWAVELIB pWaveLib);
void WaveLib_AudioBuffer(PWAVELIB pWaveLib, UINT Index);
void Wavelib_Playbacktime(HWAVEOUT hWaveOut);
int GetSize();
int GetLength();
void showbar();
void loadbar(int x, int n, int r, int w);
 void WaveTest_Play();
 void WaveTest_PrintArgs(void);
 void WaveTest_PrintError(void);
 void readPlaylist(char *pszFileName);
 float PlaybackPercentage = 0;
 DWORD volume;
 DWORD nextvolume;
 DWORD playbackrate;
 MMRESULT result;
 WAVEOUTCAPS devicecapability;

 /***********************************************************************
  * WaveLib_Init
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 HWAVELIB WaveLib_Init(PCHAR pWaveFile, BOOL bPause)
 {
     PWAVELIB pWaveLib = NULL;
 
     if(pWaveLib = (PWAVELIB)LocalAlloc(LMEM_ZEROINIT, sizeof(WAVELIB)))
     {
         pWaveLib->bPaused = bPause;

         if(WaveLib_OpenWaveSample(pWaveFile, &pWaveLib->WaveSample))
         {
             if(waveOutOpen(&pWaveLib->hWaveOut, WAVE_MAPPER, &pWaveLib->WaveSample.WaveFormatEx, (ULONG)WaveLib_WaveOutputCallback, (ULONG)pWaveLib, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
             {
                WaveLib_UnInit((HWAVELIB)pWaveLib);
                pWaveLib = NULL;
             }
             else
             {
 
                 if(pWaveLib->bPaused)
                 {
                     waveOutPause(pWaveLib->hWaveOut);
                 }

                 WaveLib_CreateThread(pWaveLib);
             }
         }
         else
         {
             WaveLib_UnInit((HWAVELIB)pWaveLib);
             pWaveLib = NULL;
         }
     }

     return (HWAVELIB)pWaveLib;
 }


  /***********************************************************************
  * WaveLib_Pause
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 void WaveLib_Pause(HWAVELIB hWaveLib, BOOL bPause)
 {
     PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;

     pWaveLib->bPaused = bPause;

     if(pWaveLib->bPaused)
     {
         waveOutPause(pWaveLib->hWaveOut);
     }
     else
     {
         waveOutRestart(pWaveLib->hWaveOut);
     }
 }
 
 /***********************************************************************
  * WaveLib_WaveOutputCallback
  *  
  *    Audio Callback 
  *
  * Parameters
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/ 
void CALLBACK WaveLib_WaveOutputCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    PWAVELIB pWaveLib = (PWAVELIB)dwInstance;

    switch(uMsg)
    {
      case WOM_OPEN:
            WaveLib_WaveOpen(hwo, pWaveLib);
            break;

       case WOM_DONE:
            WaveLib_WaveDone(hwo, pWaveLib);
            break;

       case WOM_CLOSE:
            WaveLib_WaveClose(hwo, pWaveLib);
            break;
    }
}
 /***********************************************************************
  * WaveLib_WaveOpen
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_WaveOpen(HWAVEOUT hWaveOut, PWAVELIB pWaveLib)
{
  // Do Nothing
}


 /***********************************************************************
  * WaveLib_WaveDone
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_WaveDone(HWAVEOUT hWaveOut, PWAVELIB pWaveLib)
{
    SetEvent(pWaveLib->hEvent);
}


 /***********************************************************************
  * WaveLib_WaveClose
  *  
  *    Audio Callback 
  * Parameters
  * Return Value
  *     Handle To This Audio Session
  ***********************************************************************/
void WaveLib_WaveClose(HWAVEOUT hWaveOut, PWAVELIB pWaveLib)
{
  // Do Nothing
}

BOOL WaveLib_OpenWaveSample(CHAR *pFileName, PWAVE_SAMPLE pWaveSample)
{
    BOOL bSampleLoaded = FALSE;
    HANDLE hFile;
    RIFF_CHUNK RiffChunk = {0};
    DWORD dwBytes, dwReturnValue;
   
    DWORD dwIncrementBytes;

    if(hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL))
    {
        char szIdentifier[5] = {0};

        SetFilePointer(hFile, 12, NULL, FILE_CURRENT);
        filesize = GetFileSize(hFile,NULL);

        ReadFile(hFile, &RiffChunk, sizeof(RiffChunk), &dwBytes, NULL);
        ReadFile(hFile, &WaveFileHeader, sizeof(WaveFileHeader), &dwBytes, NULL);

        pWaveSample->WaveFormatEx.wFormatTag      = WaveFileHeader.wFormatTag;         
        pWaveSample->WaveFormatEx.nChannels       = WaveFileHeader.wChannels;          
        pWaveSample->WaveFormatEx.nSamplesPerSec  = WaveFileHeader.dwSamplesPerSec;    
        pWaveSample->WaveFormatEx.nAvgBytesPerSec = WaveFileHeader.dwAvgBytesPerSec;   
        pWaveSample->WaveFormatEx.nBlockAlign     = WaveFileHeader.wBlockAlign;  
        pWaveSample->WaveFormatEx.wBitsPerSample  = WaveFileHeader.wBitsPerSample;
        pWaveSample->WaveFormatEx.cbSize          = 0;

        dwIncrementBytes = dwBytes;

		printf("FileSize: %dbytes\n",filesize);
		printf("wFormatTag: %d\n",pWaveSample->WaveFormatEx.wFormatTag);
		printf("nSamplesPerSec: %d\n",pWaveSample->WaveFormatEx.nSamplesPerSec);
		printf("wBitsPerSample: %d\n",pWaveSample->WaveFormatEx.wBitsPerSample);
		printf("nChannels: %d\n",pWaveSample->WaveFormatEx.nChannels);
		printf("AvgBytesPerSec: %dbps\n",pWaveSample->WaveFormatEx.nAvgBytesPerSec);
		printf("BlockAlign %d\n",pWaveSample->WaveFormatEx.nBlockAlign);
		printf("RiffChunk.dwLength %d\n",sizeof(RiffChunk));

		total_time = (filesize-44)/(pWaveSample->WaveFormatEx.nAvgBytesPerSec); //wavsize/Bps = tot play time
		printf("Total Length of WAV File: %d seconds\n", total_time);

        do {
             SetFilePointer(hFile, RiffChunk.dwLength - dwIncrementBytes, NULL, FILE_CURRENT);
             
             dwReturnValue = GetLastError();

             if(dwReturnValue == 0)
             {
                 dwBytes = ReadFile(hFile, &RiffChunk, sizeof(RiffChunk), &dwBytes, NULL);
             
                 dwIncrementBytes = 0;

                 memcpy(szIdentifier, RiffChunk.IdentifierString, 4); 
             }

        } while(_stricmp(szIdentifier, "data") && dwReturnValue == 0) ;

        if(dwReturnValue == 0)
        {
            pWaveSample->pSampleData = (char *)LocalAlloc(LMEM_ZEROINIT, RiffChunk.dwLength);

            pWaveSample->Size = RiffChunk.dwLength;

            ReadFile(hFile, pWaveSample->pSampleData, RiffChunk.dwLength, &dwBytes, NULL);

            CloseHandle(hFile);

            bSampleLoaded = TRUE;
        }
    }

    return bSampleLoaded;
}
 /***********************************************************************
  * WaveLib_CreateThread
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_CreateThread(PWAVELIB pWaveLib)
{
    DWORD dwThreadId;

    pWaveLib->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    pWaveLib->hThread = CreateThread(NULL, 0, WaveLib_AudioThread, pWaveLib, 0, &dwThreadId);
}
 /***********************************************************************
  * WaveLib_AudioThread
  *  
  *    Audio Thread
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
DWORD WINAPI WaveLib_AudioThread(PVOID pDataInput)
{
    PWAVELIB pWaveLib = (PWAVELIB)pDataInput;
    DWORD dwReturnValue = 0;
    UINT Index;

    WaveLib_SetupAudio(pWaveLib);

    while(!pWaveLib->bWaveShouldDie)
    {
        WaitForSingleObject(pWaveLib->hEvent, INFINITE);

        for(Index = 0; Index < 8; Index++)
        {
            if(pWaveLib->WaveHdr[Index].dwFlags & WHDR_DONE)
            {
               WaveLib_AudioBuffer(pWaveLib, Index);
               waveOutWrite(pWaveLib->hWaveOut, &pWaveLib->WaveHdr[Index], sizeof(WAVEHDR));
            }
        }
    }

    waveOutReset(pWaveLib->hWaveOut);

    return dwReturnValue;
}

void WaveLib_AudioBuffer(PWAVELIB pWaveLib, UINT Index)
{
    UINT uiBytesNotUsed = SAMPLE_SIZE;

    pWaveLib->WaveHdr[Index].dwFlags &= ~WHDR_DONE;

    if(pWaveLib->WaveSample.Size - pWaveLib->WaveSample.Index < uiBytesNotUsed)
    {
        memcpy(pWaveLib->AudioBuffer[Index], pWaveLib->WaveSample.pSampleData + pWaveLib->WaveSample.Index, pWaveLib->WaveSample.Size - pWaveLib->WaveSample.Index);

        uiBytesNotUsed -= (pWaveLib->WaveSample.Size - pWaveLib->WaveSample.Index);

        memcpy(pWaveLib->AudioBuffer[Index], pWaveLib->WaveSample.pSampleData, uiBytesNotUsed);

        pWaveLib->WaveSample.Index = uiBytesNotUsed;

        uiBytesNotUsed = 0;
    }
    else
    {
       memcpy(pWaveLib->AudioBuffer[Index], pWaveLib->WaveSample.pSampleData + pWaveLib->WaveSample.Index, uiBytesNotUsed);

       pWaveLib->WaveSample.Index += SAMPLE_SIZE;
       uiBytesNotUsed = 0;
    }

    pWaveLib->WaveHdr[Index].lpData = pWaveLib->AudioBuffer[Index];

    pWaveLib->WaveHdr[Index].dwBufferLength = SAMPLE_SIZE - uiBytesNotUsed;
}
 /***********************************************************************
  * WaveLib_SetupAudio
  *  
  *    Audio Thread
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void WaveLib_SetupAudio(PWAVELIB pWaveLib)
{
    UINT Index = 0;

    for(Index = 0; Index < 8; Index++)
    {
        pWaveLib->WaveHdr[Index].dwBufferLength = SAMPLE_SIZE;
        pWaveLib->WaveHdr[Index].lpData         = pWaveLib->AudioBuffer[Index]; 

        waveOutPrepareHeader(pWaveLib->hWaveOut, &pWaveLib->WaveHdr[Index], sizeof(WAVEHDR));

        WaveLib_AudioBuffer(pWaveLib, Index);

        waveOutWrite(pWaveLib->hWaveOut, &pWaveLib->WaveHdr[Index], sizeof(WAVEHDR));
    }
}

float WaveLib_PlaybackTime(HWAVEOUT hWaveOut)
{
	//PWAVELIB pWaveLib = NULL;
	FLOAT time;
	//int total_playtime = WaveFileHeader.

	playbacktime.wType = TIME_MS;				 //Make sure time is in the smpte section of the mmtime struct
	playbacktime_size = sizeof(playbacktime);
	waveOutGetPosition(hWaveOut, &playbacktime, playbacktime_size);

	time = (float)playbacktime.u.ms / 100000;
	//printf("Elapsed Time: %0.4f",time);
	return time;
	//printf("Elapsed Time: %d:%",playbacktime.u.smpte.min, playbacktime.u.smpte.sec);
}

 /***********************************************************************
  * WaveLib_Init
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 void WaveLib_UnInit(HWAVELIB hWaveLib)
 {
     PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;
	 WaveLib_PlaybackTime(pWaveLib->hWaveOut);
     if(pWaveLib)
     {
         if(pWaveLib->hThread)
         {
             pWaveLib->bWaveShouldDie = TRUE;

             SetEvent(pWaveLib->hEvent);
             WaitForSingleObject(pWaveLib->hThread, INFINITE);

             CloseHandle(pWaveLib->hEvent);
             CloseHandle(pWaveLib->hThread);
         }

         if(pWaveLib->hWaveOut)
         {
             waveOutClose(pWaveLib->hWaveOut);
         }


         if(pWaveLib->WaveSample.pSampleData)
         {
             LocalFree(pWaveLib->WaveSample.pSampleData);
         }
		 LocalFree(pWaveLib);
     }

 }
 
 int GetSize()
 {
	 return filesize;
 }

 int GetLength()
 {
	 return total_time;
 }
 
float GetPlayTime(HWAVELIB hWaveLib)
 {
	 float time = 0;
	 PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;
	 time = WaveLib_PlaybackTime(pWaveLib->hWaveOut);
	 return time;
 }

//-----------------------------------------------------------
// This function gets the waveOut ID of the audio endpoint
// device that is currently assigned to the specified device
// role. The caller can use the waveOut ID to open the
// waveOut device that corresponds to the endpoint device.
//-----------------------------------------------------------
#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

HRESULT GetWaveOutId(ERole role, int *pWaveOutId)
{
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    WCHAR *pstrEndpointIdKey = NULL;
    WCHAR *pstrEndpointId = NULL;

    if (pWaveOutId == NULL)
    {
        return E_POINTER;
    }

    // Create an audio endpoint device enumerator.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                          NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    // Get the audio endpoint device that the user has
    // assigned to the specified device role.
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, role,
                                              &pDevice);
    EXIT_ON_ERROR(hr)

    // Get the endpoint ID string of the audio endpoint device.
    hr = pDevice->GetId(&pstrEndpointIdKey);
    EXIT_ON_ERROR(hr)

    // Get the size of the endpoint ID string.
    size_t  cbEndpointIdKey;

    hr = StringCbLength((STRSAFE_PCNZCH)pstrEndpointIdKey,  STRSAFE_MAX_CCH * sizeof(WCHAR),
                        &cbEndpointIdKey);
    EXIT_ON_ERROR(hr)

    // Include terminating null in string size.
    cbEndpointIdKey += sizeof(WCHAR);

    // Allocate a buffer for a second string of the same size.
    pstrEndpointId = (WCHAR*)CoTaskMemAlloc(cbEndpointIdKey);
    if (pstrEndpointId == NULL)
    {
        EXIT_ON_ERROR(hr = E_OUTOFMEMORY)
    }

    // Each for-loop iteration below compares the endpoint ID
    // string of the audio endpoint device to the endpoint ID
    // string of an enumerated waveOut device. If the strings
    // match, then we've found the waveOut device that is
    // assigned to the specified device role.
    int waveOutId;
    int cWaveOutDevices = waveOutGetNumDevs();

    for (waveOutId = 0; waveOutId < cWaveOutDevices; waveOutId++)
    {
        MMRESULT mmr;
        size_t cbEndpointId;

        // Get the size (including the terminating null) of
        // the endpoint ID string of the waveOut device.
        mmr = waveOutMessage((HWAVEOUT)IntToPtr(waveOutId),
                             DRV_QUERYFUNCTIONINSTANCEIDSIZE,
                             (DWORD_PTR)&cbEndpointId, NULL);
        if (mmr != MMSYSERR_NOERROR ||
            cbEndpointIdKey != cbEndpointId)  // do sizes match?
        {
            continue;  // not a matching device
        }

        // Get the endpoint ID string for this waveOut device.
        mmr = waveOutMessage((HWAVEOUT)IntToPtr(waveOutId),
                             DRV_QUERYFUNCTIONINSTANCEID,
                             (DWORD_PTR)pstrEndpointId,
                             cbEndpointId);
        if (mmr != MMSYSERR_NOERROR)
        {
            continue;
        }

        // Check whether the endpoint ID string of this waveOut
        // device matches that of the audio endpoint device.
        if (lstrcmpi((LPCSTR)pstrEndpointId, (LPCSTR)pstrEndpointIdKey) == 0)
        {
            *pWaveOutId = waveOutId;  // found match
            hr = S_OK;
            break;
        }
    }

    if (waveOutId == cWaveOutDevices)
    {
        // We reached the end of the for-loop above without
        // finding a waveOut device with a matching endpoint
        // ID string. This behavior is quite unexpected.
        hr = E_UNEXPECTED;
    }

Exit:
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pDevice);
    CoTaskMemFree(pstrEndpointIdKey);  // NULL pointer okay
    CoTaskMemFree(pstrEndpointId);
    return hr;
}


void WaveTest_Play(char *pszFileName)
 {
	 HRESULT WaveIdResult;
     HWAVELIB hWaveLib = WaveLib_Init(pszFileName, FALSE);
	 PWAVELIB pWaveLib = (PWAVELIB)hWaveLib;
	 int x;
	 int WaveOutId = 0;
	 float mastervolume;
	 // hWaveLib = WaveLib_Init(pszFileName, FALSE);
     FileSize = GetSize();
	 FileLength = GetLength();
	 //IAudioSessionManager xxx;
	 WaveIdResult = IAudioSessionManager::GetAudioSessionControl (GUID_NULL,FALSE);
	 //waveOutGetDevCaps(&pWaveLib->hThread,&devicecapability, sizeof(devicecapability));
	 printf("WaveOutID before: %d\n", WaveOutId);
	 //WaveIdResult = GetWaveOutId(eMultimedia, &WaveOutId);

	 printf("WaveOutID: %d | Status: %x\n", WaveOutId, WaveIdResult);
	 if(hWaveLib)
	 {
		 do
		 {
			PlaybackTimeNow =GetPlayTime(hWaveLib);
			PlaybackPercentage = (PlaybackTimeNow/total_time)*100;
			//printf("Querying device capabilities...\n");

			//loadbar(PlaybackTimeNow,total_time,10,10);
			//getPlaybackTimePercentage(FileLength, PlaybackTimeNow);
			showbar();
			
			if (PlaybackPercentage >= 0 && PlaybackPercentage < 10)
			 {
				 //waveOutGetVolume(pWaveLib->hWaveOut,&volume);
				 nextvolume = 0x0000;
				 nextvolume = ((int)PlaybackPercentage*25)<<8;
				 waveOutSetVolume(pWaveLib->hWaveOut,nextvolume);
				 printf("Ch1++|Vol:%X|", nextvolume);
			 }
			 else if (PlaybackPercentage >= 10 && PlaybackPercentage < 20)
			 {
				 nextvolume = 0xFF00 -  (((int)(PlaybackPercentage-10)*25)<<8);
				 waveOutSetVolume(pWaveLib->hWaveOut,nextvolume);
				 printf("Ch1--|Vol:%X|", nextvolume);
			 }
			 else if (PlaybackPercentage >= 20 && PlaybackPercentage < 30)
			 {
				 nextvolume = 0x0000;
				 nextvolume = ((int)(PlaybackPercentage-20)*25);
				 waveOutSetVolume(pWaveLib->hWaveOut,nextvolume);
				 printf("Ch2++|Vol:00%X|", nextvolume);
			 }
			 else if (PlaybackPercentage >= 30 && PlaybackPercentage < 40)
			 {
				 nextvolume = 0x00FF - ((int)(PlaybackPercentage-30)*25);
				 waveOutSetVolume(pWaveLib->hWaveOut,nextvolume);
				 printf("Ch2--|Vol:00%X|", nextvolume);
			 }
			 else if (PlaybackPercentage >= 40 && PlaybackPercentage < 50)
			 {
				 nextvolume = 0x0000;
				 nextvolume = (((int)(PlaybackPercentage-40)*25<<8)) | ((int)(PlaybackPercentage-40)*25);
				 waveOutSetVolume(pWaveLib->hWaveOut,nextvolume);
				 printf("Ch1&2++|Vol:%X|", nextvolume);
			 }
			 else if (PlaybackPercentage >= 50 && PlaybackPercentage < 70)
			 {
				 playbackrate = ((((int)(PlaybackPercentage-50)*25)/64)<<16);
				 result = waveOutSetPlaybackRate(pWaveLib->hWaveOut,playbackrate);
				 if (result == MMSYSERR_INVALHANDLE){
					 printf("nohandle");}
				 else if (result == MMSYSERR_NODRIVER)
					 printf("nodriver");
				 else if (result == MMSYSERR_NOMEM)
					 printf("nomem");
				 else if (result == MMSYSERR_NOTSUPPORTED){
					 printf("notsup");//printf("WARNING: Changing playback rate unsupported.\n");
				 }
				 else if (result == MMSYSERR_NOERROR);
					 printf("@%dx|", (((int)(PlaybackPercentage-50)*25)/64));
			 }
			 else if (PlaybackPercentage >= 70 && PlaybackPercentage < 90)
			 {
				 //playbackrate = ((((int)(PlaybackPercentage-70)*25)/64)<<16);
				 waveOutGetPlaybackRate(pWaveLib->hWaveOut,&playbackrate);
				 //printf("@%dx|", (((int)(PlaybackPercentage-50)*25)/64));
				 printf("@%Xx|", playbackrate);
			 }
		 }

		 while(PlaybackTimeNow < FileLength);
		 printf("Playbacktime: %d | FileLength: %d",PlaybackTimeNow,total_time);
		 WaveLib_UnInit(hWaveLib);
	 }
	 else
	 {
		 WaveTest_PrintError();
	 }
     
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

//void getPlaybackTimePercentage(int FileLength, int PlaybackTimeNow)
// {
//	 PlaybackPercentage = (PlaybackTimeNow/FileLength)*100;
//	
// }

// Process has done i out of n rounds,
// and we want a bar of width w and resolution r.
//void loadbar(float x, int n, int r, int w)
//{
//	float ratio = x/n;
//	int   c = ratio * w;
//
//	/*if (x % (n/r) != 0 ) 
//	{return;}*/
//	printf("Ratio: %d", (int)ratio);
//	printf("%0.1f [",ratio*100);
//	
//	for (x=0;x<c;x++)
//	{
//		printf("=");
//	}
//	for (x=c;x<w;x++)
//	{
//		printf(" ");
//	}
//
//	//printf("]\n33[F33[J");
//	printf("\r");
//	//fflush(stdout);
//}

void showbar()
{
	int x;
	printf("Play:%0.1f/100[",PlaybackPercentage);
			for (x=0;x<(PlaybackTimeNow/total_time)*30;x++)
				printf(":");
			for (x=(PlaybackTimeNow/total_time)*10;x<30;x++)
				printf(" ");
			printf("\r");
}


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
