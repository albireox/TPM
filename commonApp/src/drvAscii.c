static char rcsid[] = "$Id: drvAscii.c,v 1.6 2001/04/10 14:51:02 peregrin Exp $";
 
/* 
 * *
 *  Author: Allan Honey (for the KECK Observatory)
 *  Date: 5-30-96 
 *
 *  NOTES
 *  The callback mechanism and interface to drvSerial was derived from
 *  drvCmSx which was written by Jeff Hill for the Keck observatory.
 *
 *  The driver support requires the facilities provided in drvSerial.c. 
 *
 *  The basic design is to format an output string, from the data supplied 
 *  by device support, and pass a resultant request-for-output to the 
 *  drvSerial transmit task. Included in the request is the routine which 
 *  is to be used for handling the i/o. For instance, if the output string 
 *  was a query for data then a response is expected so the transmit task 
 *  must be blocked until the response is received. The drvSerial receive 
 *  task, which is passed a parsing routine (getFrame) on startup, unblocks 
 *  the transmitter when the response is received.
 * 
 *  The interface to drvAscii is via seven functions.
 *
 *  On driver init the function drvAsciiInit() must be called one time.
 *  This function is the drvInitFunc_t which is in the drvAsciiSio structure
 *  specified in drvSup.ascii. drvAsciiInit initializes: the global linkList,
 *  which is a list of all serial links configured in drvAscii; the 
 *  globalMutex which is the semaphore for the linkList. 
 *
 *  Note that the function drvAsciiReport is also in the drvAsciiSio 
 *  structure.
 *
 *  During device init the function drvAsciiInitiateAll() is called one 
 *  time from devAscii. This function currently does nothing.
 *
 *  During record init the function drvAsciiCreateSioLink() is called once
 *  from devAscii for each record. Suffice it to say that this function
 *  ensures that a link is established via drvSerial, and caches information
 *  needed to complete a transaction with the remote device for the record
 *  in question, including the asynchronous callback info associated with
 *  devAscii.
 *
 *  During record init, an additional function, namely drvAsciiReadOutput(), 
 *  is called from devAscii for each output record. This function provides 
 *  for obtaining initial values for outputs.
 *
 *  During normal record processing, devAscii interfaces to drvAscii with 
 *  the functions drvAsciiIntIo(), drvAsciiRealIo(), and drvAsciiStringIo().
 *  Of course this is in addition to the asynchronous completion routines
 *  which are part of devAscii.
 *
 */
 
/*
 *  ANSI C
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

/*
 *  vxWorks
 */
#include <taskLib.h>
#include <sysLib.h>
#include <semLib.h>
#include <tickLib.h>
#include <ioLib.h>
#include <logLib.h>

#include <errMdef.h>
#include <ellLib.h>
#include <epicsPrint.h>
#include <drvSup.h>
#include <dbDefs.h>
#include <dbScan.h>
#include "drvSerial.h"
#include "drvAscii.h" 


/*
 *  driver support entry table
 */
long  drvAsciiReport(int level);
STATIC  long  drvAsciiInit(void);

struct
{
        long      number;
        DRVSUPFUN report;
        DRVSUPFUN init;

}  drvAsciiSio =
   {
     2L,
     drvAsciiReport,
     drvAsciiInit
   };

STATIC rbvFunc     getInteger;
STATIC rbvFunc     getReal;
STATIC rbvFunc     getSlope;
STATIC rbvFunc     getTimeout;
STATIC rbvFunc     getDebugFlag;
STATIC rbvFunc     getConnState;
STATIC rbvFunc     noOutputReadFunc;

STATIC cmdFunc     readConnState;
STATIC cmdFunc     setConnState;
STATIC cmdFunc     setSlope;
STATIC cmdFunc     setTimeout;
STATIC cmdFunc     setDebugFlag;

STATIC cmdFunc     writeInteger;
STATIC cmdFunc     readInteger;
STATIC cmdFunc     writeReal;
STATIC cmdFunc     readReal;

STATIC cmdFunc     setWriteCmt;
STATIC cmdFunc     setReadCmt;
STATIC cmdFunc     writeString;
STATIC cmdFunc     readString;

STATIC drvAsciiPrivateCallBack responseNotifier;

/* 
 *  The cmdTable consists of the possible 'command strings' for which
 *  i/o functions exist. During init, the function drvAsciiCreateSioLink
 *  is called. If the command string or data type string do not match
 *  one of the entries in this table then the associated record is invalid
 *  (ie. its parm field is incorrect). If the command string or data type
 *  string match one of the entries in this table then a pointer is retained
 *  to the relevant entry such that succeeding i/o is performed via the
 *  function(s) in that entry.
 *
 *  In order to add additional functionality to drvAscii it is necessary
 *  to add an entry to this table, both of the functions and an
 *  associated string. This may be appropriate when adding one or two
 *  commands for a remote device, where it is not worth writing a new device
 *  support layer on top of drvAscii.
 *
 */
STATIC cmdTableEntry cmdTable[] = {
  {"INTEGER_OUT",  writeInteger,   getInteger},
  {"INTEGER_IN",   readInteger,    noOutputReadFunc},
  {"REAL_OUT",     writeReal,      getReal},
  {"REAL_IN",      readReal,       noOutputReadFunc},
  {"STRING_OUT",   writeString,    noOutputReadFunc},
  {"STRING_IN",    readString,     noOutputReadFunc},

  /* 
   *  the following are special records, that is, records which affect
   *  the serial link and/or how drvAscii comms over that link
   */
  {"connect",      setConnState,   noOutputReadFunc},
  {"connectSts",   readConnState,  getConnState},
  {"slope",        setSlope,       getSlope},
  {"readCmt",      setReadCmt,     noOutputReadFunc},
  {"timeout",      setTimeout,     getTimeout},
  {"writeCmt",     setWriteCmt,    noOutputReadFunc},
  {"debug",        setDebugFlag,   getDebugFlag},

  {NULL,           NULL,           NULL}
};


/*
 *  forward references 
 */
STATIC drvSerialParseInput getFrame;

STATIC drvSerialSendCB putFrame;
STATIC drvSerialSendCB putFrameAndBlock;

STATIC int  drvAsciiQueryStringReq( drvCmndPrivate    *pPriv, 
				    const 	char  *pMsg, 
				    drvSerialResponse *pRes );
STATIC int  drvAsciiQuery( drvCmndPrivate    *pPriv, 
			   drvSerialRequest  *pReq, 
			   drvSerialResponse *pRes );
STATIC int  drvAsciiSendIgnoreResp( drvAsyncIO       *pIO, 
				    drvSerialRequest *pReq );
STATIC int  drvAsciiSend( drvAppPrivate *pDas, drvSerialRequest *pReq );
STATIC void drvAsciiLoadReq( drvSerialRequest *pReq, const char *pMsg );
STATIC int  putFrameAndRemoveResp( FILE *fp, drvSerialRequest *pReq );
STATIC int  drvAsciiDebug( char *pFormat, ... );
STATIC long drvAsciiOpen( const char *pFileName,drvAppPrivate **ppDas );
STATIC int  drvAsciiSendCallBackWithResp( drvAsyncIO *pIO, 
					  drvSerialRequest *pReq,
					  int noUnlock );
STATIC int  putFrameAndCallBack( FILE *fp, drvSerialRequest *pReq );

/*
 *  global variables controllable from the shell
 */
int      drvAsciiDebugLevel;

/*
 *  local variables that are used for all links
 */
STATIC ELLLIST	linkList;
STATIC SEM_ID	globalMutex;


/*
 * ---------------------------------------------------------------------------
 *  drvInitiateAll() - called once during device init. 
 *  Currently does nothing.
 *
 *  Public as it is used by device support
 */
