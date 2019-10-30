/*
static char rcsid[] = "$Id: drvAscii.h,v 1.1 2000/04/20 01:37:13 peregrin Exp $";
*/
/*
 *
 *
 * Author: Allan Honey (for the KECK Observatory)
 * Date: 7-02-96
 *
 */

#define LOCAL STATIC
#define STATIC static  

/*
 * created by drvCreateSioLink()
 */
typedef const void	*drvSioLinkId;

/* 
 * ========================
 * called by device support
 * ========================
 */

#define MAX_STRING_SIZE 40

#define RBF_UNDEFINED 0
#define RBF_STRING 1
#define RBF_INT    2
#define RBF_REAL   3
#define RBF_BINARY 4

typedef struct drv_response_info {
  unsigned int        kill,
                      killAll;

  long                dataType;
                    
  long                killCnt,
                      dataCnt,
                      recordCnt,
                      arrayCnt,
                      preAmbleCnt;

  char                *delimiters,
                      *dataFormat,
                      *preAmble;

} drvResponseInfo;

typedef struct drv_cmnd_arg {
  long                funcArg;

  drvResponseInfo     cmndPromptInfo;
  drvResponseInfo     cmndInfo;

  drvResponseInfo     rbvPromptInfo;
  drvResponseInfo     rbvInfo;

  char 		      cmndPrompt[MAX_STRING_SIZE]; 
  char                cmndFormat[MAX_STRING_SIZE];
  char                rbvPrompt[MAX_STRING_SIZE];
  char                rbvFormat[MAX_STRING_SIZE];
} drvCmndArg;


/*
 * link private structure
 */
typedef struct {
  ELLNODE	  node;
  ELLLIST 	  remoteDevList;
  SEM_ID	  mutex;
  SEM_ID	  syncLock;
  SEM_ID	  sendBlockSem;
  char		  fileName[0x100];
  int		  taskId;
  drvSerialLinkId linkId;
}drvSioLinkPrivate;

typedef struct drv_sio_link_state {
  unsigned	noComm:1;
} drvSioLinkState;

typedef struct drv_app_private {
  ELLNODE		node;
  drvSioLinkPrivate	*pLink;
  SEM_ID		mutexSem;
  drvSioLinkState       state;
  unsigned		reset;
  long                  queryFailures;
  long                  dataErrors;
  long                  responseTMO;
  float                 slope;        /* used for converting float to int */
				      /* and int to float */

  long                  recordCnt;

  long                  debugFlag;

  char                  writeCmt[MAX_STRING_SIZE+1]; /* output string command terminator */
  char                  readCmt[MAX_STRING_SIZE+1];  /* input string command terminator */

  void                  *appPrivate;
}drvAppPrivate;

typedef void drvAsyncUpdateCallBack(void *pArg, long status, long value);

/*
 * parameter private structure
 */
typedef struct {
  ELLNODE		     node;
  drvAppPrivate	            *pAppPriv;
  struct cmd_table_entry    *pCmndTableEntry;

  drvCmndArg                *pCmndArg;

  drvAsyncUpdateCallBack    *pCB;	/* called on parameter change */
  const void 		    *pArg;	/* parameter to above call back */	
  long                       respStrCnt;
  char                       respStr[DRV_SERIAL_BUF_SIZE];

  long                       rbvFlag;
  const void 		    *pRbvArg;	/* parameter to above call back */     
}drvCmndPrivate;

typedef void devIoDoneCallBack(void *pArg, long status, long value);
typedef void devRealIoDoneCallBack(void *pArg, long status, double value);

typedef struct drv_async_io{
  drvCmndPrivate       	id;
  devIoDoneCallBack	*pCB;
  const void		*pArg;
  /*
   * for app private use below this comment
   */
  void			(*pAppDrvPrivate)(struct drv_async_io *pIO);
}drvAsyncIO;
typedef void drvAsciiPrivateCallBack( drvAsyncIO *pio);

typedef long cmdFunc(drvAppPrivate *pAppPriv, void *pData, drvAsyncIO *pIO, drvCmndArg *pFuncParam);

typedef long strCmdFunc(drvAppPrivate *pAppPriv, char *pDataStr, drvAsyncIO *pIO, drvCmndArg *pFuncParam);

typedef long rbvFunc(drvCmndPrivate *pCmndPriv, void *pData, drvAsyncIO *pIO, drvCmndArg *pFuncParam);

typedef struct cmd_table_entry {
  const char	*pCmndName;
  cmdFunc	*pCmdFunc;
  rbvFunc       *pOutReadFunc;
} cmdTableEntry;

long drvAsciiCreateSioLink(
       const char     *pRecType,   /* record type */
       const char     *pFileName,  /* serial port device name */
       drvCmndArg     *pCmndArgs,

       drvCmndPrivate *pId,	  /* for future ref of this link */
       drvAsyncUpdateCallBack *pCB,/* called on parameter change */
       const void     *pArg);      /* parameter to above call back */

