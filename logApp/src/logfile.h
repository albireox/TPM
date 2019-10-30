/*==============================================================================

  L O G F I L E . H

  $Id: logfile.h,v 1.1.1.1 2000/01/27 21:18:18 cvsuser Exp $
==============================================================================*/

/*
 *		Data types...
 */
#if defined(_VXWORKS)
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif

#define REG_NAME_LEN	16

typedef enum {
	REG_UINT1, REG_INT8, REG_UINT8, REG_INT16, REG_UINT16, REG_INT32,
	REG_UINT32, REG_FLOAT
} RegType;

#define REG_BLOB(s)		((RegType) ((s) >= 0x07f ? 0x0ff : (s) | 0x80))
#define	IS_BLOB(r)		(0 != ((r)->type & 0x80))
#define	BLOB_SIZE(r)	(IS_BLOB(r) ? (r)->type & 0x07f : 0)

struct _LogRegister;
typedef struct _LogRegister* HLOGREG;

struct _LogFile;
typedef struct _LogFile* HLOGFILE;

/*
 *		Function Prototypes...
 */
void logClose(HLOGFILE*);
HLOGFILE logCreate(char const*, unsigned short, unsigned short);
HLOGREG logCreateRegister(HLOGFILE, char const*, char const*, RegType,
						  unsigned long);
time_t logGetBaseTime(HLOGFILE);
char const* logGetDescription(HLOGFILE);
unsigned long logGetLastIndex(HLOGFILE);
HLOGREG logGetNamedRegister(HLOGFILE, char const*);
unsigned logGetSampleRate(HLOGFILE);
HLOGREG logGetRegister(HLOGFILE, unsigned);
unsigned short logGetRegisterCount(HLOGFILE);
int logLoadData(HLOGFILE, unsigned);
int logLoadRegData(HLOGFILE, unsigned, unsigned);
HLOGFILE logOpen(char const*);
int logSaveData(HLOGFILE);
void logSetDescription(HLOGFILE, char const*);

int regDataSize(HLOGREG);
u_int32_t regGet(HLOGREG);
int regGetBlob(HLOGREG, void*, size_t);
double regGetDouble(HLOGREG);
char const* regGetDescription(HLOGREG);
char const* regGetName(HLOGREG);
unsigned long regGetPeriod(HLOGREG);
RegType regGetType(HLOGREG);
int regPut(HLOGREG, u_int32_t);
int regPutBlob(HLOGREG, void const*, size_t);
int regPutDouble(HLOGREG, double);