long drvAsciiInitiateAll( void )
{
  /* !!! need to pass app specific init here */
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiIntIo() - this function is the interface between  
 *  devAscii and drvAscii for integer data.
 *
 *  Public as it is used by device support
 */
long drvAsciiIntIo( drvAsyncIO *pIO,	/* async io structure  */
		    long       *pValue) /* raw read/write data */
{
  drvCmndPrivate *pCmndPriv = &pIO->id;

  /*
   *  if the remote device is not connected then return an error 
   *  except for records which are relevant to the connection 
   *  (ie. allow connection status and reconnect commands)
   */
  assert( pCmndPriv->pAppPriv->pLink );
  if ( !pCmndPriv->pAppPriv->pLink->linkId ) return S_drvAscii_linkErr;


  if (   pCmndPriv->pAppPriv->state.noComm
	 && (*pCmndPriv->pCmndTableEntry->pCmdFunc != readConnState)
	 && (*pCmndPriv->pCmndTableEntry->pCmdFunc != setConnState) 
	 )
    return (S_drvAscii_noComm);

  /* 
   *  call the relevant function in the cmdTable, which was setup 
   *  during init, so as to perform the serial i/o
   */
  return (*pCmndPriv->pCmndTableEntry->pCmdFunc)
    ( pCmndPriv->pAppPriv, (void *)pValue, pIO, pCmndPriv->pCmndArg );
} 

/*
 * ---------------------------------------------------------------------------
 *  drvAsciiRealIo() - this function is the interface between 
 *  devAscii and drvAscii for real/floating-point data.
 *
 *  Public as it is used by device support
 */
long drvAsciiRealIo( drvAsyncIO *pIO,	/* async io structure  */
		     double     *pValue)/* raw read/write data */
{
  drvCmndPrivate *pCmndPriv = &pIO->id;

  /*
   *  if the remote device is not connected then return an error 
   *  except for records which are  relevant to the connection 
   *  (ie. allow connection status and reconnect commands)
   */
  assert( pCmndPriv->pAppPriv->pLink );
  if ( !pCmndPriv->pAppPriv->pLink->linkId ) return S_drvAscii_linkErr;

  if (   pCmndPriv->pAppPriv->state.noComm
	 && (*pCmndPriv->pCmndTableEntry->pCmdFunc != readConnState)
	 && (*pCmndPriv->pCmndTableEntry->pCmdFunc != setConnState) 
	 )
    return (S_drvAscii_noComm);

  /* 
   *  call the relevant function in the cmdTable, which was setup 
   *  during init, so as to perform the serial i/o
   */
  return (*pCmndPriv->pCmndTableEntry->pCmdFunc)
    ( pCmndPriv->pAppPriv, (void *)pValue, pIO, pCmndPriv->pCmndArg );
} 

/*
 * ---------------------------------------------------------------------------
 *  drvAsciiStringIo() - this function is the interface between 
 *  devAscii and drvAscii for string data.
 *
 *  Public as it is used by device support
 */
long drvAsciiStringIo( drvAsyncIO *pIO,	     /* async io structure */
		       char       *pDataStr) /* raw read/write data */
{
  drvCmndPrivate *pCmndPriv = &pIO->id;

  /*
   *  if the remote device is not connected then return an error 
   *  except for records which are  relevant to the connection 
   *  (i.e allow connection status and reconnect commands)
   */
  assert( pCmndPriv->pAppPriv->pLink );
  if ( !pCmndPriv->pAppPriv->pLink->linkId ) return S_drvAscii_linkErr;

  if (   pCmndPriv->pAppPriv->state.noComm
	 && (*pCmndPriv->pCmndTableEntry->pCmdFunc != readConnState)
	 && (*pCmndPriv->pCmndTableEntry->pCmdFunc != setConnState) 
	 )
    return ( S_drvAscii_noComm );

  /* 
   *  call the relevant function in the cmdTable, which was setup 
   *  during init, so as to perform the serial i/o
   */
  return (*pCmndPriv->pCmndTableEntry->pCmdFunc)
    ( pCmndPriv->pAppPriv, (void *)pDataStr, pIO, pCmndPriv->pCmndArg );
} 

/*
 * ---------------------------------------------------------------------------
 *  drvAsciiReadOutput() - this function is called during record init
 *  for those  output records for which initial values can be obtained.
 *  This is the function called from devAscii.
 *
 *  Public as it is used by device support
 */
long drvAsciiReadOutput( 
			drvCmndPrivate *id,	 /* link id */
			long           *pValue)  /* raw read/write data */
{

  if ( id->pAppPriv->state.noComm )
    return( S_drvAscii_noComm );

  assert( id->pAppPriv->pLink );
  if ( !id->pAppPriv->pLink->linkId ) return S_drvAscii_linkErr;
	
  return (*id->pCmndTableEntry->pOutReadFunc )
    ( id, pValue, NULL, id->pCmndArg );
}


/*
 * ---------------------------------------------------------------------------
 *  processFormat - private routine for parsing the command prompt, 
 *  command response, read back prompt, and read back response strings.
 */
STATIC long processFormat( char *format, drvResponseInfo *formatInfo )
{
  int          count1 = 0; 

  unsigned int suppress = FALSE, 
               dataTypeFound = FALSE;

  char         *spec, *spec1, *delimEnd;

  /* 
   *  the format string (either the prompt or response) is parsed to  
   *  ensure that it is valid.
   *
   *  If more than one '%' exists then one must be for '%nK' or  '%nB %['  
   *  or '%nT', all others must be of the form '%*,' except if '%nB' does 
   *  not exist, in which case one of '%s'. '%f', '%f',... is valid. That 
   *  is a data type specification is only valid for one value in the 
   *  format string but any number of '%*' is valid. Note that '%*b' is  
   *  not permitted.
   *
   *  The definition of the non-standard C specifications are:
   *   '%nK' is the number of characters to 'kill' at the beginning
   *        of a data stream, '%*K' is valid and means ignore response 
   *
   *   '%nB' indicates that the incoming character stream is a series 
   *        of '0' and '1' from which an integer is to be assembled
   *        the '%[...]', following '%nb', allows for delimiters in a bit
   *        stream.
   *
   *   '%nT' indicates the string data spans 'n' records (ie. 'n' read 
   *         command terminators will occur before all characters are 
   *         received).
   *
   *  If the 'n' following '%' is anything other than 1, for data types
   *  other than 'k' or 'b' or 's', then the parameter is invalid, as 
   *  arrays are not currently implemented (as of 970107).
   */

  spec = formatInfo->dataFormat = format;

  while ( (spec1 = strchr( spec, '%' )) ) {
    spec = spec1; 
    spec++;
    count1 = 0;

    delimEnd = strchr( spec, ']' );

    if ( *spec == '*' ) { 
      spec++;
      suppress = TRUE;
    }
    else 
      suppress = FALSE;

    while ( isdigit( *spec ) ) {
      count1 = count1 * 10 + (int)(*spec - '0');
      spec++;
    }

    if ( suppress )
      count1 = 0;
    else {
      count1 = count1 > 0 ? count1 : 1;
    }

    /* 
     *  ignore %l as all integer and real data is scanned as 
     *  long or double 
     */
    if ( *spec == 'l' || *spec == 'L' ) spec++;

    if ( *spec == 'k' || *spec == 'K' ) {
      /* 
       *  some or all of the input stream should be killed (ignored).
       *  this spec can only be at the beginning of the format string
       */
      if ( spec1 != format ) return S_drvAscii_badParam;
      
      formatInfo->kill = TRUE;
	
      if ( suppress ) {
	formatInfo->killAll = TRUE; /* entire input will be ignored */
      }
      else {
	formatInfo->killCnt = count1;
      }

      format = 
      formatInfo->dataFormat = ++spec;

    }

    /* 
     *  data stream can span multipe record boundaries (line terminators)
     *  but the designation must be either at the beginning of the format
     *  string or immediately following '%nk' or '%*k'. Also '%*t' is not
     *  valid.
     */
    else if ( *spec == 't' || *spec == 'T' ) {
      if ( suppress || 
	   dataTypeFound || 
	   formatInfo->recordCnt ||
	   (spec1 != formatInfo->dataFormat) ) 
	return S_drvAscii_badParam;

      formatInfo->recordCnt = count1;

      /*  skip over the %t */
      format =
      formatInfo->dataFormat = ++spec;
    }

    else if ( *spec == 'b' || *spec == 'B' ) {
      /* 
       *  data is expected as a bit stream (ie. all '0' and '1') 
       *  potentially with delimiters. Binary streams cannot be 
       *  suppressed (ie. '%*b' is not allowed as one can accomplish 
       *  the same with %*d or %*f) 
       */
      if ( suppress || dataTypeFound ) return S_drvAscii_badParam;

      formatInfo->dataType = RBF_BINARY;
      dataTypeFound = TRUE;

      formatInfo->dataCnt = count1;
      
      if ( formatInfo->dataFormat != spec1 ) {
        formatInfo->preAmbleCnt = spec1-format;
	formatInfo->preAmble = formatInfo->dataFormat;
      }

      /*  skip over the %nb */
      formatInfo->dataFormat = ++spec;
    }

    else if ( *spec == '[' ) {
      /* 
       *  delimiters exist for a binary stream but the designation must
       *  immediately follow the '%nb' the syntax is '%[c...]' where 'c'
       *  is a delimiter character of which there can be many
       */

      /*  if ']' is missing then abort */
      if ( !delimEnd )
	return S_drvAscii_badParam;

      /* 
       *  if the spec is '%*[' then assume it is not a bit stream delimiter
       *  spec. Otherwise, if it does not immediately follow a '%nb' bit
       *  stream spec, assume it is not a bit stream delimiter. That is,
       *  it can only be a bit stream delimiter if it immediately follows
       *  '%nb' and does not have an assignment suppression char '*'
       */
      if ( spec == formatInfo->dataFormat && !suppress ) {
	if ( formatInfo->dataType != RBF_BINARY )
	  return S_drvAscii_badParam; 

	formatInfo->delimiters = ++spec;
	formatInfo->dataFormat = ++delimEnd;
	spec = delimEnd;
      }
      
      /*  must be for a set capture */
      else { 
	spec = delimEnd; 
      } 

    }

    else if ( *spec == 's' || *spec == 'S' ||
	      *spec == 'c' || *spec == 'C'    ) {
      if ( !suppress && dataTypeFound ) return S_drvAscii_badParam;

      if ( !suppress ) {
	formatInfo->dataType = RBF_STRING;
      
	formatInfo->dataCnt = count1;
      
	dataTypeFound = TRUE;

	if ( formatInfo->dataFormat != spec1 ) {
	  formatInfo->preAmbleCnt = spec1 - formatInfo->dataFormat;
	  formatInfo->preAmble = formatInfo->dataFormat;
	}

	/*  skip over the %nc or %ns */
	formatInfo->dataFormat = ++spec;
      }
    }

    else {
      if ( !suppress && dataTypeFound ) return S_drvAscii_badParam;

      /*  all integer data is scanned into a long */
      if ( *spec == 'd' || *spec == 'D' ||
	   *spec == 'i' || *spec == 'I' ||
	   *spec == 'u' || *spec == 'U' ||
	   *spec == 'o' || *spec == 'O' ||
	   *spec == 'x' || *spec == 'X'    ) {
	if ( !suppress ) {
	  formatInfo->dataCnt = count1;
	  formatInfo->dataType = RBF_INT;
	}
      }

      /*  all real data is scanned into a double */
      else if ( *spec == 'f' || *spec == 'F' ||
		*spec == 'e' || *spec == 'E'    ) {		
	if ( !suppress ) {
	  formatInfo->dataCnt = count1;
	  formatInfo->dataType = RBF_REAL;
	}
      }
      
      else
	/*  other specs are not valid */
	return S_drvAscii_badParam;
    }
  }

  return S_drvAscii_Ok;
}

/*
 * ---------------------------------------------------------------------------
 *  drvAsciiDebug() - function to generate printfs when 
 *  drvAsciiDebugLevel is set.
 */
STATIC int drvAsciiDebug( char *pFormat, ... )
{
  va_list args;

  if( drvAsciiDebugLevel == 0 ) {
    return 0;
  }

  va_start( args, pFormat );
  vprintf( pFormat, args );
  va_end( args );

  return 0;
}


/*
 * ---------------------------------------------------------------------------
 *  printString - private routine for printing ascii strings such that 
 *   non-printable characters are printed as hex values enclosed in 
 *   brackets '[%x]'.
 */
STATIC void printString( long count, char *str )
{
  char *ptr,*end;

  if ( count > 0 )
    end = str+count;
  else end = str;

  ptr = str;

  while ( (ptr < end) || (count > 0 ? count : *ptr != '\0') ) {
    if ( (int)*ptr < 32 || (int)*ptr > 126 )
      printf( "[%2d]", (int)*ptr );
    else
      printf( "%c", *ptr );

    ptr++;
    
    if (count > 0) count--;
  }
}


/*
 * ---------------------------------------------------------------------------
 *  printCmndArgs - used for displaying the command args at debug level.
 *  This routine is public so that one can debug from the shell.
 */
void printCmndArgs( drvCmndArg *pCmndArgs )
{
  if ( !pCmndArgs ) {
    printf(" pCmndArgs=0x%x is NULL\n",(int)pCmndArgs );
    return;
  }

  printf( "\nCMND PROMPT INFO\n" );
  printf( "kill=%d, killAll=%d, killCnt=%ld\n",
	  pCmndArgs->cmndPromptInfo.kill,
	  pCmndArgs->cmndPromptInfo.killAll,
	  pCmndArgs->cmndPromptInfo.killCnt );
  printf( "dataType=%ld, preAcnt=%ld, dataCnt=%ld, recordCnt=%ld, arrayCnt= %ld\n",
	  pCmndArgs->cmndPromptInfo.dataType,
	  pCmndArgs->cmndPromptInfo.preAmbleCnt,
	  pCmndArgs->cmndPromptInfo.dataCnt,
	  pCmndArgs->cmndPromptInfo.recordCnt,
	  pCmndArgs->cmndPromptInfo.arrayCnt  );
  printf( "delimiters=0x%x, preAmble=0x%x, dataFormat=0x%x\n",
	  (int)pCmndArgs->cmndPromptInfo.delimiters,
	  (int)pCmndArgs->cmndPromptInfo.preAmble,
	  (int)pCmndArgs->cmndPromptInfo.dataFormat );

  printf( "\nCMND INFO\n" );
  printf( "kill=%d, killAll=%d, killCnt=%ld\n",
	  pCmndArgs->cmndInfo.kill,
	  pCmndArgs->cmndInfo.killAll,
	  pCmndArgs->cmndInfo.killCnt );
  printf( "dataType=%ld, preAcnt=%ld, dataCnt=%ld, recordCnt=%ld, arrayCnt= %ld\n",
	  pCmndArgs->cmndInfo.dataType,
	  pCmndArgs->cmndInfo.preAmbleCnt,
	  pCmndArgs->cmndInfo.dataCnt,
	  pCmndArgs->cmndInfo.recordCnt,
	  pCmndArgs->cmndInfo.arrayCnt  );
  printf( "delimiters=0x%x, preAmble=0x%x, dataFormat=0x%x\n",
	  (int)pCmndArgs->cmndInfo.delimiters,
	  (int)pCmndArgs->cmndInfo.preAmble,
	  (int)pCmndArgs->cmndInfo.dataFormat );

  printf( "\nRBV INFO\n" );
  printf( "kill=%d, killAll=%d, killCnt=%ld\n",
	  pCmndArgs->rbvInfo.kill,
	  pCmndArgs->rbvInfo.killAll,
	  pCmndArgs->rbvInfo.killCnt );
  printf( "dataType=%ld, dataCnt=%ld, recordCnt=%ld, arrayCnt= %ld\n",
	  pCmndArgs->rbvInfo.dataType,
	  pCmndArgs->rbvInfo.dataCnt,
	  pCmndArgs->rbvInfo.recordCnt,
	  pCmndArgs->rbvInfo.arrayCnt  );
  printf( "delimiters=0x%x, dataFormat=0x%x\n",
	  (int)pCmndArgs->rbvInfo.delimiters,
	  (int)pCmndArgs->rbvInfo.dataFormat );
 
  printf( "\ncmndPrompt=" );
  printString( -1, pCmndArgs->cmndPrompt );
  printf( "\n" );

  printf( "cmndFormat=" );
  printString( -1, pCmndArgs->cmndFormat );
  printf( "\n" );

  printf( "rbvPrompt=" );
  printString( -1, pCmndArgs->rbvPrompt );
  printf( "\n" );

  printf( "rbvFormat=" );
  printString( -1, pCmndArgs->rbvFormat );
  printf( "\n" );

}
 

/*
 * ---------------------------------------------------------------------------
 *  printCmndPrivate - used for displaying the command private info  
 *  for a given record. 
 *
 *  This routine is public so that one can debug from the shell.
 */
void printCmndPrivate( drvCmndPrivate *ptr )
{
  printf("drvCmndPrivate:\n");
  if ( !ptr ) {
    printf( "ptr=0x%x is NULL\n", (int)ptr );
    return;
  }

  printf( "node=0x%x, pAppPriv=0x%x, pCmndTableEntry=0x%x, pCmndArg=0x%x\n",
	  (int)&ptr->node, (int)ptr->pAppPriv,
	  (int)ptr->pCmndTableEntry,(int)ptr->pCmndArg );
  printf( "pCb=0x%x, pArg=0x%x, rbvFlag=%d\n",
	 (int)ptr->pCB, (int)ptr->pArg, (int)ptr->rbvFlag );


  printf( "respStrCnt=%ld\n", ptr->respStrCnt );
  printf( "respStr=");
  printString( ptr->respStrCnt, ptr->respStr );
  printf( "\n\n" );

  printCmndArgs( ptr->pCmndArg );
}
  

/*
 * ---------------------------------------------------------------------------
 *  printAsyncIo - used for displaying the command private info for a 
 *  given record. 
 *
 *  This routine is public so that one can debug from the shell.
 */
void printAsyncIo( drvAsyncIO  *ptr )
{
  printf("\ndrvAscynIO:\n");
  if ( !ptr ) { 
    printf("ptr=0x%x is NULL\n", (int)ptr );
    return;
  }
  
  printf("p(id)=0x%x, pCB=0x%x, pArg=0x%x, pAppDrvPrivate=0x%x\n",
	 (int)&ptr->id, (int)ptr->pCB, 
	 (int)ptr->pArg, (int)ptr->pAppDrvPrivate );

  printCmndPrivate( &ptr->id );

  return;  
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiReportOneDevice() - function to printf the app priv info 
 *  for a device report.
 */
STATIC void drvAsciiReportOneDevice( drvAppPrivate *pAppPriv, 
				     unsigned      level)
{
  char *pChar;

  assert(pAppPriv);

  if ( level > 0 ) {
    /* call an app specific report function (if it exists). */
    ;
  }
  if ( level > 1 ) {
    printf ( "\t\tBad queries=%lu\n",
	     pAppPriv->queryFailures );
    printf ( "\t\tData errors=%lu\n",
	     pAppPriv->dataErrors );
    printf ( "\t\tAnalog I/O slope=%f\n",
	     pAppPriv->slope );
    printf ( "\t\tResponse timeout=%lu\n",
	     pAppPriv->responseTMO/sysClkRateGet() );
    
    /*  display the current command terminators */
    printf( "\t\tWrite terminator=" );
    pChar = pAppPriv->writeCmt;
    while ( pChar && (*pChar != '\0') ) {
      if ( (int)(*pChar) < 32 || (int)(*pChar) > 126 ) 
	printf( "[%2d]", (int)(*pChar) );
      else 
	printf( "%c",*pChar );
      pChar++;
    }
    printf( "\n" );
    printf( "\t\tRead terminator=" );
    pChar = pAppPriv->readCmt;
    while ( pChar && (*pChar != '\0') ) {
      if ( (int)(*pChar) < 32 || (int)(*pChar) > 126 ) 
	printf( "[%2d]",(int)(*pChar) );
      else 
	printf( "%c",*pChar );
      pChar++;
    }
    printf( "\n" );
  }
  if ( level > 2 ) {
    semShow( pAppPriv->mutexSem, level );
  }
  
  return;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiReport() - function called for device report.
 *
 *  This routine is public so that it can be used from the shell.
 */
long drvAsciiReport(int level)
{
  drvSioLinkPrivate	*pLink;
  drvAppPrivate	        *pAppPriv;
  int			status;

  /*
   *  run the list of all links with a remote device on them 
   *  (mutex protects list)
   */
  status = semTake( globalMutex, WAIT_FOREVER );
  assert( status == OK );
  pLink = (drvSioLinkPrivate *) ellFirst( &linkList );
  while ( pLink ) {

    printf( "\tdrvAscii: port=%s\n", pLink->fileName );
    
    /*
     *  run the list of all devices on this link 
     *  (mutex protects list). Currently only a single remote
     *  device is permitted.
     */
    status = semTake( pLink->mutex, WAIT_FOREVER );
    assert( status == OK );

    /*  currently only one remote device is allowed on a link */
    pAppPriv = (drvAppPrivate *) ellFirst( &pLink->remoteDevList );
    if ( !pAppPriv->state.noComm )
      drvAsciiReportOneDevice( pAppPriv, level );

    status = semGive( pLink->mutex );
    assert( status == OK );
    
    if ( level > 2 ) {
      printf( "\nsendBlockSem:" );
      semShow( pLink->sendBlockSem, level );
      printf( "\nmutex:" );
      semShow( pLink->mutex, level );
    }
     
    pLink = (drvSioLinkPrivate *) ellNext( &pLink->node );
  }
  status = semGive( globalMutex );
  assert( status == OK );
  
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiInit() - driver init function which is called one time 
 *  during ioc init.
 */
STATIC long drvAsciiInit( void )
{
  ellInit( &linkList );
  globalMutex = 
    semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE );

  if ( !globalMutex ) {
    return S_drvAscii_noMemory;
  }
  
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiCreateSioLink() - this is the function called from 
 *  devAscii, during record init, so as to establish a link, and 
 *  create and initialize the structures which are ultimately linked 
 *  into the record (dvpt) such that succeeding i/o calls can be handled.
 *
 *  It is herein that the links into cmdTable are established.
 *
 */
long drvAsciiCreateSioLink(
		      const char     *pDataType,  /* i/o data type */
		      const char     *pFileName,  /* serial port device name*/
		      drvCmndArg     *pCmndArgs,
                                             /* for future ref of this link */
		      drvCmndPrivate *pId,	
                                             /* called on parameter change  */
		      drvAsyncUpdateCallBack *pCB,
                                             /* parameter to above call back*/
		      const void     *pArg)	  
{
  drvAppPrivate  *pAppPriv = NULL;
  long	         status = S_drvAscii_Ok;
  cmdTableEntry  *pTableEntry = NULL;
  char           *pCmndName = pCmndArgs->cmndPrompt;

  /*  
   *  ensure that the command args structure is valid 
   */
  if ( !pCmndArgs->cmndPrompt ||
       !pCmndArgs->cmndFormat ||
       !pCmndArgs->rbvPrompt  ||
       !pCmndArgs->rbvFormat)
    return S_drvAscii_badParam;

  /*
   *  Search for the command in the table
   */
  pTableEntry = cmdTable;
  while ( pTableEntry->pCmndName != NULL ) {
    if ( strncmp( pTableEntry->pCmndName, 
		  pCmndName,
		  strlen(pTableEntry->pCmndName) ) == 0) {
      break;
    }
    pTableEntry++;
  }
  
  /*
   *  If the command is not in the table then use the data type string 
   *  as the table lookup key. In this case the assumption is that the 
   *  command is a prompt for which a value of the associated type will 
   *  be returned.
   */
  if ( pTableEntry->pCmndName == NULL ) {
    pTableEntry = cmdTable;
    while ( pTableEntry->pCmndName ) {
      if ( strcmp( pTableEntry->pCmndName, pDataType ) == 0 ) {
	break;
      }
      pTableEntry++;
    }
  }
  
  /*
   *  Neither the command name nor the 'record' type string is valid 
   *  so abort
   */ 
  if ( pTableEntry->pCmndName == NULL ) {
    drvAsciiDebug( "(%s %s): Prompt string invalid!\n", 
		   pCmndName, pDataType );
    return S_drvAscii_badParam;
  }

  /* 
   *  Parse the command arg strings. Syntactical faults are captured 
   *  but faults relevant to the record type are left for the record 
   *  init routine, which invoked this routine, within device support.
   */
  if ( strlen( pCmndArgs->cmndPrompt ) )
       status = processFormat( pCmndArgs->cmndPrompt,
			       &pCmndArgs->cmndPromptInfo );
  if ( status ) { 
    drvAsciiDebug( "(%s %s): Bad Cmd prompt!\n",pCmndName, pDataType ); 
    if ( drvAsciiDebugLevel & 0x8 ) printCmndArgs( pCmndArgs );
    return status;
  }

  if ( strlen( pCmndArgs->cmndFormat ) ) 
    status = processFormat( pCmndArgs->cmndFormat,
			    &pCmndArgs->cmndInfo );
  
  if ( status ) { 
    drvAsciiDebug( "(%s %s): Bad Cmd response format!\n",pCmndName, pDataType); 
    if ( drvAsciiDebugLevel & 0x8 ) printCmndArgs( pCmndArgs );
    return status;
  }

  if ( strlen( pCmndArgs->rbvPrompt ) )
       status = processFormat( pCmndArgs->rbvPrompt, 
			       &pCmndArgs->rbvPromptInfo );
  if ( status ) { 
    drvAsciiDebug( "(%s %s): Bad Rbv prompt!\n",pCmndName, pDataType); 
    if ( drvAsciiDebugLevel & 0x8 ) printCmndArgs( pCmndArgs );
    return status;
  }

  if ( strlen( pCmndArgs->rbvFormat ) )
       status = processFormat( pCmndArgs->rbvFormat, 
			       &pCmndArgs->rbvInfo );
  if ( status ) { 
    drvAsciiDebug( "(%s %s): Bad Rbv response format!\n",pCmndName, pDataType); 
    if ( drvAsciiDebugLevel & 0x8 ) printCmndArgs( pCmndArgs );
    return status;
  }

  /*
   *  Find an existing application private structure (or create a
   *  new one). This ultimately results in a read and write task for
   *  the serial i/o.
   */
  status = drvAsciiOpen( pFileName, &pAppPriv );

  /*
   *  If the open fails then the serial 'linkId' will be null.
   *  This is needed so as to ensure subsequent i/o is rejected.
   */
  pId->pAppPriv = pAppPriv;

  pId->pCmndTableEntry = pTableEntry;
  pId->pCB = pCB;
  pId->pArg = pArg;
  pId->pCmndArg = pCmndArgs;
  pId->respStr[0] = '\0';
  pId->respStrCnt = 0;

  return status;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiOpenLink() - function which is called to establish a new
 *  serial link. This is the interface to drvSerial.
 */
STATIC long drvAsciiOpenLink( const char        *pFileName, 
			      drvAppPrivate     **ppAppPriv,
			      drvSioLinkPrivate **ppLinkPriv)
{
  drvAppPrivate	        *pAppPriv = NULL;

  drvSioLinkPrivate	*pLink;

  long			status,
                        length;


  *ppAppPriv = NULL;

  /*
   *  allocate and init link private structure 
   */
  pLink = (drvSioLinkPrivate *) calloc( 1, sizeof( *pLink ) );
  if ( !pLink ) {
    return S_drvAscii_noMemory;
  }

  *ppLinkPriv = pLink;
  pLink->linkId = NULL;

  /* 
   *  initialize the link's remote device list. Currently only one is allowed. 
   */
  ellInit( &pLink->remoteDevList );

  /*  
   *  create the link's mutual exclusion semaphore 
   */
  pLink->mutex = 
    semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE );
  if ( !pLink->mutex ) {
    return S_drvAscii_noMemory;
  }

  /*
   *  allocate application private structure 
   */
  pAppPriv = (drvAppPrivate *) calloc( 1, sizeof( *pAppPriv ) );
  if ( !pAppPriv ) {
    return S_drvAscii_noMemory;
  }

  *ppAppPriv = pAppPriv;
  pAppPriv->pLink = pLink;

  pAppPriv->state.noComm = 1;
  pAppPriv->reset = 0;
  pAppPriv->slope = 1.0;
  pAppPriv->responseTMO = 3 * sysClkRateGet();
  pAppPriv->dataErrors = 0;
  pAppPriv->queryFailures = 0;
  pAppPriv->debugFlag = 0;
  pAppPriv->writeCmt[0] = '\r';
  pAppPriv->writeCmt[1] = '\n';
  pAppPriv->writeCmt[2] = '\0';
  pAppPriv->readCmt[0] = '\r';
  pAppPriv->readCmt[1] = '\n';
  pAppPriv->readCmt[2] = '\0';
  
  /*
   *  Allocate the application private structure's mutual exclusion 
   *  semaphore .
   */
  pAppPriv->mutexSem = semBCreate(SEM_Q_FIFO, SEM_FULL);
  /*
    semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE );
    */
  if ( pAppPriv->mutexSem == NULL ) {
    return S_drvAscii_noMemory;
  }

  /*
   *  Keep a list of all devices of this type on this link.
   */
  ellAdd( &pLink->remoteDevList, &pAppPriv->node );

  /*
   *  Create the transmitter semaphore (prevent writes when a 
   *  synchronous write-read cycle is in progress).
   */
  pLink->sendBlockSem = semBCreate( SEM_Q_FIFO, SEM_EMPTY );
  if ( pLink->sendBlockSem == NULL ) {
    return S_drvAscii_noMemory;
  }
  
  /*
   *  create the receiver semaphore
   */
  pLink->syncLock = semBCreate( SEM_Q_FIFO, SEM_FULL );
  if ( pLink->syncLock == NULL ) {
    return S_drvAscii_noMemory;
  }

  /*  
   *  Store the link name (name used in fopen).
   */
  length =  min( strlen( pFileName ),sizeof( pLink->fileName )-1 );
  strncpy( pLink->fileName, pFileName, length );
	  
  pLink->fileName[ min( strlen( pFileName ),
			sizeof( pLink->fileName )-1 )]='\0';

  /*
   *  Open the link. 
   */
  status = drvSerialCreateLink( pFileName,
				getFrame,
				pLink,
				&pLink->linkId );
  if ( status ) {
    return status;
  }
   
  status = semTake( globalMutex, WAIT_FOREVER );
  assert( status == OK );
  ellAdd( &linkList, &pLink->node );
  status = semGive( globalMutex );
  assert( status == OK );
    
  /*  
   *  Everything is OK so mark the comms state as ok.
   */
  pAppPriv->state.noComm = 0;

  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiOpen() - function which determines if a new serial link
 *  needs be established or connects to an existing link .
 */
STATIC long drvAsciiOpen( const char    *pFileName,
			  drvAppPrivate **ppAppPriv)
{
  drvAppPrivate	        *pAppPriv = NULL;
  drvSioLinkPrivate	*pLink = NULL;
  long 			status;

  *ppAppPriv = NULL;

  /*
   *  Check to see if anything is on this link.
   */
  status = drvSerialAttachLink( pFileName,
				getFrame,
				(void **)&pLink );
  if ( status == S_drvSerial_OK ) {
    /* 
     *  S link exists for this pFileName so simply return the 
     *  relevant handle.
     */
    assert( pLink );
    status = semTake( pLink->mutex, WAIT_FOREVER );
    assert( status == OK );

    /*  
     *  Currently only one remote device is allowed on a link.
     */
    pAppPriv = (drvAppPrivate *) ellFirst( &pLink->remoteDevList );

    status = semGive( pLink->mutex );
    assert( status == OK );
 
    if ( pAppPriv ) {
      *ppAppPriv = pAppPriv;
      return S_drvAscii_Ok;
    }
    else {
      *ppAppPriv = NULL;
      return S_drvAscii_linkErr;
    }
  }
  else if ( status == S_drvSerial_noneAttached ) {
    /*
     *  No serial link exists for pFileName so create a new one.
     */
    status = drvAsciiOpenLink( pFileName, ppAppPriv, &pLink );
    if ( status ) {
      return status;
    }
  }
  else {
    /*  
     *  A fault is exists in serial driver so abort.
     */
    return status;
  }
  	  
  return status;
}
 

/*
 * ---------------------------------------------------------------------------
 *  drvAsciiIoctl() - public function to allow ioctl of the underlying
 *    serial port. This may be used for disabling features such as
 *    CTS/RTS usage, half or full duplex (in which case handling of CTS/RTS
 *    is critical). The relevant ioctl functions and parameter are serial
 *    driver specific which may or may not be a vxworks driver.
 */
long drvAsciiIoctl( char      *portName, 
			   long      ioctlFunc,
			   long      ioctlOption )
{
  drvSioLinkPrivate *pLink;

  FILE              *FP;
 
  long               status;

  /*
   *  Run the list of all links with a remote device on them 
   *  (mutex protects list).
   */
  status = semTake( globalMutex, WAIT_FOREVER );
  assert( status == OK );

  pLink = (drvSioLinkPrivate *) ellFirst( &linkList );
  while ( pLink ) {
    if ( !strcmp( portName, pLink->fileName ) ) break;

    pLink = (drvSioLinkPrivate *) ellNext( &pLink->node );
  }

  status = semGive( globalMutex );
  assert( status == OK );

  if ( !pLink ) { 
    printf("%s: is not a known drvSerial port!\n", portName);
    return -1;
  }

  FP = drvSerialGetFile( pLink->linkId, WRITE );

  if ( !FP ) {
    printf("%s: Not currently opened for i/o!\n", portName);
    return -1;
  }

  return( ioctl( fileno( FP ), ioctlFunc, ioctlOption ) );
}


/*
 * ---------------------------------------------------------------------------
 *  getConnState() - returns a link's connection state: 0 = not connected.
 *  currently a link can only be marked as not connected if a fault occurs
 *  during init, or from a database record (ie. the user can force the 
 *  condition).
 */
STATIC long getConnState( drvCmndPrivate *pCmndPriv, 
			  void           *pValue, 
			  drvAsyncIO     *pIO, 
			  drvCmndArg     *pFuncParam )
{
  *(long *)pValue = !( pCmndPriv->pAppPriv->state.noComm );
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  getSlope()- returns a link's slope value. 
 *
 *  The slope is a value by which all float output values are divided 
 *  by and all float input values are multiplied by. The default value 
 *  is 1.0.
 *
 *  Slope is needed because: the RVAL for an analog input record is an  
 *  integer nd there is no way to circumvent conversion from RVAL to VAL 
 *  so ESLO,   ASLO and ROFF come into play; the RVAL from an analog output 
 *  record is an  integer and if one used VAL then the ESLO, ASLO and ROFF 
 *  would be circumvented.
 *
 *  This needs to be upgraded so it is record specific.
 */
STATIC long getSlope ( drvCmndPrivate *pCmndPriv, 
		       void           *pValue, 
		       drvAsyncIO     *pIO, 
		       drvCmndArg     *pFuncParam )
{
  *(long *)pValue = pCmndPriv->pAppPriv->slope;
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  getTimeout() - returns a link's current maximum time allowed for a
 *  response to a data prompt (default is 10 seconds). The originating 
 *  record can be a float or integer
 *
 */
STATIC long getTimeout ( drvCmndPrivate *pCmndPriv, 
			 void           *pValue, 
			 drvAsyncIO     *pIO, 
			 drvCmndArg     *pFuncParam )
{
  *(long *)pValue = pCmndPriv->pAppPriv->responseTMO / sysClkRateGet();
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  getDebugFlag() - returns a link's debug state
 *                   record should be a longin
 */
STATIC long getDebugFlag ( drvCmndPrivate *pCmndPriv, 
			   void          *pValue, 
			   drvAsyncIO    *pIO, 
			   drvCmndArg    *pFuncParam )
{
  *(long *)pValue = pCmndPriv->pAppPriv->debugFlag;

  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  extractRealData() - the data stream is expected to be of the form '%f'
 * 
 */
STATIC long extractRealData( char            *pBuf,
			     char            *format,
			     drvResponseInfo *formatInfo,
			     char            *cmt,
			     double          *pValue )
{
  long   count = 0;

  char   bfr[DRV_SERIAL_BUF_SIZE];

  float  fValue;

  char   *ptr, *sign;

  int    zeroIt = 1;

  /*
   *  Force any spaces that are between the numeric sign and the digits
   *  to be 0 so that sscanf yields a numeric value. Note this is needed
   *  for some devices such as Sony gauges. 
   *
   *  This should probably be a selectable on a record by record basis
   *  - a future mod.
   */

  sign = strchr( pBuf, '-' );

  if (!sign)
    sign = strchr( pBuf, '+' );

  if ( sign ) {
    ptr = sign;
    ptr++;

    while ( !isdigit( *ptr ) ) {
      if ( *ptr != ' ' ) {
	zeroIt = 0;
	break;
      }

      if ( *ptr == '\0' )
	break;
      
      ptr++;
    }

    if ( zeroIt ) {
      ptr = ++sign;

      while ( *ptr == ' ' ) {
	*ptr = '0';
	ptr++;
      }
    }
  }

  if ( formatInfo->dataType == RBF_REAL ) {
    /* 
     *  A data type spec of real exists so the data stream is expected 
     *  to be of the form '%f'. A '%n' is appended to the data format  
     *  specification so the total number of characters consumed can be 
     *  tallied.
     */
    sprintf( bfr,"%s%s%s", formatInfo->dataFormat, cmt, "%n" );

    sscanf( pBuf, bfr, &fValue, &count );

    *pValue = fValue;
  }
  else {
    /*  
     *  A data format specification does not exist so assume '%f' 
     */ 
    if ( format ) 
      sprintf( bfr, "%s%s%s%s", format, "%f", cmt, "%n" );
    
    else
      sprintf( bfr, "%s%s%s", "%f", cmt, "%n" );

    sscanf( pBuf, bfr, &fValue, &count );

    *pValue = fValue;
  }

  return count;
}


/*
 * ---------------------------------------------------------------------------
 *  extractIntData() - the data stream is expected to be of the form '%d'
 * 
 */
STATIC long extractIntData( char            *pBuf,
			    char            *format,
			    drvResponseInfo *formatInfo,
			    char            *cmt,
			    long            *pValue )
{
  long   count = 0,
         iValue;

  char   bfr[DRV_SERIAL_BUF_SIZE];

  char   *ptr, *sign;

  int    zeroIt = 1;

  /*
   *  Force any spaces that are between the numeric sign and the digits
   *  to be 0 so that sscanf yields a numeric value. Note this is needed
   *  for some devices such as Sony gauges. 
   *
   *  This should probably be a selectable option - a future
   *  mod.
   */
  sign = strchr( pBuf, '-' );

  if (!sign)
    sign = strchr( pBuf, '+' );

  if ( sign ) {
    ptr = sign;
    ptr++;

    while ( !isdigit( *ptr ) ) {
      if ( *ptr != ' ' ) {
	zeroIt = 0;
	break;
      }

      if ( *ptr == '\0' )
	break;
      
      ptr++;
    }

    if ( zeroIt ) {
      ptr = ++sign;

      while ( *ptr == ' ' ) {
	*ptr = '0';
	ptr++;
      }
    }
  }

  if ( formatInfo->dataType == RBF_INT ) {
    /* 
     *  A data type spec of integer exists so the data stream is
     *  expected to be of the form '%f'. A '%n' is appended to the 
     *  data format specification so the total number of characters 
     *  consumed can be tallied.
     */
      sprintf( bfr,"%s%s%s", formatInfo->dataFormat, cmt, "%n" );

      sscanf( pBuf, bfr, pValue, &count );
  }
  else {
    /*  
     *  a data format specification does not exist so assume '%d' 
     */ 
    if ( format ) 
      sprintf( bfr, "%s%s%s%s", format, "%d", cmt, "%n" );
    else
      sprintf( bfr, "%s%s%s", "%d", cmt, "%n" );

    sscanf( pBuf, bfr, &iValue, &count );

    *pValue = iValue;
  }

  return count;
}


/*
 * ---------------------------------------------------------------------------
 *  extractBinaryData()- the data stream is expected to be a string of 
 *  '0' and '1'.
 * 
 */
STATIC long extractBinaryData( char            *pBuf,
			       char            *format,
			       drvResponseInfo *formatInfo,
			       char            *cmt,
			       long            *pValue )
{
  long     count = 0,
           idx = 0,
           iValue = 0;

  unsigned abort = FALSE;

  char     *pDelim,
           bfr[DRV_SERIAL_BUF_SIZE];

  /*  
   *  if characters preceed the bit stream then process them first.
   */
  if ( formatInfo->preAmble ) {
    strncpy( bfr, formatInfo->preAmble, formatInfo->preAmbleCnt );
    bfr[formatInfo->preAmbleCnt] = '\0';

    idx = strlen( bfr );
    bfr[idx] = '%';
    bfr[++idx] = 'n';
    bfr[++idx] = '\0';
    sscanf( pBuf, bfr, &count );
  }

  /*  
   *  vxworks implementation of '%*c' skips whitespace so there is 
   *  no way for the user to specify that whitespace (ie. record 
   *  terminators) should be skipped. Thus we always strip whitespace 
   *  before converting to integer. Brutal but necessary. 
   */
  while ( isspace( pBuf[count] ) ) count++;
   
  /*  
   *  Process the bit stream, skipping over any delimiter characters. 
   */
  for ( idx = 0; idx < formatInfo->dataCnt; ) 
    if ( pBuf[count] != '0' && pBuf[count] != '1' ) {
      abort = TRUE;
      
      if ( formatInfo->delimiters ) {
	pDelim = formatInfo->delimiters;
	while ( pDelim && (*pDelim != '\0') && (*pDelim != ']') ) {
	  if (*pDelim == pBuf[count]) { abort = FALSE; break;}
	  pDelim++;
	}
      }

      if ( abort ) { count = 0; break; }

      else count++;
    }
    else {
      idx++;
      iValue = (iValue << 1) | (pBuf[count++] - '0');
    }

  /*  
   *  If the stream contains characters after the bit stream then 
   *  process them.
   */
  if ( count ) {
    idx = sprintf( bfr, "%s%s%s", formatInfo->dataFormat, cmt, "%n");
    sscanf( &pBuf[count], bfr, &idx );
    
    count += idx;
    
    *pValue = iValue;
  }

  return count;
}


/*
 * ---------------------------------------------------------------------------
 *  extractStringData()- the data stream is expected to be a string 
 *  which will be converted to integer 
 *  (eg. "abcd" -> 0x61626364 -> 1633837924).
 * 
 */
STATIC long extractStringData( char            *pBuf,
			       char            *format,
			       drvResponseInfo *formatInfo,
			       char            *cmt,
			       long            *pValue )
{
  char     bfr[DRV_SERIAL_BUF_SIZE],
           pData[DRV_SERIAL_BUF_SIZE],
           preAmble[128];

  long     count = 0,
           idx = 0,
           length = 0,
           iValue = 0;

  if ( formatInfo->preAmble ) {
    count = min( sizeof( preAmble ), 
		  formatInfo->preAmbleCnt );
    
    strncpy( preAmble, formatInfo->preAmble, count );
    
    preAmble[count] = '\0';
  }

  sprintf( bfr, "%s%%s%s%s%s", preAmble, cmt, formatInfo->dataFormat, "%n" );

  sscanf( pBuf, bfr, pData, &count );
  
  length = formatInfo->dataCnt - 1;

  if ( pData[0] == '\0' ) return 0;

  /*  
   *  The data stream must be converted from string to integer.
   */
  for ( idx = 0; idx <= length; idx++ ) {
    iValue = (iValue << 8) | (int) pData[length-idx]; 
  }

  *pValue = iValue;

  return count;
}


/*
 * ---------------------------------------------------------------------------
 *  processRealData() - a data stream of type real is expected.
 * 
 */
STATIC long processRealData( char            *pBuf,
			     char            *format,
			     drvResponseInfo *formatInfo,
			     char            *cmt,
			     double          *pValue )
{
  long   count = 0,
         iValue;

  double fValue;

  if ( formatInfo->killAll ) return S_drvAscii_badParam;

  /* 
   *  If a command prompt exists then format the output string and
   *  send it.
   */
  if ( formatInfo->dataType == RBF_INT ) {
    /* 
     *  The data stream is expected to have integer format so process 
     *  the stream and convert the value to real.
     */
    count = extractIntData( pBuf, format, formatInfo, cmt, &iValue ); 

    *pValue = iValue;
  }

  else if ( formatInfo->dataType == RBF_REAL ) {
    /* 
     *  The data stream is expected to have real format so process 
     *  as such and convert to integer.
     */
    count = extractRealData( pBuf, format, formatInfo, cmt, &fValue ); 

    *pValue = fValue; 
  }

  else if ( formatInfo->dataType == RBF_BINARY ) {
    /* 
     *  The data stream is expected to be a bit stream (a series of 
     *  '0' and '1') so process the stream and convert the value to 
     *  integer.
     */
    count = extractBinaryData( pBuf, format, formatInfo, cmt, &iValue ); 

    *pValue = iValue;
  }

  else if ( formatInfo->dataType == RBF_STRING ) {
    /* 
     *  The data stream is expected to be a sequence of ascii chars  
     *  which must be converted to an integer  
     *  (eg.  "abcd" -> 0x61626364 -> 1633837924) and returned as a real
     */
    count = extractStringData( pBuf, format, formatInfo, cmt, &iValue );

    *pValue = iValue;
  }

  else { 
    /*  
     *  No specification exists for the data stream so assume real.
     */
    count = extractRealData( pBuf, format, formatInfo, cmt, &fValue ); 

    *pValue = fValue;
  }

  return count;
}


/*
 * ---------------------------------------------------------------------------
 *  processIntegerData() -  a data stream of type integer is expected.
 * 
 */
STATIC long processIntegerData( char            *pBuf,
				char            *format,
				drvResponseInfo *formatInfo,
				char            *cmt,
				long            *pValue )
{
  long   count = 0,
         iValue;

  double fValue;

  if ( formatInfo->killAll ) return S_drvAscii_badParam;

  /* 
   *  If a command prompt exists then format the output string and
   *  send it.
   */
  if ( formatInfo->dataType == RBF_INT ) {
    /* 
     *  The data stream is expected to have integer format so process 
     *  as such.
     */
     count = extractIntData( pBuf, format, formatInfo, cmt, &iValue ); 

    *pValue = iValue;
  }

  else if ( formatInfo->dataType == RBF_REAL ) {
    /* 
     *  The data stream is expected to have real format.
     */
    count = extractRealData( pBuf, format, formatInfo, cmt, &fValue ); 

    *pValue = (long) fValue;
  }

  else if ( formatInfo->dataType == RBF_BINARY ) {
    /* 
     *  The data stream is expected to be a bit stream (a series
     *  of '0' and '1') so process the stream and convert the value 
     *  to real.
     */
    count = extractBinaryData( pBuf, format, formatInfo, cmt, &iValue ); 

    *pValue = iValue;
  }

  else if ( formatInfo->dataType == RBF_STRING ) {
    /* 
     *  The data stream is expected to be a sequence of ascii chars  
     *  which must be converted to an integer  
     *  (eg.  "abcd" -> 0x61626364 ->1633837924).
     */
    count = extractStringData( pBuf, format, formatInfo, cmt, &iValue );

    *pValue = iValue;
  }

  else { 
    /*  
     *  No specification exists for the data stream so assume integer.
     */
    count = extractIntData( pBuf, format, formatInfo, cmt, &iValue ); 

    *pValue = iValue;
  }

  return count;
}


/*
 * ---------------------------------------------------------------------------
 *  getInteger() - this function is intended to be used to obtain  
 *  initial values, from a remote device, for items/signals/quantities 
 *  which are integer output things.
 *
 *  This function would normally be called during init to obtain intial 
 *  values integer output records (BO, MBBO, and LO ).
 *
 */
STATIC long getInteger( drvCmndPrivate *pCmndPriv,
			void           *pValue, 
			drvAsyncIO     *pIO, 
			drvCmndArg     *pFuncParam )
{
  drvAppPrivate    *pAppPriv = pCmndPriv->pAppPriv;

  drvSerialResponse resp;

  long              status, value, count;
  char              msg[256], *pBuf;

  /* 
   *  If a prompt exists then format the output strin.
   */
  if ( strlen( pFuncParam->rbvPrompt ) > 0 ) {
    
    sprintf( msg, "%s%s", pFuncParam->rbvPrompt, pAppPriv->writeCmt );
    
    /* 
     *  Send the output and wait for the response 
     */
    pCmndPriv->rbvFlag = TRUE;

    status = drvAsciiQueryStringReq( pCmndPriv, msg, &resp );

    pCmndPriv->rbvFlag = FALSE;

    if ( status ) {
      pAppPriv->queryFailures++;
      return S_drvAscii_queryFail;
    }

    pBuf = (char *)resp.buf;


    if ( strlen( pFuncParam->rbvFormat ) > 0 ) {
      /* 
       *  Skip over (kill) any leading characters if so designated 
       *  in the readback format string.
       */
      pBuf = pBuf + pFuncParam->rbvInfo.killCnt;

      /* 
       *  Get the data value as per the specified forma.
       */
      count  = processIntegerData( pBuf, 
				   pFuncParam->rbvFormat, 
				   &pFuncParam->rbvInfo, 
				   pAppPriv->readCmt,
				   &value );

      /* 
       *  If all the data was not cosumed (ie. the stream did not 
       *  match the specification) then return an error.
       */
      if (count + pFuncParam->rbvInfo.killCnt + 1 != resp.bufCount) 
	status = -1;

      else *((long *)pValue) = value;
    }
    else
      /* 
       *  Get the data value as per the default format.
       */
      status = sscanf( pBuf, "%ld", (long *) pValue );

    if ( status != 1 ) {
      pAppPriv->dataErrors++;
      return S_drvAscii_dataErr;
    }    
  }

  /*  
   *  If there was no readback capability then simply return.
   */
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  getReal() - this function is intended to be used to obtain 
 *  initial values, from a remote device, for items/signals/quantities
 *   which are float output things.
 *
 *  This function would normally be called during init to obtain intial 
 *  values for real output records (AO).
 *
 */
STATIC long getReal( drvCmndPrivate *pCmndPriv,
		     void          *pValue, 
		     drvAsyncIO    *pIO, 
		     drvCmndArg    *pFuncParam )
{
  drvAppPrivate    *pAppPriv = pCmndPriv->pAppPriv;

  drvSerialResponse resp;

  long              status,
                    count;
  double            fValue;
  char              msg[256], *pBuf;


  /*  
   *  If a prompt exists then format the output string.
   */
  if ( strlen( pFuncParam->rbvPrompt ) > 0 ) {
    
    sprintf( msg, "%s%s", pFuncParam->rbvPrompt, pAppPriv->writeCmt );

    /*  
     *  Send the output and wait for the response.
     */
    pCmndPriv->rbvFlag = TRUE;

    status = drvAsciiQueryStringReq( pCmndPriv, msg, &resp );

    pCmndPriv->rbvFlag = FALSE;

    if ( status ) {
      pAppPriv->queryFailures++;
      return S_drvAscii_queryFail;


    }

    pBuf = (char *)resp.buf;

    if ( strlen( pFuncParam->rbvFormat ) > 0 ) { 
      /* 
       *  Skip over (kill) any leading characters, if designated  
       *  as such in the readback format string.
       */
      pBuf = pBuf + pFuncParam->rbvInfo.killCnt;

     /*  
      *  Get the data value as per the specified format.
      */
      count  = processRealData( pBuf, 
				   pFuncParam->rbvFormat, 
				   &pFuncParam->rbvInfo, 
				   pAppPriv->readCmt,
				   &fValue );

      /* 
       *  If all the data was not cosumed (ie. the stream did not 
       *  match the specification) then return an error.
       */
      if ( count + pFuncParam->rbvInfo.killCnt +1 != resp.bufCount) 
	status = -1;

      else *((long *)pValue) = fValue;
    }
    else
      /*  
       *  Get the data value as per the default format.
       */
      status = sscanf( pBuf, "%lf", &fValue );

    if ( status != 1 ) {
      pAppPriv->dataErrors++;
      return S_drvAscii_dataErr;
    }    

    *(long *)pValue = fValue * pAppPriv->slope;
  }

  /*  
   *  If there was no readback capability then simply return.
   */
  return S_drvAscii_Ok;
}


/* 
 * ---------------------------------------------------------------------------
 *  processOutputResponse() - process responses from output commands
 *               This is essentially a pattern match to determine if 
 *              a command was rejected.
 */
STATIC void processOutputResponse( drvAsyncIO *pIO )
{
  drvCmndPrivate    *pCmndPriv = &pIO->id;
  drvAppPrivate     *pAppPriv = pCmndPriv->pAppPriv; 
  drvSerialResponse resp;
  char              *pBuf;
  long              status, count, length;

  char              format[2 * MAX_STRING_SIZE + 2];

  status = drvSerialNextResponse( pAppPriv->pLink->linkId, &resp );

  if ( status != S_drvSerial_OK ) {
    logMsg( "%s (%d): {%s <%s><%s>} No response data! \n",
	    (int)__FILE__,(int)__LINE__,(int)pAppPriv->pLink->fileName,
	    (int)pCmndPriv->pCmndArg->cmndPrompt,
	    (int)pCmndPriv->pCmndArg->cmndFormat, 0 );
  }
  else {
    pBuf = (char *)resp.buf;

    /*  
     *  Is there a response string? Should not be here if not! 
     */
    if ( strlen( pCmndPriv->pCmndArg->cmndFormat ) > 0 ) {

      /* 
       *  Should any of the response be ignored? Only the first
       *  'n' chars can be ignored, that is, %nk is only valid
       *  at the beginning of the format string.
       */
      length = strlen( pBuf );

      pBuf = pBuf + pCmndPriv->pCmndArg->cmndInfo.killCnt;

      /* 
       *  No assignments are made for the format string so
       *  all data specs must be of the form %*t. The total
       *  number of chars comsumed in the sscanf is expected
       *  to be the same as the length of data read. If this
       *  is not so then a data error is assumed!
       */
      sprintf( format,"%s%s%s",
	       pCmndPriv->pCmndArg->cmndInfo.dataFormat,
	       pAppPriv->readCmt, "%n" );

      sscanf( pBuf, format, &count );

      if ( (count + pCmndPriv->pCmndArg->cmndInfo.killCnt) != length ) {
	logMsg( "%s (%d): {%s <%s><%s>} response data error \n",
	    (int)__FILE__,(int)__LINE__,(int)pAppPriv->pLink->fileName,
	    (int)pCmndPriv->pCmndArg->cmndPrompt,
	    (int)pCmndPriv->pCmndArg->cmndFormat, (int)0 );
      }
    } 
    else
    /*  The format string does not exist so the record is faulty. 
     *  In actual fact it should not be possible to get here as the 
     *  format string was checked in sendOutput()  
     */
    logMsg( "%s (%d): {%s <%s><%s>} No response format string.\n",
	    (int)__FILE__,(int)__LINE__,(int)pAppPriv->pLink->fileName, 
	    (int)pCmndPriv->pCmndArg->cmndPrompt, 0, 0 );
  }
}
  

/*
 * ---------------------------------------------------------------------------
 *  sendOutput() - determines the mechanism for sending an output 
 *  command. The decision is based on whether or not a response is 
 *  expected or to be ignored.
 */
STATIC long sendOutput( drvAppPrivate     *pAppPriv,
			drvSerialRequest  *pReq, 
			drvAsyncIO        *pIO, 
			drvCmndArg        *pFuncParam )
{
  long status;

  /*  
   *  Is there a response string? 
   */
  if ( strlen( pFuncParam->cmndFormat ) > 0 ) {
    
    /*  
     *  Is the response to be ignored? 
     */
    if ( pFuncParam->cmndInfo.killAll ) {
      /*  
       *  Ignore the response.
       */
      status =  drvAsciiSendIgnoreResp( pIO, pReq );

      if ( status != S_drvSerial_OK ) return status;
      else return S_drvAscii_Ok;
    }
    else {
      /* 
       *  Do not ignore the response (essentially an error check 
       *  on the response) 
       */
      pIO->pAppDrvPrivate = processOutputResponse;

      status = drvAsciiSendCallBackWithResp( pIO, pReq, 0 );
      if ( status ) return status;
      else return S_drvAscii_Ok;
    }
  }
  else
    /*  
     *  No response is expected so simply send the command.
     */ 
    return drvAsciiSend( pAppPriv, pReq );
}


/*
 * ---------------------------------------------------------------------------
 *  writeInteger() - used for outputting strings in which integer data is
 *  to be embedded, typically BO, MBBO, and LO (longout) records.
 */
STATIC long writeInteger( drvAppPrivate *pAppPriv, 
			  void          *pValue, 
			  drvAsyncIO    *pIO, 
			  drvCmndArg    *pFuncParam )
{
  drvSerialRequest req;

  char             format[256],
                   preAmble[128];
  double           fValue;
  long             iValue, iVal,
                   idx, posn, length;
  
  if (pAppPriv->debugFlag & 0x8) printCmndArgs( pFuncParam );

  /* 
   *  If a command prompt exists then format the output string and
   *  send it.
   */
  if ( strlen( pFuncParam->cmndPrompt ) > 0 ) {
    if ( pFuncParam->cmndPromptInfo.dataType == RBF_UNDEFINED ) 
      sprintf( (char *)req.buf, "%s%ld%s", 
	       pFuncParam->cmndPrompt, *(long *)pValue, pAppPriv->writeCmt );

    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_REAL ) {
      fValue = *(long *)pValue;
      sprintf( format, pFuncParam->cmndPrompt, fValue ); 
      sprintf( (char *)req.buf, "%s%s", format, pAppPriv->writeCmt );
    }

    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_INT ) {
      sprintf( format, pFuncParam->cmndPrompt, *(long *)pValue ); 
      sprintf( (char *)req.buf, "%s%s", format, pAppPriv->writeCmt );
    }
    
    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_BINARY ) {
      iValue = *(long *)pValue;
      
      if ( pFuncParam->cmndPromptInfo.preAmble ) {
	length = min( sizeof( preAmble ), 
		      pFuncParam->cmndPromptInfo.preAmbleCnt );

	strncpy( preAmble, pFuncParam->cmndPromptInfo.preAmble, length );
	
	preAmble[length] = '\0';
      }

      else preAmble[0] = '\0';

      posn = 0;
      for ( idx = pFuncParam->cmndPromptInfo.dataCnt - 1; idx >= 0; idx-- ) 
	sprintf( &format[posn++], "%c", 
		 (char)(0x30 + (1 & (iValue >> idx))) );

      format[posn] = '\0';

      sprintf( (char *)req.buf, "%s%s%s%s", preAmble, format, 
	       pFuncParam->cmndPromptInfo.dataFormat, pAppPriv->writeCmt );
    }

    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_STRING ) {
      iValue = *(long *)pValue;

      if ( pFuncParam->cmndPromptInfo.preAmble ) {
	length = min( sizeof( preAmble ), 
		      pFuncParam->cmndPromptInfo.preAmbleCnt );

	strncpy( preAmble, pFuncParam->cmndPromptInfo.preAmble, length );
	
	preAmble[length] = '\0';
      }

      else preAmble[0] = '\0';

      posn = 0;
      for ( idx = pFuncParam->cmndPromptInfo.dataCnt - 1; idx >= 0; idx-- ) {
	/*  
	 *  Must suppress output of null bytes! This ain't pretty and is  
	 *  actually a bug. To correct this is a major change as bufCount 
	 *  would need to be done prior to each of the send calls.        
	 *  Maybe outputting as a string should be disallowed or only     
	 *  allow single character conversions such that a null byte will 
	 *  cause no output. 
	 */
        iVal = iValue >> (8 * idx);
        if ( iVal > 0)
	  sprintf( &format[posn++], "%c", (char)iVal );
      }

      format[posn] = '\0';

      sprintf( (char *)req.buf, "%s%s%s%s", preAmble, format, 
	       pFuncParam->cmndPromptInfo.dataFormat, pAppPriv->writeCmt );
    }

    return sendOutput( pAppPriv, &req, pIO, pFuncParam );
  }
  
  /* 
   *  There is no prompt string so the record is faulty. 
   *  Should not be here as this should have been trapped at init time.
   */
  return S_drvAscii_badParam;
}


/*
 * ---------------------------------------------------------------------------
 *  writeReal() - used for sending floating point values, typically
 *  AI records.
 */
STATIC long writeReal( drvAppPrivate *pAppPriv, 
		       void          *pValue, 
		       drvAsyncIO    *pIO, 
		       drvCmndArg    *pFuncParam )
{
  drvSerialRequest req;

  char             format[256],
                   preAmble[128];
  double           fValue;
  long             iValue,
                   idx, posn, length;
    

  if (pAppPriv->debugFlag & 0x8) printCmndArgs( pFuncParam );

  /* 
   *  The value is always divided by slope (default=1.0) so that the
   *  values can be adjusted to the precision required by a remote
   *  device. This is needed because the rval from an analog output
   *  record is an integer - if one used VAL then the ESLO, ASLO and
   *  ROFF would be circumvented.
   */
  fValue = (double)(*(int *)pValue / pAppPriv->slope);

  /* 
   *  If a command prompt exists then format the output string and
   *  send it.
   */
  if ( strlen( pFuncParam->cmndPrompt ) > 0 ) {
    if ( pFuncParam->cmndPromptInfo.dataType == RBF_UNDEFINED ) 
      sprintf( (char *)req.buf, "%s%f%s", 
	       pFuncParam->cmndPrompt, fValue, pAppPriv->writeCmt );

    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_REAL ) {
      sprintf( format, pFuncParam->cmndPrompt, fValue ); 
      sprintf( (char *)req.buf, "%s%s", format, pAppPriv->writeCmt );
    }

    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_INT ) {
      iValue = fValue;
      sprintf( format, pFuncParam->cmndPrompt, iValue ); 
      sprintf( (char *)req.buf, "%s%s", format, pAppPriv->writeCmt );
    }
    
    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_BINARY ) {
      iValue = fValue;
      
      if ( pFuncParam->cmndPromptInfo.preAmble ) {
	length = min( sizeof( preAmble ), 
		      pFuncParam->cmndPromptInfo.preAmbleCnt );

	strncpy( preAmble, pFuncParam->cmndPromptInfo.preAmble, length );

	posn = 0;
	for ( idx = pFuncParam->cmndPromptInfo.dataCnt - 1; idx >= 0; idx-- ) 
	  sprintf( &format[posn++], "%c", 
		   (char)(0x30 + (1 & (iValue >>idx))) );
	
	sprintf( format, pFuncParam->cmndPrompt, iValue ); 
	sprintf( (char *)req.buf, "%s%s%s%s", preAmble, format, 
		 pFuncParam->cmndPromptInfo.dataFormat, pAppPriv->writeCmt );
      }
    }

    else if ( pFuncParam->cmndPromptInfo.dataType == RBF_STRING ) {
      ; /*  currently undefined */
    }

    return sendOutput( pAppPriv, &req, pIO, pFuncParam );
  }

  /* 
   *  There is no prompt string so the record is faulty. 
   *  Should not be here as this should have been trapped at init time.
   */
  return S_drvAscii_badParam;

}

/*
 * ---------------------------------------------------------------------------
 *  writeString() - used for sending strings 
 */
STATIC long writeString( drvAppPrivate *pAppPriv, 
			 void          *pDataStr, 
			 drvAsyncIO    *pIO, 
			 drvCmndArg    *pFuncParam )
{
   drvSerialRequest req;

  char              buffer[256];

  if (pAppPriv->debugFlag & 0x8) printCmndArgs( pFuncParam );

  /* 
   *  If a command prompt exists then format the output string and
   *  send it.
   */
  if ( strlen( pFuncParam->cmndPrompt ) > 0 ) {
    if ( pFuncParam->cmndPromptInfo.dataType != RBF_UNDEFINED ) {
      sprintf( buffer, "%s%s", pFuncParam->cmndPrompt, pAppPriv->writeCmt );

      sprintf( (char *)req.buf, buffer, (char *)pDataStr );
    }
    else 
      sprintf( (char *)req.buf, "%s%s%s",
	       pFuncParam->cmndPrompt, (char *)pDataStr, pAppPriv->writeCmt );
  }
  else {
    sprintf( (char *)req.buf, "%s%s", (char *)pDataStr, pAppPriv->writeCmt );
  }
 
  return sendOutput( pAppPriv, &req, pIO, pFuncParam );
}


/*
 * ---------------------------------------------------------------------------
 * processIntegerResponse()
 * async function which is the call back for integer data prompts
 */
STATIC void processIntegerResponse( drvAsyncIO *pIO )
{
  drvCmndPrivate   *pCmndPriv = &pIO->id;
  drvAppPrivate    *pAppPriv = pCmndPriv->pAppPriv; 
  drvResponseInfo  *formatInfo;
  
  drvSerialResponse resp;

  char              *pBuf, *format;
  long              status, sstatus = 1;
  long              count, value = 0;
 
  /*  
   *  Fetch the response string.
   */
  status = drvSerialNextResponse( pAppPriv->pLink->linkId, &resp );

  if ( status != S_drvSerial_OK ) {
    /*  
     *  Serial i/o fault so abort.
     */
    (*pIO->pCB)( (void *)pIO->pArg, status, 0 ); 
  }
  else {
    pBuf = (char *)resp.buf;

    if ( pCmndPriv->rbvFlag ) {
      format = pCmndPriv->pCmndArg->rbvFormat;
      formatInfo = &pCmndPriv->pCmndArg->rbvInfo;
    }
    else {
      format = pCmndPriv->pCmndArg->cmndFormat;
      formatInfo = &pCmndPriv->pCmndArg->cmndInfo;
    }

    if ( strlen( format ) > 0 ) {
      /*
       *  If the format string begins with %k or %'n'K then skip/kill 
       *  the specified number of bytes. Note that the special case '%*k' 
       *  (kill all) is trapped before here such that this processing  
       *  never occurs.
       *
       */
      pBuf = pBuf + formatInfo->killCnt;

      count = processIntegerData( pBuf, format, formatInfo, 
				  pAppPriv->readCmt, &value );

      if ( count + formatInfo->killCnt + 1 != resp.bufCount ) sstatus = -1;
    }
    else {
      /* 
       *  Obtain the expected integer value using the default 
       *  format string 
       */
      sstatus = sscanf( pBuf, "%ld", &value );
    }

    if ( sstatus != 1 ) {
      /*  
       *  Response string was not as expected so ensure an error 
       *  is returned
       */
      status = S_drvAscii_dataErr;
      value = 0;
    }
 
    /*  this is the asynchronous callback to device support */
    (*pIO->pCB)( (void *)pIO->pArg, status, value );
  }
}


/*
 * ---------------------------------------------------------------------------
 *  readInteger() - function which prompts for an integer data value
 */
STATIC long readInteger( drvAppPrivate *pAppPriv, 
			 void          *pValue, 
			 drvAsyncIO    *pIO, 
			 drvCmndArg    *pFuncParam )
{
  drvCmndPrivate        *pCmnd = &pIO->id;

  drvSerialRequest req;

  long             status;

  if (pAppPriv->debugFlag & 0x8) printCmndArgs( pFuncParam );

  /* 
   *  If a command prompt exists then format the output string and
   *  send it - error otherwise, which should have been trapped 
   *  during init
   */
  if ( strlen( pFuncParam->cmndPrompt ) > 0 ) {
    sprintf( (char *)req.buf, "%s%s", 
	     pFuncParam->cmndPrompt, pAppPriv->writeCmt );

    /* 
     *  this is a prompt for integer data so setup the appropriate callback
     *  for the response string
     */
    pIO->pAppDrvPrivate = processIntegerResponse;

    pCmnd->rbvFlag = FALSE;

    status = drvAsciiSendCallBackWithResp( pIO, &req, 0 );
    if ( status ) return status;
    /* 
     *  indicate to device support that async completion will occur
     *  (ie. the records PACT will get set)
     */
    else return S_drvAscii_AsyncCompletion;
  }
  else {
    /*  should not get here as this is trapped during init */
    return S_drvAscii_badParam;
  }
}


/*
 * ---------------------------------------------------------------------------
 *  processRealResponse()
 *  async function which is the call back for real value data prompts
 */
STATIC void processRealResponse( drvAsyncIO *pIO )
{
  drvCmndPrivate   *pCmndPriv = &pIO->id;
  drvAppPrivate    *pAppPriv = pCmndPriv->pAppPriv; 
  drvResponseInfo  *formatInfo;

  drvSerialResponse resp;

  char              *pBuf, *format;
  long              status, sstatus = 1;
  long              count;
  double            fValue = 0.0;
 
  /*  fetch the response string */
  status = drvSerialNextResponse( pAppPriv->pLink->linkId, &resp );

  if ( status != S_drvSerial_OK ) {
    /*  serial i/o fault so abort */
    (*pIO->pCB)( (void *)pIO->pArg, status, 0 ); 
  }
  else {
    pBuf = (char *)resp.buf;

    if ( pCmndPriv->rbvFlag ) {
      format = pCmndPriv->pCmndArg->rbvFormat;
      formatInfo = &pCmndPriv->pCmndArg->rbvInfo;
    }
    else {
      format = pCmndPriv->pCmndArg->cmndFormat;
      formatInfo = &pCmndPriv->pCmndArg->cmndInfo;
    }

    if ( strlen( format ) > 0 ) {
      /*
       *  if the format string begins with %k or %'n'K then skip/kill 
       *  the specified number of bytes. Note that the special case '%*k' 
       *  (kill all) is trapped before here such that this processing  
       *  never occurs.
       *
       */
	pBuf = pBuf + formatInfo->killCnt;

      /* 
       *  obtain the expected float value using the specified format string 
       */
      count = processRealData( pBuf, format, formatInfo, 
			       pAppPriv->readCmt, &fValue );

      if ( count + formatInfo->killCnt + 1 != resp.bufCount ) sstatus = -1;
    }
    else {
      /*  obtain the expected float value using the default format string */
      sstatus = sscanf( pBuf, "%lf", &fValue );
    }

    if ( sstatus != 1 ) {
      /*  response string was not as expected so ensure an error is returned*/
      status = S_drvAscii_dataErr;
      fValue = 0.0;
    }
    else {
      status = S_drvAscii_Ok;

      /* 
       *  The value is always multiplied by slope (default=1.0) so that the
       *  values can be adjusted from the precision required by a remote
       *  device. This is needed because the rval for an analog input
       *  record is an integer - there is no way to circumvent conversion
       *  from RVAL to VAl so ESLO, ASLO and ROFF come into play.
       */
      fValue *= pAppPriv->slope;
    }

    /*  this is the asynchronous callback to device support */
    (*pIO->pCB)( (void *)pIO->pArg, status, (long) fValue );
  }
}


/*
 * ---------------------------------------------------------------------------
 *  readReal() - function which prompts for float data
 */
STATIC long readReal( drvAppPrivate *pAppPriv, 
		      void          *pValue, 
		      drvAsyncIO    *pIO, 
		      drvCmndArg    *pFuncParam )
{
  drvCmndPrivate        *pCmnd = &pIO->id;

  drvSerialRequest req;

  long status;
 
  /* 
   *  If a command prompt exists then format the output string and
   *  send it - error otherwise.
   */
  if (pAppPriv->debugFlag & 0x8) printCmndArgs( pFuncParam );

  if ( strlen( pFuncParam->cmndPrompt ) > 0 ) {
    sprintf( (char *)req.buf, "%s%s", 
	     pFuncParam->cmndPrompt, pAppPriv->writeCmt );

    /* 
     *  this is a prompt for float data so setup the appropriate 
     *  callback for the response string
     */
    pIO->pAppDrvPrivate = processRealResponse;

    pCmnd->rbvFlag = FALSE;

    status = drvAsciiSendCallBackWithResp( pIO, &req, 0 );
    if ( status ) return status;
    /* 
     *  indicate to device support that async completion will occur
     *  (ie. the records PACT will get set)
     */
    else return S_drvAscii_AsyncCompletion;
  } 
  else {
    /*  should not get here as this is trapped during init */
    return S_drvAscii_badParam;
  }
}


/*
 * ---------------------------------------------------------------------------
 *  processStringResponse()
 *  async function which is the call back for stringin data prompts
 */
STATIC void processStringResponse( drvAsyncIO *pIO )
{
  drvCmndPrivate    *pCmndPriv = &pIO->id;
  drvAppPrivate     *pAppPriv = pCmndPriv->pAppPriv;
  drvSerialResponse resp;
  long              status;
  

  /*  fetch the response string */
  status = drvSerialNextResponse(pAppPriv->pLink->linkId, &resp);

  if ( status != S_drvSerial_OK ) {
    /*  serial i/o fault so abort */
    (*pIO->pCB)( (void *)pIO->pArg, status, 0 ); 
  }
  else {
    /*  store the resultant response string */
    pCmndPriv->respStrCnt = min( sizeof( pCmndPriv->respStr ), resp.bufCount );

    strncpy( pCmndPriv->respStr, (char *)resp.buf, pCmndPriv->respStrCnt );

    status = S_drvAscii_Ok;
    
    /*  this is the asynchronous callback to device support */
    (*pIO->pCB)((void *)pIO->pArg,status,0);
  }
}


/*
 * ---------------------------------------------------------------------------
 *  readString() - function which prompts for string data
 */
STATIC long readString( drvAppPrivate *pAppPriv, 
			void          *pDataStr, 
			drvAsyncIO    *pIO, 
			drvCmndArg    *pFuncParam )
{
  drvSerialRequest req;

  long             status;
  char             buf[DRV_SERIAL_BUF_SIZE];
  if (pAppPriv->debugFlag & 0x8) printCmndArgs( pFuncParam );

  /* 
   *  If a command prompt exists then format the output string and
   *  send it - error otherwise
   */
  if ( strlen( pDataStr ) > 0 ) 
    sprintf( buf, pFuncParam->cmndPrompt, pDataStr );
  else
    sprintf( buf, "%s", pFuncParam->cmndPrompt );

  /*
   *  note that the entire string is not checked to be greater than
   *  DRV_SERIAL_BUF_SIZE. This is because the default serial buf size is
   *  1024 and the default sizes of the command prompt, the record's val
   *  field, and the write command terminator are much smaller.
   */
  if ( strlen( buf ) > 0 ) 
    sprintf( (char *)req.buf, "%s%s", buf, pAppPriv->writeCmt );
  else
    sprintf( (char *)req.buf, "%s", pAppPriv->writeCmt );

  pIO->pAppDrvPrivate = processStringResponse; 
  
  status = drvAsciiSendCallBackWithResp( pIO, &req, 0 );
  if ( status ) return status;
  else return S_drvAscii_AsyncCompletion;

  return S_drvAscii_badParam;

}


/*
 * ---------------------------------------------------------------------------
 *  noOutputReadFunc() - calling this function indicates that a wrong 
 *  operation (read or write) was performed for a record.
 */
STATIC long noOutputReadFunc( drvCmndPrivate *pCmndPriv,
			      void           *pValue, 
			      drvAsyncIO     *pIO, 
			      drvCmndArg     *pFuncParam )
{
  return S_drvAscii_writeOnlyParameter;
}



/*
 * ---------------------------------------------------------------------------
 *  setConnState() - allows the link state to be changed from not 
 *  connected (0) to connected (1) and visa versa
 *
 */
STATIC long setConnState( drvAppPrivate *pAppPriv, 
			  void          *pValue, 
			  drvAsyncIO    *pIO, 
			  drvCmndArg    *pFuncParam )
{
  pAppPriv->state.noComm = !(*(long *)pValue);

  if ( *(long *)pValue ) {
    epicsPrintf( "%s (%d): Reinitializing port %s\n", 
		 __FILE__, __LINE__, pAppPriv->pLink->fileName );
    assert( pAppPriv->pLink );

  }

  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 * readConnState() - returns a link's current connection state
 *  currently a link can only be marked as not connected if a fault occurs
 *  during init, or from a database record (ie. the user can force the 
 *  condition
 */
STATIC long readConnState( drvAppPrivate *pAppPriv, 
			   void          *pValue, 
			   drvAsyncIO    *pIO, 
			   drvCmndArg    *pFuncParam)
{
  getConnState( &(pIO->id), pValue, NULL,  pFuncParam );
  
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  setSlope() - sets the slope for the link.
 *  slope is a value by which all float output values are divided by 
 *  and all float input values are multiplied by. The default value 
 *  is 1.0.
 *
 *  Slope is needed because: the RVAL for an analog input record is an 
 *  integer and there is no way to circumvent conversion from RVAL to VAL 
 *  so ESLO, ASLO and ROFF come into play; the RVAL from an analog output
 *  record is an integer and if one used VAL then the ESLO, ASLO and ROFF 
 *  would be circumvented.
 *
 */
STATIC long setSlope( drvAppPrivate *pAppPriv, 
		      void          *pValue, 
		      drvAsyncIO    *pIO, 
		      drvCmndArg    *pFuncParam)
{
 
  pAppPriv->slope = *(long *)pValue; 

  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 * setTimeout() - allows configuration of the maximum time allowed for a
 * response to a data prompt (default is 10 seconds). The originating record
 * can be a float or integer
 *
 */
STATIC long setTimeout( drvAppPrivate *pAppPriv, 
			void          *pValue, 
			drvAsyncIO    *pIO, 
			drvCmndArg    *pFuncParam)
{
  if ( *(long *)pValue <= 0 )
    return S_drvAscii_badParam;

  pAppPriv->responseTMO = *(long *)pValue * sysClkRateGet(); 
  
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  formatCmt - internal function for processing a command terminator 
 *              string
 *
 */
STATIC long formatCmt( char *pIn, char *pOut, int oStrSize )
{
  unsigned int val;
  int          oIdx = 0;

  while ( *pIn != '\0') {
    /*
     *  process any escape codes
     */
    if ( *pIn == '\\' ) {
      pIn++;
      if ( *pIn == 'b' )
	pOut[oIdx] = '\b';
      else if ( *pIn == 'f' )
	pOut[oIdx] = '\f';
      else if ( *pIn == 'n' )
	pOut[oIdx] = '\n';
      else if ( *pIn == 'r' )
	pOut[oIdx] = '\r';
      else if ( *pIn == 't' )
	pOut[oIdx] = '\t';
      else if ( *pIn == 'v' )
	pOut[oIdx] = '\v';
      else if ( *pIn == '\\' )
	pOut[oIdx] = '\\';
      else if ( (int)*pIn == 0x27 )
	pOut[oIdx] = '\'';
      else if ( (int)*pIn == 0x22 )
	pOut[oIdx] = '\"';
      else if ( isdigit(*pIn) ) {
	/* a numeric escape code */
	sscanf( pIn, "%3o", &val );
	pOut[oIdx] = (char)val;

	if (  isdigit( *(++pIn) ) ) pIn++; 
	if ( !isdigit( *(++pIn) ) ) pIn--;
      }
      else {
	/* 
	 * not an escape code so include the 
	 * two characters in the data
	 */
	pOut[oIdx++] = '\\';

	if ( oIdx >= oStrSize )
	  return S_drvAscii_badParam;

	pOut[oIdx] = *pIn;
      }
    }
    else
      pOut[oIdx] = *pIn;

    pIn++;

    if ( ++oIdx >= oStrSize )
      return S_drvAscii_badParam;
  }

  pOut[oIdx] = '\0';

  return S_drvAscii_Ok;
}

/*
 * ---------------------------------------------------------------------------
 *  setWriteCmt() - sets the command terminator for output strings. 
 *
 *  This is a string output function so the orginating record must be
 *  a stringOutput record (no check is made).
 *
 */
STATIC long setWriteCmt( drvAppPrivate *pAppPriv, 
			 void          *pDataStr, 
			 drvAsyncIO    *pIO, 
			 drvCmndArg    *pFuncParam)
{
  char str[MAX_STRING_SIZE+1];

  if ( !(char *)pDataStr ||  ( strlen((char *)pDataStr) <= 0 ) ) 
    return S_drvAscii_badParam;

  if ( formatCmt( (char *)pDataStr, str, sizeof( str ) - 1 ) )
    return S_drvAscii_badParam;

  str[MAX_STRING_SIZE] = '\0';

  strcpy( pAppPriv->writeCmt, str );

  pAppPriv->writeCmt[sizeof( pAppPriv->writeCmt ) - 1] = '\0';

  return S_drvAscii_Ok;
}

/*
 * ---------------------------------------------------------------------------
 *  setReadCmt() - sets the command terminator for input strings. 
 *
 *  This is a string output function so the orginating record must be
 *  a stringOutput record (no check is made).
 */
STATIC long setReadCmt( drvAppPrivate *pAppPriv, 
			void          *pDataStr, 
			drvAsyncIO    *pIO, 
			drvCmndArg    *pFuncParam)
{
  char str[MAX_STRING_SIZE+1];
 
  if ( strlen( (char *)pDataStr ) > sizeof( pAppPriv->readCmt ) - 1 )
    return S_drvAscii_badParam;

  if ( formatCmt( (char *)pDataStr, str, sizeof( str) - 1 ) )
    return S_drvAscii_badParam;

  str[MAX_STRING_SIZE] = '\0';

  strcpy( pAppPriv->readCmt, str );

  pAppPriv->readCmt[sizeof( pAppPriv->readCmt ) - 1] = '\0';

  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  setDebugFlag() - enables debug for a link.
 *                   should be from a longout record
 */
STATIC long setDebugFlag( drvAppPrivate *pAppPriv, 
			  void          *pValue, 
			  drvAsyncIO    *pIO, 
			  drvCmndArg    *pFuncParam)
{
 
  pAppPriv->debugFlag = *(long *)pValue; 

  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiLoadReq()
 */
STATIC void drvAsciiLoadReq( drvSerialRequest *pReq, const char *pMsg )
{
  unsigned char 	*pBuf = pReq->buf;
  
  while ( *pMsg ) {
    *pBuf++ = (unsigned char) *pMsg++;
  }
  *pBuf = *pMsg; /* add nill term */
  pReq->bufCount = pBuf - pReq->buf;
  assert ( pReq->bufCount < sizeof( pReq->buf ) );
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiQueryStringReq() 
 *
 *  This function which prompts for and waits for a response. 
 *
 *  THIS IS NOT ASYNCHRONOUS PROCESSING. 
 *
 *  Currently this is only called from get...() functions which 
 *  prompt for initial values at startup.
 */
STATIC int drvAsciiQueryStringReq( drvCmndPrivate    *pCmndPriv,
				   const char        *pMsg,
				   drvSerialResponse *pRes)
{
  drvSerialRequest	req;

  /* setup the request buffer */
  drvAsciiLoadReq( &req, pMsg );

  /* send the request and wait for the response */
  return drvAsciiQuery( pCmndPriv, &req, pRes );
}


/*
 *  responseNotifier()
 *
 *  This function serves as a callback which simply notifies, the 
 *  initiator of a message transaction, that the response has
 *  been received. This is done via a message private semaphore
 *  upon which the initiator is supposedly waiting.
 *
 *  This function executes within the context of the serial write task.
 */
STATIC void responseNotifier( drvAsyncIO *pIO )
{
  int status;

  if ( pIO->pArg != NULL ) {
    status = semGive( (SEM_ID)(pIO->pArg) );
    
    if ( status != OK ) 
      logMsg( "%s %d: lost sync - recovering\n", 
	      (int)__FILE__, (int)__LINE__, 0,0,0,0 );
  }

  return;
}


/*
 * ---------------------------------------------------------------------------
 * drvAsciiQuery()
 *
 *  Attempts to send a message to a motor controller, wait for and
 *  fetch a response, then return the response to the caller for 
 *  processing.
 *
 *  This function executes within the context of the calling task
 *  and effectively blocks that task until a response is received
 *  from the remote device, albeit there is a timeout.
 */
STATIC int drvAsciiQuery( drvCmndPrivate    *pCmndPriv,
			  drvSerialRequest  *pReq, 
			  drvSerialResponse *pRes)
{
  drvAppPrivate  *pAppPriv = pCmndPriv->pAppPriv;
  drvAsyncIO     *pIO;
  SEM_ID          responseSem;

  long		  status;

  /*
   *  Create a semaphore private to this message transaction. If this
   *  fails then an error is expressed and the message is aborted.
   *  All other errors require that an immediate exit not occur so
   *  that the semaphore can be deleted prior to exiting.
   */
  responseSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
  if ( responseSem == NULL ) {
    epicsPrintf( "%s %d: could not allocate semaphore. Message aborted!\n",
		 __FILE__, __LINE__ );
    
    return S_drvAscii_noMemory;
  }

  /*
   *  Create and Fill in the callback information. In this case the 
   *  notifier will simply give the semaphore. Note that the notifier 
   *  executes within the context of the serial write task.
   *
   *  Note that this will be done only once per records for which an
   *  initial value can be optained during record init. Hence we do
   *  not attempt to reclaim the memory.
   */ 
  pCmndPriv->pRbvArg = (void *) calloc( 1, sizeof( drvAsyncIO ) );
  pIO = (drvAsyncIO *) (pCmndPriv->pRbvArg);

  pIO->pArg = (void *)  responseSem;
  pIO->pAppDrvPrivate = responseNotifier;

  /*
   *  Attempt to send the message.
   */
  status = drvAsciiSendCallBackWithResp( pIO, pReq, 1 );
  
  if ( status == OK ) {      
    /*
     *  Wait for the response to be available. The notification callback
     *  will give this semaphore. If the give does not occur in a timely
     *  manner then it will be set to null in the callback structure, so
     *  that the notifier will not croak, before the semaphore is deleted.
     */
    status = semTake( responseSem, pAppPriv->responseTMO );
    
    if ( status == OK ) {
      /*
       *  Fetch the response.
       */      
      status = drvSerialNextResponse( pAppPriv->pLink->linkId, pRes );    

    }

    if ( status != S_drvSerial_OK ) {
      epicsPrintf( "%s %d: [%s] response failed (%d)\n", 
		   __FILE__, __LINE__, 
		   pAppPriv->pLink->fileName, (int)status );
      
    }
  }

  /*
   *  Give the semphore which will let the next message be transmitted.
   *  That is, let the next write-read cycle proceed. Note that this
   *  semaphore was taken in the put routine so as to block out other
   *  writers until such time as the response is fetched. Thus the serial
   *  write task is essentially blocked until the semaphore is given.
   */
  if (pAppPriv->debugFlag) printf("drvAsciiQuery: semGive syncLock\n");
  assert( semGive( pAppPriv->pLink->syncLock ) == OK );
  
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 * drvAsciiSend() 
 *
 *  This function simply queues a 'datagram' to the serial write task.
 */
STATIC int drvAsciiSend( drvAppPrivate *pAppPriv, drvSerialRequest *pReq )
{
  long			status;

  /*
   *  Set up the function to be use by the serial write task
   *  to transmit the message.
   */
  pReq->pCB = putFrame;
  pReq->pAppPrivate = pAppPriv;
  pReq->bufCount = strlen( (char *)pReq->buf );

  /*
   *  Attempt to queue the message in the serial write task.
   */
  status = drvSerialSendRequest( pAppPriv->pLink->linkId, dspLowest, pReq );

  if ( status != S_drvSerial_OK ) {
    epicsPrintf( "%s %d : [%s] send failed (%d)!\n",
		 (int)__FILE__, (int)__LINE__,
		 pAppPriv->pLink->fileName, status );

    return status;
  }
   
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiSendIgnoreResp() 
 *
 *  This function executes within the context of the caller.
 *
 *  A transmission function is setup which will wait for a
 *  reply from the motor controller. However, that response is
 *  simply discarded. Hence, once this function queues the message
 *  to the serial write task, the caller can continue (it is not blocked).
 *  Blocking for the response will occur within the context of the
 *  serial write task.
 */
STATIC int drvAsciiSendIgnoreResp( drvAsyncIO *pIO, 
				   drvSerialRequest *pReq )
{
  drvCmndPrivate        *pCmnd = &pIO->id;
  drvAppPrivate	        *pAppPriv = pCmnd->pAppPriv; 
  
  long			status;

  /*
   *  Setup the function to be used by the drvSerial write task 
   */
  pReq->pCB = putFrameAndRemoveResp;
  pReq->pAppPrivate = pIO;
  pReq->bufCount = strlen( (char *)pReq->buf );

  /* 
   *  Attempt to queue the message in the serial write task.
   */
  status = drvSerialSendRequest (pAppPriv->pLink->linkId, dspLowest, pReq );

  if ( status != S_drvSerial_OK ) {
    epicsPrintf( "%s %d : [%s] send failed (%d)!\n",
		 (int)__FILE__, (int)__LINE__, 
		 pAppPriv->pLink->fileName, status );

    return status;
  }
 
  return S_drvAscii_Ok;
}


/*
 * ---------------------------------------------------------------------------
 *  drvAsciiSendCallBackWithResp() 
 *  This function executes within the context of the caller.
 *
 *  A transmission function is setup which will wait for a
 *  reply from the motor controller. When the response is
 *  received a callback function, which must have been setup
 *  outside of this function, will be called so as to process
 *  said response. 
 *
 *  Note that this provides for an asynchronous callback such
 *  that the caller need not be blocked. However, if the caller
 *  specifies noUnlock then it is expected that the caller is
 *  blocking (using a private wakeup mechanism) and will free
 *  the link (allow succeeding write-read cycles) by giving
 *  the syncLock semaphore which would have been taken
 *  within putFrameAndBlock. Such is the case
 *  for drvAsciiQuery().
 *
 *  If noUnlock is not specified then the syncLock semaphore
 *  will be taken prior to transmission of the command and given
 *  after the callback function processes the response. 
 *
 *  In either case the blocking for the response will occur within
 *  the context of the serial write task. However, in the noUnlock
 *  case, the caller will also be block.
 *
 */
STATIC int drvAsciiSendCallBackWithResp( drvAsyncIO *pIO, 
					 drvSerialRequest *pReq,
					 int noUnlock )
{
  drvCmndPrivate        *pCmnd = &pIO->id;
  drvAppPrivate	        *pAppPriv = pCmnd->pAppPriv; 
  long			status;

  /* 
   *  setup the send function to be used by the drvSerial write task 
   */
  if ( noUnlock )
    pReq->pCB = putFrameAndBlock;
  else
    pReq->pCB = putFrameAndCallBack;

  pReq->pAppPrivate = pIO;

  pReq->bufCount = strlen( (char *)pReq->buf );

  /* 
   *  Queue the message in the drvSerial write task. 
   */
  status = drvSerialSendRequest( pAppPriv->pLink->linkId, dspLowest, pReq );

  if ( status != S_drvSerial_OK ) {
    epicsPrintf( "%s %d : [%s] send failed (%d)!\n",
		 __FILE__, __LINE__, pAppPriv->pLink->fileName, status );

    return status;
  }

  return S_drvAscii_Ok;
}


/* 
 * ---------------------------------------------------------------------------
 *  putFrameAndCallBack() 
 *  This runs within the context of the serial write task. It causes
 *  the write task to wait for a response via a semaphore given by
 *  the corresponding read function (getFrame) which runs within
 *  the context of the serial read task. The effect is that another
 *  write will not proceed until the read completes hence a 
 *  synchronized write-read cycle. The actual fetching and processing
 *  of the response is done within the callback routine which will
 *  run within the context of the serial write task, that is, the
 *  callback is invoked herein.
 *
 *  The upshot is that blocking for a response occurs within the 
 *  context of the serial write task, rather than within the context
 *  of task which initiated the message.
 */

STATIC int putFrameAndCallBack( FILE             *fp, 
				drvSerialRequest *pReq )
{
  drvAsyncIO	        *pIO = (drvAsyncIO *)pReq->pAppPrivate;
  drvCmndPrivate        *pCmnd = &pIO->id;
  drvAppPrivate	        *pAppPriv = pCmnd->pAppPriv;
  drvSerialResponse	 response;
  int			 status;
  int			 semStatus;

  if (pAppPriv->debugFlag) printf("PFACB: Enter\n");
  /*
   *  This thread will wait for a reply so make sure that no 
   *  other thread steals that reply. Must also wait for any other
   *  write-read cycles to complete (with timeout).
   */
  if (pAppPriv->debugFlag) {
	printf("PFACB: semTake syncLock: %x\n", 
		pAppPriv->pLink->syncLock);
  }

  status = semTake( pAppPriv->pLink->syncLock, pAppPriv->responseTMO );
  if (pAppPriv->debugFlag && status != OK) 
	printf("semTake failed with tmo of %d, status=%x\n", pAppPriv->responseTMO, status);

  status = 0;
  if ( status == OK ) {
    /*
     *  At this point the reponse queue should be empty.
     *  If it is not empty then write-read synchronization
     *  has been lost. Thus any response must be purged.
     */
    do {

      status = drvSerialNextResponse( pAppPriv->pLink->linkId, &response );
      
      if ( status == S_drvSerial_OK ) {
	if ( pAppPriv->debugFlag && response.bufCount > 0 ) {
	  logMsg( "%s %d: [%s] lost msg sync - discarding response <%s>\n", 
		  (int)__FILE__, (int)__LINE__,
		  (int)pAppPriv->pLink->fileName, 
		  (int)(&(response.buf[0])), 0,0 );
	}
      }

    } while ( status == S_drvSerial_OK );
    /*
     *  Clear the semaphore which indicates a response has been
     *  received. Normally it should be empty. If its not empty
     *  then syncrhonization has been lost, so generate a warning  
     *  and continue.
     */
    if (pAppPriv->debugFlag) printf("PFACB: semTake sendBlockSem NO_WAIT\n");
    status = semTake( pAppPriv->pLink->sendBlockSem, NO_WAIT );
    if ( pAppPriv->debugFlag && status == OK ) {
    logMsg( "%s (%d): [%s] lost sem - recovering\n", 
	    (int)__FILE__, (int)__LINE__,
	    (int)pAppPriv->pLink->fileName, 0,0,0 );
    }
    /*
     *  Attempt to send a query.
     */
    pReq->pAppPrivate = pAppPriv;
    
    pCmnd->pAppPriv->recordCnt = 
      pCmnd->rbvFlag ? pCmnd->pCmndArg->rbvInfo.recordCnt:
      pCmnd->pCmndArg->cmndInfo.recordCnt;
    
    status = putFrame( fp, pReq );
    
    if ( status == OK ) {  
      /*
       *  and block for the response but don't wait forever
       */
      if (pAppPriv->debugFlag) printf("PFACB: semTake sendBlockSem\n");
      semStatus = semTake( pAppPriv->pLink->sendBlockSem, 
			   pAppPriv->responseTMO );
      if ( semStatus != OK ) {
	if (pAppPriv->debugFlag) 
	logMsg( "%s (%d) : [%s] receive failed (%x)\n",
		(int)__FILE__, (int)__LINE__, 
		(int)pAppPriv->pLink->fileName, semStatus, 0,0 );

	if (pIO->pCB) {
	  (*pIO->pCB)( (void *)pIO->pArg, S_drvAscii_queryFail, 0 ); 
	}

	/* 
	 *  Cannot return EOF or all outstanding requests will be  
	 *  dropped on floor as drvSerial will close and reopen its 
	 *  link. The act of terminating all outstanding requests will 
	 *  cause scanning of all asynchronous completion records to be 
	 *  blocked as their pact's will be true.
	 */
	status = 0;

      } else {
	/*
	 *  call the call back
	 */
	(*pIO->pAppDrvPrivate)( pIO );

	status = 0;
      }

    } else {
      if (pAppPriv->debugFlag)
      logMsg( "%s %d: [%s] send failed (%d) for <%s>\n", 
	      (int)__FILE__, (int)__LINE__ ,
	      (int)pAppPriv->pLink->fileName, status, 
	      (int)(&(pReq->buf[0])), 0 );
      
      status = 0;
    }

    /*
     *  Unblock anyone waiting to do a write-read cycle. In this case
     *  the response should have been fetched from within the previously
     *  called callback.
     */
    if (pAppPriv->debugFlag) {
	printf("PFACB: semGive syncLock: %x\n",
		pAppPriv->pLink->syncLock);
    }

    status = semGive( pAppPriv->pLink->syncLock);
    assert( status == OK);


  } else {
    if (pAppPriv->debugFlag)
    logMsg( "%s %d : [%s] semTake failed for Sync semaphore!\n",
	    (int)__FILE__, (int)__LINE__, 
	    (int)pAppPriv->pLink->fileName, 0,0,0 );

    /* 
     *  should the semaphore be given here?
     *  if its not given here is the serial write task 
     *  blocked indefinitely?
     */

    if (pAppPriv->debugFlag) printf("PFACB: Into the undiscovered country\n");
  }

  if (pAppPriv->debugFlag) printf("PFACB: Exit\n");
  return status;
}


/* 
 * ---------------------------------------------------------------------------
 *  putFrameAndBlock() 
 * 
 *  This runs within the context of the serial write task. The
 *  serial write task is essentially blocked until such time as
 *  the message initiating task gives the locking semaphore,
 *  syncLock, as discussed below.
 *
 *  This is similar to putFrameAndCallBack() except that the 
 *  write-read cycle remains locked (semaphore is not given). It 
 *  is the responsibilty of the task which initiated the message 
 *  to unlock the cycle by giving the syncLock semaphore.
 *
 *  The simplest means of accomplishing this is for the initiator
 *  to setup a callback to provide some form of response ready
 *  notification back to the initiator, which will then give the 
 *  semaphore. Within drvAsciiQuery() the handshake with the 
 *  notification function is done with a message private semaphore 
 *  and, in this case, the initiator is effectively blocking for the  
 *  response (ie. a synchronous write-read). Note that the initiator 
 *  need not block if a sufficient callback is setup. 
 *
 *  Only the initial value read functions (getReal and getInteger) 
 *  currently use drvAsciiQuery, thus the blocking occurs during 
 *  record init and not otherwise (ie. the epics scan tasks never  
 *  block because of this behaviour).
 *  
 */
STATIC int putFrameAndBlock( FILE *fp, drvSerialRequest *pReq )
{
  drvAsyncIO	        *pIO = (drvAsyncIO *)pReq->pAppPrivate;
  drvCmndPrivate        *pCmnd = &pIO->id;
  drvAppPrivate	        *pAppPriv = pCmnd->pAppPriv;
  drvSerialResponse	 response;
  int			 status;
  int			 semStatus;

  /*
   *  This thread will wait for a reply so make sure that no 
   *  other thread steals that reply. Must also wait for any other
   *  write-read cycles to complete (with timeout).
   */
  if (pAppPriv->debugFlag) printf("PFAB: semTake syncLock\n");
  status = semTake( pAppPriv->pLink->syncLock, pAppPriv->responseTMO );

  if ( status == OK ) {
    /*
     *  At this point the reponse queue should be empty.
     *  If it is not empty then write-read synchronization
     *  has been lost. Thus any response must be purged.
     */
    do {
      status = drvSerialNextResponse( pAppPriv->pLink->linkId, &response );

      if ( status == S_drvSerial_OK ) {
	if ( response.bufCount > 0 ) {
	  logMsg( "%s %d: [%s] lost msg sync - discarding response\n", 
		  (int)__FILE__, (int)__LINE__, 
		  (int)pAppPriv->pLink->fileName, 0,0,0 );
	}
      }

    } while ( status == S_drvSerial_OK );

    /*
     *  Clear the semaphore which indicates a response has been
     *  received. Normally it should be empty. If its not empty
     *  then syncrhonization has been lost, so generate a warning  
     *  and continue.
     */
    semStatus = semTake( pAppPriv->pLink->sendBlockSem, 
			 pAppPriv->responseTMO );

    if ( semStatus == OK ) {
      logMsg( "%s %d: [%s] lost sync (%d)- recovering\n", 
	      (int)__FILE__, (int)__LINE__, 
	      (int)pAppPriv->pLink->fileName, semStatus, 0,0 );
    }
    
    /*
     *  Attempt to send a query.
     */
    pCmnd->pAppPriv->recordCnt = 
      pCmnd->rbvFlag ? pCmnd->pCmndArg->rbvInfo.recordCnt
      : pCmnd->pCmndArg->cmndInfo.recordCnt;
    
    status = putFrame( fp, pReq );
    
    if ( status == OK ) {  
      /*
       *  and block for the response.
       */
      semStatus = semTake( pAppPriv->pLink->sendBlockSem, 
			 pAppPriv->responseTMO );
      if ( semStatus != OK ) {
	if (pAppPriv->debugFlag)
	logMsg( "%s %d: [%s] receive failed (%d)!\n", 
		(int)__FILE__, (int)__LINE__, 
		(int)pAppPriv->pLink->fileName, semStatus, 0,0 );
	status = 0;

      } else {
	/*
	 *  Call the callback which will fetch and process the response.
	 */
        (*pIO->pAppDrvPrivate)( pIO );
	
	status = OK;

      }

    } else {
      logMsg( "%s %d: [%s] send failed (%d)\n", 
	      (int)__FILE__, (int)__LINE__, 
	      (int)pAppPriv->pLink->fileName, status, 0,0 );
      
      status = 0;
    }

    /*
     *  The write-read cycle semaphore (syncLock) is not given
     *  here as the response will be fetched within the context of
     *  another task. Said task is responsible for giving the semaphore.
     *  If that task does not give the semaphore then the serial write
     *  task is permanently blocked (relies on a timeout and link 
     *  reset in the write task).
     *  Note that the sempahore is given via a function within this 
     *  driver so it is reasonably safe.
     */

  } else {
    logMsg( "%s %d : [%s] semTake failed (%d) for Sync semaphore!\n",
	    (int)__FILE__, (int)__LINE__, 
	    (int)pAppPriv->pLink->fileName, status, 0,0 );

    /* 
     *  should the semaphore be given here?
     *  if its not given is the serial write task blocked indefinitely?
     */
  }


  return status;
}


/* 
 * ---------------------------------------------------------------------------
 *  putFrameAndRemoveResp()
 *
 *  This runs within the context of the serial write task. It causes
 *  the write task to wait for a response via a semaphore given by
 *  the corresponding read function (getFrame) which runs within
 *  the context of the serial read task. The effect is that another
 *  write will not proceed until the read completes hence a 
 *  synchronized write-read cycle. This function also fetches and 
 *  forgets/ignores the response. 
 *
 *  The upshot is that blocking and disposing of a response occurs 
 *  within the context of the serial write task, rather than within 
 *  the context of task -most likely an epics scan task- which 
 *  initiated the message.
 *
 */
STATIC int putFrameAndRemoveResp( FILE *fp, drvSerialRequest *pReq )
{
  drvAsyncIO	        *pIO = (drvAsyncIO *)pReq->pAppPrivate;
  drvCmndPrivate        *pCmnd = &pIO->id;
  drvAppPrivate	        *pAppPriv = pCmnd->pAppPriv;
  drvSerialResponse	 response;
  int			 status;
  int			 semStatus;

  /*
   *  This thread will wait for a reply so make sure that no 
   *  other thread steals that reply. Must also wait for any other
   *  write-read cycles to complete (with timeout).
   */
  if (pAppPriv->debugFlag) printf("PFARR: semTake syncLock\n");
  status = semTake( pAppPriv->pLink->syncLock, pAppPriv->responseTMO );

  if ( status == OK ) {
    /*
     *  At this point the reponse queue should be empty.
     *  If it is not empty then write-read synchronization
     *  has been lost. Thus any response must be purged.
     */
    do {
      status = drvSerialNextResponse( pAppPriv->pLink->linkId, &response );
      
      if ( status == S_drvSerial_OK ) {
	if ( response.bufCount > 0 ) {
	  logMsg( "%s %d: [%s] lost msg sync - discarding response\n", 
		  (int)__FILE__, (int)__LINE__,
		  (int)pAppPriv->pLink->fileName, 0,0,0);
	}
      }

    } while ( status == S_drvSerial_OK );

    /*
     *  Clear the semaphore which indicates a response has been
     *  received. Normally it should be empty. If its not empty
     *  then syncrhonization has been lost, so generate a warning  
     *  and continue.
     */
    status = semTake( pAppPriv->pLink->sendBlockSem, NO_WAIT );
    if ( status == OK ) {
      logMsg( "%s (%d): [%s] lost sem - recovering\n", 
	      (int)__FILE__, (int)__LINE__,
	      (int)pAppPriv->pLink->fileName, 0,0,0 );
    }

    /*
     *  Attempt to send a query.
     */
    pCmnd->pAppPriv->recordCnt = 
      pCmnd->rbvFlag ? pCmnd->pCmndArg->rbvInfo.recordCnt
      : pCmnd->pCmndArg->cmndInfo.recordCnt;
    
    status = putFrame( fp, pReq );
    
    if ( status == OK ) {  
      /*
       *  and block for its response
       */
      semStatus = semTake( pAppPriv->pLink->sendBlockSem, 
			   pAppPriv->responseTMO );
      if ( semStatus != OK ) {
	logMsg( "%s %d: [%s] semTake failed (%d) for send semaphore\n", 
		(int)__FILE__, (int)__LINE__, 
		(int)pAppPriv->pLink->fileName, semStatus, 0,0 );
	
	status = 0;

      } else {
	/*
	 *  Discard the response.
	 */
	status = drvSerialNextResponse( pAppPriv->pLink->linkId, &response );

	if ( status ) {
	  if (pAppPriv->debugFlag)
	  logMsg( "%s %d : [%s] receive failed (%d)!\n",
		  (int)__FILE__, (int)__LINE__, 
		  (int)pAppPriv->pLink->fileName, status,0,0 );

	  status = 0;

	} else { 
	  status = OK;
	}
      }

    } else {
      logMsg( "%s %d: [%s] send failed (%d)\n", 
	      (int)__FILE__, (int)__LINE__, 
	      (int)pAppPriv->pLink->fileName, status, 0,0 );
      
      status = 0;
      
    }

    /*
     *  Unblock anyone waiting to do a write-read cycle.
     */
    if (pAppPriv->debugFlag) printf("PFARR: semGive syncLock\n");
    status = semGive( pAppPriv->pLink->syncLock );
    assert( status == OK );
    
  } else {
    logMsg( "%s %d : [%s] semTake failed for Sync semaphore!\n",
	    (int)__FILE__, (int)__LINE__,
	    (int)pAppPriv->pLink->fileName, 0,0,0 );
    
    /* 
     *  should the semaphore be given here?
     *  if its not given is the serial write task blocked indefinitely?
     */

  }
  
  return status;
}


/* 
 * ---------------------------------------------------------------------------
 *  putFrame()
 *
 *  This function executes within the context of the serial 
 *  write task.
 *
 *  This function outputs a message to a remote device one
 *  character at a time.
 *
 *  This function should not be called directly as it would bypass
 *  the queued messages, within the serial write task, as well as
 *  bypass all synchronizing semaphores (ie. messages and response
 *  could become interleaved).
 */
STATIC int putFrame( FILE *fp, drvSerialRequest *pReq )
{
  uint8_t       *pC;
  int		status = S_drvAscii_Ok;
  drvAppPrivate *pAppPriv = (drvAppPrivate *)(pReq->pAppPrivate);

  /*
   *  add the message body
   */
  for ( pC = pReq->buf; pC < &pReq->buf[pReq->bufCount]; pC++ ) {
    status = putc( *pC, fp );

    if ( status == EOF ) {
      logMsg( "%s %d : [%s] cannot out'put' character!\n",
	      (int)__FILE__, (int)__LINE__,
	      (int)pAppPriv->pLink->fileName, 0,0,0 );
      return EOF;
    }
  }

  /* 
   *  force the data out on the serial link 
   */
  fflush( fp );
  
  if ( pAppPriv->debugFlag ) {
    /*
     *  Do not use epicsPrintf as deadlock over a semaphore 
     *  could occur.
     */
    printf( "drvAscii(%d) => ", pReq->bufCount );
    printString( pReq->bufCount, (char *)(pReq->buf) );
    printf( "\n" );
  }

  return OK;
}


/* 
 * ---------------------------------------------------------------------------
 * getFrame()
 * 
 *  This function executes within the context of the serial 
 *  read task.
 *
 *  This function inputs a message from a remote device one
 *  character at a time.
 *
 *  This function provides a notification that a complete message
 *  has been received by giving a link private semaphore, sendBlockSem, 
 *  upon which it is assumed that a reader is blocked. Currently, in
 *  all cases within this driver, it is effectively the serial
 *  write task that would be blocked.
 *
 *  This function should not be called directly as it would bypass
 *  the queued messages, within the serial write task, as well as
 *  bypass all synchronizing semaphores (ie. messages and response
 *  could become interleaved).
 *  
 *  Note that a link can have only one command terminator. This means 
 *  that all remote devices attached to a single serial port must have 
 *  the same command terminator. This is because there is only one rx 
 *  framing routine associated with the link.
 */
STATIC int getFrame( FILE *fp, drvSerialResponse *pResp, void *pAppPriv )
{
  drvSioLinkPrivate  	*pLink = (drvSioLinkPrivate *) pAppPriv;
  drvAppPrivate         *devInfo = NULL;
  uint8_t         	*pC;
  int             	c;
  int			status;
  int                   cmt = 1;
  int                   cmtIndex = 0;
  int                   firstGet = TRUE,
                        done = FALSE;

  /*
   * find the remote device info in order to access the command 
   * terminator. The first time through there is a timing issue 
   * so a delaying while loop is required.
   */
  devInfo = (drvAppPrivate *) ellFirst( &pLink->remoteDevList );

  while ( !devInfo ) {
    taskDelay( sysClkRateGet() );
    devInfo = (drvAppPrivate *) ellFirst( &pLink->remoteDevList );
  }

  if (devInfo->debugFlag) printf("getFrame: Enter\n");
  /* 
   * Parse out token until we see the command terminator
   */
  pC = pResp->buf;
  while ( pC < &pResp->buf[NELEMENTS(pResp->buf)] && cmt > 0 ) {
    c = getc( fp );
  
    if ( c == EOF ) {
      return EOF;
    }

    if ( firstGet ) {
      cmt = devInfo->recordCnt;

      if ( (cmt <= 0) && (strlen( devInfo->readCmt ) > 0) ) cmt = 1;

      firstGet = FALSE;
    }

    /*  
     *  strip null characters as these cause succeeding 
     *  scanf calls to fail
     */
    if ( (char)c == '\0' ) continue;

    /*
     *  If a command terminator exists then continue getting  
     *  characters until the entire terminator is received, 
     *  otherwise terminate on  the first white space char. 
     *  If the data is expected to span  multiple terminators 
     *  (ie. multiple record response) then wait  for the number 
     *  of lines given by 'recordCnt'.
     */
    if ( cmt > 0 ) {
      if ( (char)c == devInfo->readCmt[cmtIndex] )
	cmtIndex++;
      else
	cmtIndex = 0;
      if (devInfo->debugFlag) {
	  printf("[%02x]", c);
      }	
      if ( devInfo->readCmt[cmtIndex] == '\0' ) {
	cmt--;
	cmtIndex = 0;
	if (devInfo->debugFlag) printf("\n");
      }

      if ( cmt == 0 ) done = TRUE;
    }
    else if ( isspace( c ) ) { 
      break;  
    }  

    *(pC++) = (uint8_t) c;
    *pC = '\0';

    if ( done )
      break;
  }
  
  devInfo->recordCnt = 0;

  if (devInfo->debugFlag) {
    /*
     *  Do not use epicsPrintf as deadlock over a semaphore 
     *  could occur.
     */
    printf( "drvAscii(%d) <= ", pResp->bufCount);
    printString( pResp->bufCount, (char *)(pResp->buf) );
    printf( "\n");
  }

  /*
   *  message buffer over flow (ignore this frame)
   */
  if ( pC >= &pResp->buf[NELEMENTS(pResp->buf)] ) {
    logMsg( "%s (%d) : [%s] buf OVF\n", 
	    (int)__FILE__, (int)__LINE__,
	    (int)devInfo->pLink->fileName, 0,0,0 );
    return 0;
  }
     
  /*
   *  wake up send task 
   *  (don't allow further commands until we get response from query)
   */
  if (devInfo->debugFlag) printf("getFrame: semGive sendBlockSem\n");
  status = semGive( pLink->sendBlockSem );
  assert( status == OK );
  
  /*
   *  this tells the recv task to forward this frame
   */
  pResp->bufCount = pC - pResp->buf + 1;

  if (devInfo->debugFlag) printf("getFrame: Exit\n");
  return pResp->bufCount;
}

/*+*********************************************************************
  $Log: drvAscii.c,v $
  Revision 1.6  2001/04/10 14:51:02  peregrin
  Added more debug statements.

  Revision 1.5  2001/04/06 20:50:48  peregrin
  Fixed syncLock semaphore bug!!
  Problems with assert(semGive(..) == 0) construct.

  Revision 1.4  2001/02/01 22:00:30  peregrin
  v2_2_2 added some more diagnostics.

  Revision 1.3  2000/09/20 14:37:15  peregrin
  First checkin for v2_1_0.

  Revision 1.2  2000/06/23 01:54:13  peregrin
  First cut of v1_1_0.

  Revision 1.1  2000/04/20 01:37:13  peregrin
  End of April bright time devel.

  Revision 1.4  1999/09/02 01:24:03  kics
  Significant changes to alleviate miscomms between read and write
  tasks. This was instigated by changes to drvCmSx which suffered
  from similar problem (ie. this is derived from that).

  Revision 1.1  1998/12/03 23:56:11  ktsubota
  Initial insertion

  Revision 1.24  1998/03/18 01:39:22  ahoney
  Updated the processing for stringIn records to allows for dynamic
  prompts. This is accomplished by using the VAL field for output when
  prompting and input when a response arrives.

  Revision 1.22  1998/03/02 21:36:00  ahoney
  Modified extractIntData() and extractRealData() so that spaces bewteen
  the numberic sign and the first digit are changed to 0. This is so
  sscanf functions correctly.

  The need was the idiosyncracy of the Sony Gauges on the secondary
  The gauges provide independent secondary position feedback.

  Revision 1.21  1997/05/13 02:27:51  ahoney
  Removed commented out epicsPrintf statements that were replace with
  logMsg calls.

 * Revision 1.18  1997/05/06  01:00:34  ahoney
 * Mod to eliminate errorPrintf from with the framing routines, as there is
 * the possibilty that a link down condition could cause a serial link task
 * to be deleted when the semaphorer within errorPrintf is taken. Note that
 * errorPrintf was replaced with logMsg().
 *
 * Revision 1.17  1997/03/07  01:36:20  ahoney
 * Added the ability to enable 'debug' on a link-by-link basis. The
 * param field keyword is <debug> and should be associated with a
 * longout record.
 *
 * Revision 1.16  1997/02/19  01:37:20  ahoney
 * Reenabled NULL stripping within getFrame()
 *
 * Revision 1.14  1997/02/13  18:25:14  ahoney
 * Modified getFrame() so that <null> characters are ignored.
 *
 * Revision 1.13  1997/02/10  23:23:48  ahoney
 * Corrected a bug in putFrameAndCallback() - the device level callback on
 * error is now conditional as it was invalid to do so if no async completion
 * routine existed, as was true for output records.
 *
 * Revision 1.12  1997/02/08  00:51:50  ahoney
 * Removed returning EOF on failure within getFrame as this would cause
 * drvSerial to discard all queued commands and delete and restart its
 * tx and rx tasks.
 *
 * This will need to be changed in the future so that a database record
 * can be used to specify the behavior, as in some instances the user may
 * want to old behavior to occur.
 *
 * Revision 1.11  1997/01/23  02:18:10  ahoney
 * Added drvAsciiIoctl() to provide a mechanism to set ioctl options on
 * an underlying serial device driver.
 *
 * Also altered (corrected) extractBinaryData so that skipping characters
 * is possible, although one must use '%*[...]' rather than '%*c' as the
 * latter is not correctly handled by Vxworks implementation of sscanf.
 *
 * Revision 1.10  1997/01/21  02:37:59  ahoney
 * Mods to better handle conversion from integer to strings. This is
 * still unacceptable as drvAscii cannot handle null bytes as they
 * appear to be end-of-string terminators.
 *
 * Revision 1.9  1997/01/20  20:51:01  ahoney
 * Extensive mods to accomodate:
 *   -added mbbi and mbbo direct records;
 *   -added waveform records (long stringin);
 *   -added conversion of binary streams '0' and '1', with or without
 *    delimiters
 *   -added conversion from string to numeric 'abcd'->0x61626364->1633837924
 *   -added support for muliple line input strings
 *
 * Revision 1.8  1996/12/18  23:27:33  ahoney
 * Mods to support ao,bi,bo,mbbi,mbbo,longin,longout,stringin, and stringout
 * records. These were necessitated for use with IFSM and chopper.
 *
 * Revision 1.7  1996/09/13  21:38:49  ahoney
 * Added the port name to all error message epicsPrints.
 *
 * Revision 1.5  1996/09/13  20:49:53  ahoney
 * Mods to accomodate the flat lamp device.
 *
 * Note this driver was completed only so far as was necessary for the
 * data acquisition systems. The drive will still need a few mods for
 * the IFSM.
 *
 * Revision 1.4  1996/09/12  01:09:40  ahoney
 * Corrected an epicsPrint statement in putFrameAndCallBack().
 *
 * Revision 1.3  1996/09/11  21:51:52  ahoney
 * Mods to incorporate longin and longout records as needed for the
 * dome flat lamps. Also modified the handling of '%nk' in data formats
 * so that 'n' characters can be 'killed' at the beginning of a data
 * stream - this allows stripping leading NULLs,...
 *
 * Revision 1.2  1996/07/13  00:38:05  ahoney
 * Removed some debugging printf's.
 *
 * Revision 1.1  1996/07/12  23:26:24  ahoney
 * drvAscii is a new directory for support for serial comms to remote
 * devices via ascii strings
 *

 *
***********************************************************************/
 