/*
 * enable scaning of the device
 * (after all links are created)
 */
long drvAsciiInitiateAll(void);

/*
 * generalized IO to/from a parameter
 * (called by device support)
 */
long drvAsciiIntIo( drvAsyncIO *pIO, long *pValue );
long drvAsciiRealIo( drvAsyncIO *pIO, double *pValue );
long drvAsciiStringIo( drvAsyncIO *pIO, char *pValue );

/*
 * read output parameter during init
 * (some parameters do  not persist in the device and 
 * so S_drvAscii_readOnlyParameter is returned to indicate that
 * we always wish to initialize the parameter from the
 * value stored in the database file
 */
long drvAsciiReadOutput( drvCmndPrivate *id,	/* link id */
			 long *pValue);		/* raw read/write data */

#define M_drvAsciiLib (1002<<16U)
#define drvAsciiError(CODE) (M_drvAsciiLib | (CODE)) 

#define S_drvAscii_Ok 0 /* success */
#define S_drvAscii_badParam drvAsciiError(1) /*Ascii driver: bad parameter*/
#define S_drvAscii_AsyncCompletion drvAsciiError(2) /*Ascii driver: async completion*/
#define S_drvAscii_writeOnlyParameter drvAsciiError(3) /*Ascii driver: write only parameter (cant be read)*/
#define S_drvAscii_noMemory drvAsciiError(4) /*Ascii driver: no memory*/
#define S_drvAscii_uknParam drvAsciiError(5) /*Ascii driver: no paramater by that name*/
#define S_drvAscii_noComm drvAsciiError(6) /*Ascii driver: no communication with device*/
#define S_drvAscii_toLate drvAsciiError(7) /*Ascii driver: func has no effect after initalization*/
#define S_drvAscii_queryFail drvAsciiError(8) /* Ascii driver: a controller query failed */
#define S_drvAscii_dataErr drvAsciiError(9) /* Ascii driver: response string invalid */
#define  S_drvAscii_linkErr drvAsciiError(10) /* Ascii driver: link structure invalid */
#define  S_drvAscii_cmdTmo drvAsciiError(11) /* Ascii driver: command timeout (overloaded?) */


/*===========================================================================
 *
 * PUBLIC DEFINITIONS
 *
 ============================================================================*/



/*+*********************************************************************
  $Log: drvAscii.h,v $
  Revision 1.1  2000/04/20 01:37:13  peregrin
  End of April bright time devel.

  Revision 1.5  1999/09/02 01:24:04  kics
  Significant changes to alleviate miscomms between read and write
  tasks. This was instigated by changes to drvCmSx which suffered
  from similar problem (ie. this is derived from that).

  Revision 1.4  1999/07/22 01:21:09  kics
  corrected a typo in a comment.

 * Revision 1.3  1999/07/22  01:00:02  ahoney
 * Changed the drvAsciiPrivateCallBack typedef to not contain
 * a 'const' parameter.
 *
  Revision 1.1  1998/12/03 23:56:12  ktsubota
  Initial insertion

  Revision 1.8  1997/05/13 02:27:11  ahoney
  Changed pOutReadFunc within cmdTableEntry to be of type cmdFunc in
  order to eliminate compiler warnings.

 * Revision 1.7  1997/03/07  01:36:23  ahoney
 * Added the ability to enable 'debug' on a link-by-link basis. The
 * param field keyword is <debug> and should be associated with a
 * longout record.
 *
 * Revision 1.6  1997/01/23  02:19:50  ahoney
 * Added a #define for LOCAL
 *
 * Revision 1.5  1997/01/20  20:51:05  ahoney
 * Extensive mods to accomodate:
 *   -added mbbi and mbbo direct records;
 *   -added waveform records (long stringin);
 *   -added conversion of binary streams '0' and '1', with or without
 *    delimiters
 *   -added conversion from string to numeric 'abcd'->0x61626364->1633837924
 *   -added support for muliple line input strings
 *
 * Revision 1.4  1996/12/18  23:27:36  ahoney
 * Mods to support ao,bi,bo,mbbi,mbbo,longin,longout,stringin, and stringout
 * records. These were necessitated for use with IFSM and chopper.
 *
 * Revision 1.3  1996/09/13  20:49:54  ahoney
 * Mods to accomodate the flat lamp device.
 *
 * Note this driver was completed only so far as was necessary for the
 * data acquisition systems. The drive will still need a few mods for
 * the IFSM.
 *
 * Revision 1.2  1996/09/11  21:51:54  ahoney
 * Mods to incorporate longin and longout records as needed for the
 * dome flat lamps. Also modified the handling of '%nk' in data formats
 * so that 'n' characters can be 'killed' at the beginning of a data
 * stream - this allows stripping leading NULLs,...
 *
 * Revision 1.1  1996/07/12  23:26:26  ahoney
 * drvAscii is a new directory for support for serial comms to remote
 * devices via ascii strings
 *

 *
***********************************************************************/
 

