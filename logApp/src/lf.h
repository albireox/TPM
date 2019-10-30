/*==============================================================================

  L F . H

  This header is an internal header to the log file library.

  $Id: lf.h,v 1.1.1.1 2000/01/27 21:18:18 cvsuser Exp $
==============================================================================*/
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "logfile.h"

#if defined(_VXWORKS) || defined(__sgi)
#include <netinet/in.h>			/* for htonl, ntohl etc */
#else
#include <sys/param.h>
#endif

enum LogMode { modeInvalid, modeRead, modePreWrite, modeWrite };

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  struct LogRegister

  This structure defines the information needed to express a register
  of a log file. A "register" is a data type that allows access to a
  data point that has been sampled and saved in a log file.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
struct _LogRegister {
	unsigned long period;
	char name[REG_NAME_LEN + 1];
	unsigned char type;
	char descr[106];
	union {
		u_int8_t ui1;
		int8_t i8;
		u_int8_t ui8;
		int16_t i16;
		u_int16_t ui16;
		int32_t i32;
		u_int32_t ui32;
		double d;
		void* b;
	} data;
};

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  struct FileHeader

  This structure defines the layout of the first sizeof(FileHeader)
  bytes of the log file. This structure has been carefully laid out so
  that alignment problems for different hardware platforms has been
  minimized.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
struct FileHeader {
	unsigned long signature;
	unsigned short version;
	unsigned short saveRate;
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	
	unsigned char padding0;

	char description[1024];
	unsigned short nReg;

	unsigned short padding1;
};

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  struct FileRegister

  This is the layout of the register, as seen on the disk.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
struct FileRegister {
	unsigned long period;
	char name[17];
	unsigned char type;
	char descr[106];
};

/*::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  struct LogFile

  This structure defines the information need to describe the state of
  a log file.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
struct _LogFile {
	time_t baseTime;
	unsigned short saveRate;
	enum LogMode mode;
	FILE* fp;

	char description[1024];

	unsigned long maxIndex;

	unsigned long tick;

	unsigned short nReg;
	unsigned short totReg;
	HLOGREG reg;
};

/*
 *		Internal function prototypes...
 */
int regDataSize(HLOGREG);
int regRead(HLOGREG, FILE*);
int regWrite(HLOGREG, FILE*);
