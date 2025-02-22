#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <system.h>
#include <sys/alt_alarm.h>
#include <io.h>
#include "fatfs.h"
#include "diskio.h"
#include "ff.h"
#include "monitor.h"
#include "uart.h"
#include "alt_types.h"
#include <altera_up_avalon_audio.h>
#include <altera_up_avalon_audio_and_video_config.h>
#include <unistd.h>

char *ptr;
DIR Dir;
long p1, s1, s2;
char song_name[20][20];
unsigned long song_size[20];
FRESULT res;
FILINFO Finfo;
FATFS Fatfs[_VOLUMES];

static void put_rc(FRESULT rc)
{
    const char *str =
        "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
        "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
        "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
        "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
    FRESULT i;

    for (i = 0; i != rc && *str; i++) {
        while (*str++);
    }
    xprintf("rc=%u FR_%s\n", (uint32_t) rc, str);
}

int isWav(char *filename) {
	// split at last occurrence
	char *dot = strrchr(filename, '.');
	if (dot != NULL) {
		return !strncmp(".WAV", dot, 4);
	}
	return 0;

}
int main()
{
xprintf("rc=%d\n", (uint16_t) disk_initialize((uint8_t) p1));
put_rc(f_mount((uint8_t) p1, &Fatfs[p1]));
  while (*ptr == ' ')
      ptr++;
  res = f_opendir(&Dir, ptr);
  if (res) // if res in non-zero there is an error; print the error.
  {
      put_rc(res);
  }
  p1 = s1 = s2 = 0; // otherwise initialize the pointers and proceed.
  int s_index = 0;
  for (;;)
  {
      res = f_readdir(&Dir, &Finfo);
      if ((res != FR_OK) || !Finfo.fname[0])
          break;
      if(isWav(Finfo.fname)) {
    	  song_size[s_index] = Finfo.fsize;
    	  memcpy(song_name[s_index], Finfo.fname, strlen(Finfo.fname));
    	  s_index++;
      } else {
    	  continue;
      }
      if (Finfo.fattrib & AM_DIR)
      {
          s2++;
      }
      else
      {
          s1++;
          p1 += Finfo.fsize;
      }
//      xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
//              (Finfo.fattrib & AM_DIR) ? 'D' : '-',
//              (Finfo.fattrib & AM_RDO) ? 'R' : '-',
//              (Finfo.fattrib & AM_HID) ? 'H' : '-',
//              (Finfo.fattrib & AM_SYS) ? 'S' : '-',
//              (Finfo.fattrib & AM_ARC) ? 'A' : '-',
//              (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
//              (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, Finfo.fsize, &(Finfo.fname[0]));
#if _USE_LFN
      for (p2 = strlen(Finfo.fname); p2 < 14; p2++)
          xputc(' ');
      xprintf("%s\n", Lfname);
#else
      xputc('\n');
#endif
  }
  int i;
  for (i = 0; i < s_index; i++) {
	  printf("%s\n", song_name[i]);
	  printf("%l\n", song_size[i]);
  }
  return 0;
}
