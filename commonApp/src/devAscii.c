static char rcsid[] = "$Id: devAscii.c,v 1.2 2001/01/30 22:48:32 peregrin Exp $"; 
/*
 * Author: Allan Honey (Keck observatory) with considerable collaboration 
 *         by W.Lupton.
 *
 *         Derived from the Compumotor SX device support (devCmSx.c)
 *         written by Jeff Hill.
 *         
 */

/*
 * devAscii provides the interface between records, whose DTYP='Ascii SIO',
 * and drvAscii. The records supported are ai, ao, bi, bo, mbbi, mbbi direct,
 * mbbo, mbbo direct, longin, longout, stringin, stringout, and waveform.
 * Where the waveform record is used as a long stringin record such that
 * strings greater than MAX_STRING_SIZE (40) can be handled. 
 *
 * The basic design is to pass record specific data to drvAscii, which 
 * formats a request for drvSerial which eventually asynchronously calls 
 * back devAscii with the response.
 *
 * During record init the PARM string (record's INP or OUT field) is parsed
 * and the information is cached and passed to drvAscii. drvAscii uses the 
 * information to determine how to handle i/o for the record.
 *
 * The format of the PARM string is: 
 *    serial port name {[signal number | array size] @}
                       {<special record> || 
 *                      [<command prompt><response format>]
 *                     {<readback prompt><readback response format>}}
 *
 *    where:
 *    signal number = Sn @     - not implemented
 *    array number  = An @     - not implemented
 *
 *    special_record 
 *    = [ slope || 
 *        timeout || 
 *        writeCmt %s || 
 *        readCmt %s || 
 *        connect || 
 *        connectSts || 
 *        debug ]
 *
 *    note that the %s for the write and read command terminators are 
 *    necessary!
 *
 *    Note that all records must have a serial port name. As of Jan 97 the
 *    driver was not tested with file streams but in principle only minor
 *    changes should needed to be made to make it work. All records
 *    except a stringout record must have a command prompt.
 *
 * The 'special_records' control various aspects of how drvAscii functions.
 * Note that a special record affects the i/o of all records associated
 * with a serial link. A description of each 'special' record follows.
 *
 * 'slope' is used to manipulate analog i/o values. Slope is needed because: 
 * the RVAL for an analog input record is an integer and there is no way to 
 * circumvent conversion from RVAL to VAL (probably would not want to anyways)
 * so ESLO, ASLO and ROFF come into play; the RVAL from an analog output 
 * record is an integer and if one used VAL then the ESLO, ASLO and ROFF 
 * would be circumvented. Thus all analog output values (rval) are divided
 * by slope prior to formatting the output string, and all analog input values
 * are multiplied by slope (eg. a remote device which returns values with a
 * precision to the second decimal place, such as 2.34, may have a slope of
 * 100 such that the associated AI rval would be 234 -> 0xea, the record's
 * EGUL and EGUF could be set so that the original value is restored or
 * converted directly into some other unit). 
 *
 * The 'slope' record should be associated with an AO record.
 *
 * 'timeout' allows one to control the maximum time that drvAscii will wait
 * for a response to a command/prompt. 'timeout' should be associated with
 * a longout record. (future mod is to allow this to be a float so fractions
 * of seconds can be specified).
 *
 * 'writeCmt %s' and 'readCmt %s' allows one to change the write and read 
 * command terminators. The writeCmt is tagged on to the end of the output 
 * string (prompts), the default is 'crlf'. The readCmt defines the end of 
 * a response string, default is 'crlf'. The associated records must be 
 * stringout records. The command terminator strings will be parsed using C 
 * character and numeric escape codes - be aware that dbpf and caput do not 
 * handle escape codes identically! Also converting from a capfast drawing
 * to a database strips the '/' in '/n' so one cannot currently specify the
 * usual command terminators as a constant to a capfast record.
 *
 * 'connect' allows one to change the connection state of a serial link.
 * Upon successful initialization the connection state will be on (1).
 * When the connection state is off (0) then drvAscii will not perform any
 * i/o. 
 *
 * 'connectSts' returns the current connection state of a serial link
 * where 1 means on or ok and 0 means off or disabled.
 *
 *
 *      PROMPT AND RESPONSE FORMAT STRINGS (extensions to scanf syntax)
 *
 * KILL SYNTAX
 * In order to make drvAscii as flexible as possible, with respect to remotely
 * connected devices, (at least as far as my imagination could fathom) the
 * format strings were extended beyond the usual '%' C specifications.
 *
 * drvAscii makes extensive use of sscanf, sprintf, strlen, ...
 * thus certain values embedded in return strings could cause problems. One
 * of the first uses of drvAscii, at Keck, was interfacing to the shutter
 * clinometers.  Unfortunately, the underlying device preceeds its response 
 * with a NULL byte ('\0'). Which of course is going to make string processing 
 * very cumbersome (the string appears to be a null string).
 *
 * To provide a mechanism for 'ignoring' a leading null byte or, for that
 * matter, any series of leading bytes, a kill syntax was implemented 
 * ('%nk' or '%*k'). This essentially means skip the first 'n' characters 
 * in a response, '%*k' means ignore the entire response. If the kill syntax 
 * is used it must be the first characters in the response format string and 
 * only one such specification is permitted. The kill syntax is not permitted
 * in a prompt string as it would be meaningless.
 *
 *
 * MULTILINE INPUT SYNTAX
 * At Keck the infra-red choppers have status request commands which generate
 * multi-line responses. To handle this type of response it is necessary to
 * circumvent the normal read command terminator mechanism (ie. the low level
 * framing routine normally waits for a single command terminator sequence).
 * This was accomplished by the command terminator count syntax '%nt'. This
 * causes the framing to wait for 'n' command terminator sequences. 
 *
 * The command terminator count format string must be either at the beginning 
 * of the response format string or immediately following the kill string.
 *
 * 
 * BINARY CONVERSIONS
 * At Keck the secondary actuators are driven by Compumotor motor controllers,
 * to which the interface is via serial i/o. Several of the commands to these
 * controllers return statuses as a string of '0' and '1' such as '1011001101'
 * or '1110_0110_1111'. It is necessary to convert these strings to integer
 * values before returning them to the Epics records, which cannot be done
 * with standard C scanf syntax.
 *
 * drvAscii augments the standard C scanf syntax with '%nb %[abc...]'. This
 * syntax indicates that the incoming string is a sequence of '1' and '0' 
 * which must be converted to an integer value (eg. '101101' -> 0x2d -> 45).
 * The optional '%[abc...]' syntax allows one to specify a list of delimiter 
 * characters. If the delimiter string exists then it must immediately follow
 * the '%nb' string. Also note that '%*b' is not valid, as the same can be
 * accomplished with '%*d' or '%*f'.
 *
 * 
 * STRING TO NUMERIC
 * At, Keck we have some devices, namely the Compumotor motor controllers, 
 * which return statuses as ascii characters (why they did this with some
 * statuses and binary strings '0' and '1' for other statuses and did not
 * allow one to select a mode is a mystery). In any event, drvAscii allows
 * '%nc' or '%ns' to be specified for numerica data. Note that 'n' may not
 * be larger than 4.  
 *
 * This syntax will cause the incoming string to be converted to an integer, 
 * and hence to a float for analog inputs. For instance the string "abcd" 
 * would be converted to the integer value 1633837924 which is 0x61626364.
 * Note that the first character of the incoming string will be the high
 * byte of the resultant integer value.
 *
 * Note that this implementation is not complete and actually unacceptable
 * with converting string<->numeric. The problem is that string functions
 * are extensively used and, therefore, a 0 is converted to a null byte, 
 * causing unexpected results. I suggest that string->numeric be used for
 * single character values and numeric->string not be used at all! A long
 * term plan is to correct this but it requires extensive mods that I am not
 * currently (1/97) able to devote the time to.

 * LONG STRING INPUTS
 * At Keck we had a need to allow inputting of strings longer than the 
 * maximum string allowed for a stringin record (40 bytes). To accommodate
 * this, device support for the waveform record was implemented. This allows
 * strings upto DRV_SERIAL_BUF_SIZE (currently defined in drvSerial.h as
 * 0x400 -> 1024). 
 *
 * Note that the user is assumed to have connected the waveform record to a 
 * subroutine record (or equivalent) so as to parse the incoming string as 
 * desired.
 *
 *  
 * PARM STRING EXAMPLES 
 *    @/tyCo/1 <timeout>
 *    this would be a record, associated with the link on '/tyCo/1' (which 
 *    is the vxworks stream associated with an on board serial port), that
 *    allows control of the i/o timeout for asynchronous i/o.
 *
 *    @/tyCo/1
 *    this would have to be a stringout record as they are the only records 
 *    for which a command/prompt is not necessary.
 *
 *    @/tyCo/1 <CHN1><001,%f>
 *    this would be an AI record for which a remote device would respond
 *    to the prompt 'CHN1' with '001,' followed by a floating point value.
 *    Note that the angle brackets are necessary delimiters however they 
 *    are stripped off and never seen by drvAscii. At this time, Jan 97,
 *    there is no provision for an escape code for angle brackets.
 *
 *    @/tyCo/1 <CHN1><%4k%f>
 *    same as the previous example except the '001,' will be ignored (ie.
 *    pattern matching will not occur). Note that all characters in an 
 *    input stream must be consumed or the stream is rejected. So if a
 *    remote device returns a string of characters before and/or after 
 *    a numeric string then those strings must be specified in the response
 *    format string. If the before or after strings can vary then it is
 *    necessary to input to a stringin or waveform record and process the
 *    stream with a subroutine.
 *
 *    @/tyCo/1 <CHN1><%*k>
 *    this is invalid as an input record cannot ignore all characters.
 * 
 * A word of warning! Be very careful how one uses '%nc' as '%c' does
 * not function that same as '%1c' and '%*c', with Vxworks implementation
 * of sscanf. In actual fact '%1c' and '%*c' function as though they were
 * '%1s'. I also believe that '%[^...]' is incorrect. 

 * If one has the following string as input "1RA\r*@\r" then only the 
 * following response  format string will succeed "%2t1RA%*[*]%s". These 
 * will not "%2t1RA%*c%s", "%2t1RA%*[^*]%s". The problem is that leading 
 * whitespace is incorrectly handled by sscanf.
 *
 *   
 * PROCESSING
 * When an Ascii SIO record processes, the devAscii i/o function passes 
 * drvAscii a pointer to an asynchronous IO structure and a pointer to an 
 * output value. drvAscii formats an ascii string, as per the format 
 * registered on record init as specified in the record's parm string 
 * (INP or OUT field), setups up a callback routine, queues the string 
 * plus port specific info, to the appropriate transmit task in drvSerial,
 * and returns a status value to devAscii.

 * If a response is expected then the status is set to
 * S_drvAscii_AsyncCompletion (only occurs for input records) causing devAscii 
 * to mark the record's PACT=true. If and when the remote device responds the 
 * response string is parsed, as per the specified response string format and, 
 * in the case of input records, the devAscii asynchronous callback routine
 * is called. 
 *
 * If the write/read cycle fails then the asynchronous callback routine is
 * passed an error status such that devAscii sets the records alarm state.
 *
 * The async callback routine sets the records rval (val for stringin) then 
 * causes the record to be processed again, which ultimately will clear the 
 * PACT.
 * 
 */
 
/*
 * ANSI C
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<limits.h>

/*
 * EPICS
 */
#include	<epicsAssert.h>
#include	<alarm.h>
#include	<cvtTable.h>
#include	<dbDefs.h>
#include	<dbAccess.h>
#include	<dbFldTypes.h>
#include        <recSup.h>
#include	<devSup.h>
#include	<dbScan.h>
#include	<link.h>
#include	<aiRecord.h>
#include	<aoRecord.h>
#include	<biRecord.h>
#include	<boRecord.h>
#include	<mbbiRecord.h>
#include        <mbbiDirectRecord.h>
#include        <mbboRecord.h>
#include        <mbboDirectRecord.h>
#include        <stringinRecord.h>
#include        <stringoutRecord.h>
#include        <longinRecord.h>
#include        <longoutRecord.h>
#include	<waveformRecord.h>
#include	<devLib.h>

/*
 * Ascii driver
 */
#include   <drvSerial.h>
#include   <drvAscii.h>

#define S_devAscii_OK            0
#define S_devAscii_dontConvert   2
#define S_devAscii_badRec        3
 
#define devInitPassBeforeDevInitRec 0
#define devInitPassAfterDevInitRec 1

/* 
 * The following structure is created for each record.
 * The resulting structure is pointed to by the record's dpvt field. This 
 * is essentially the handle used by the driver (drvAscii.c) for processing 
 * the request.
 */ 
typedef struct ascii_dev_priv { 
  void        *pRec;  /* record pointer */
  long        sigNum_arraySize; /* NOT YET IMPLEMENTED 970106 */
  IOSCANPVT   spvt;   /* support interrupt from this parameter    */
  drvAsyncIO  aio;    /* async io structure. included in this structure is:*/
                      /* id - (drvSioLinkId) drvSerial structure for tx/rx */
		      /* pCB - the device callback routine for async       */
                      /* completion pArg - (arg for pCB) which is a pointer*/
                      /* to the record (*pAppDrvPrivate) - drvAscii        */
                      /* callback function used for proccessing responses  */
                      /* to output commands                                */
}devAsciiPriv; 
 
LOCAL drvAsyncUpdateCallBack devAsciiUpdate;

/*
 *      DEVICE ENTRY TABLES
 */
LOCAL long devAsciiInit();

typedef struct {
  long		number;
  DEVSUPFUN	report;
  DEVSUPFUN	init;
  DEVSUPFUN	init_record;
  DEVSUPFUN	get_ioint_info;
  DEVSUPFUN	read_write;
} INTEGERDSET;

typedef struct {
  long		number;
  DEVSUPFUN	report;
  DEVSUPFUN	init;
  DEVSUPFUN	init_record;
  DEVSUPFUN	get_ioint_info;
  DEVSUPFUN	read_write;
  DEVSUPFUN	special_linconv;
} FLOATDSET;

/*
 * Device entry table for AI records
 */
LOCAL long aiInitRec();
LOCAL long aiGetIoIntInfo();
LOCAL long aiRead();
LOCAL long aiLinearConvert();
LOCAL devIoDoneCallBack aiReadAsyncCompletion;

FLOATDSET devAiAscii = {
  6,
  NULL,
  devAsciiInit,
  aiInitRec,
  aiGetIoIntInfo,
  aiRead,
  aiLinearConvert
};

/*
 * Device entry table for AO records
 */
LOCAL long aoInitRec();
LOCAL long aoWrite();
LOCAL long aoLinearConvert();
FLOATDSET devAoAscii = {
  6,
  NULL,
  devAsciiInit,
  aoInitRec,
  NULL,
  aoWrite,
  aoLinearConvert
};

/*
 * Device entry table for BI records
 */

LOCAL long biInitRec();
LOCAL long biGetIoIntInfo();
LOCAL long biRead();
LOCAL devIoDoneCallBack biReadAsyncCompletion;
INTEGERDSET devBiAscii = { 
  5,
  NULL,
  devAsciiInit,
  biInitRec,
  biGetIoIntInfo,
  biRead
};

/*
 * Device entry table for BO records 
 */

LOCAL long boInitRec();
LOCAL long boWrite();
INTEGERDSET devBoAscii = { 
  5,
  NULL,
  devAsciiInit,
  boInitRec,
  NULL,
  boWrite
};

/*
 * Device entry table for MBBI records
 */

LOCAL long mbbiInitRec();
LOCAL long mbbiGetIoIntInfo();
LOCAL long mbbiRead();
LOCAL devIoDoneCallBack mbbiReadAsyncCompletion;
INTEGERDSET devMbbiAscii = {
  5,
  NULL,
  devAsciiInit,
  mbbiInitRec,
  mbbiGetIoIntInfo,
  mbbiRead
};

/*
 * Device entry table for MBBO records
 */

LOCAL long mbboInitRec();
LOCAL long mbboWrite();
INTEGERDSET  devMbboAscii = { 
  5,
  NULL,
  devAsciiInit,
  mbboInitRec,
  NULL,
  mbboWrite
};

/*
 * Device entry table for MBBI direct records
 */
LOCAL long mbbiDirectInitRec();
LOCAL long mbbiDirectGetIoIntInfo();
LOCAL long mbbiDirectRead();
LOCAL devIoDoneCallBack mbbiDirectReadAsyncCompletion;
INTEGERDSET devMbbiDirectAscii = {
  5,
  NULL,
  devAsciiInit,
  mbbiDirectInitRec,
  mbbiDirectGetIoIntInfo,
  mbbiDirectRead
};

/*
 * Device entry table for MBBO direct records
 */

LOCAL long mbboDirectInitRec();
LOCAL long mbboDirectWrite();
INTEGERDSET  devMbboDirectAscii = {
  5,
  NULL,
  devAsciiInit,
  mbboDirectInitRec,
  NULL,
  mbboDirectWrite
};

/*
 * Device entry table for stringin records
 */
LOCAL long siInitRec();
LOCAL long siGetIoIntInfo();
LOCAL long siRead();
LOCAL devIoDoneCallBack siReadAsyncCompletion;
INTEGERDSET  devSiAscii = {
  5,
  NULL,
  devAsciiInit,
  siInitRec,
  siGetIoIntInfo,
  siRead
};

/*
 * Device entry table for stringout records
 */
LOCAL long soInitRec();
LOCAL long soWrite();
INTEGERDSET  devSoAscii = {
  5,
  NULL,
  devAsciiInit,
  soInitRec,
  NULL,
  soWrite
};

/*
 * Device entry table for longin records
 */
LOCAL long liInitRec();
LOCAL long liGetIoIntInfo();
LOCAL long liRead();
LOCAL devIoDoneCallBack liReadAsyncCompletion;
INTEGERDSET devLiAscii = {
  5,
  NULL,
  devAsciiInit,
  liInitRec,
  liGetIoIntInfo,
  liRead
};

/*
 * Device entry table for longout records
 */
LOCAL long loInitRec();
LOCAL long loWrite();
INTEGERDSET devLoAscii = {
  5,
  NULL,
  devAsciiInit,
  loInitRec,
  NULL,
  loWrite
};


/*
 * Device entry table for waveform (long stringin) records
 */
LOCAL long wfInitRec();
LOCAL long wfGetIoIntInfo();
LOCAL long wfRead();
LOCAL devIoDoneCallBack wfReadAsyncCompletion;
INTEGERDSET  devWfAscii = {
  5,
  NULL,
  devAsciiInit,
  wfInitRec,
  wfGetIoIntInfo,
  wfRead
};


/*
 * ---------------------------------------------------------------------------
 * copyString - copys one string to another upto a maximum length
 *     or a specified character. This is used by parseAsciiAddress.
 */
LOCAL void copyString ( char *pStr,
			char eos,
			char *pDest,
			long pDestLength )
{
  long index = 0;

  while ( index < pDestLength 
	  && 
	  pStr[index] != eos ) {
    pDest[index] = pStr[index];
    index++;
  }
  
  pDest[index] = '\0';
}


/*
 * ---------------------------------------------------------------------------
 * getChannel - returns the 'channel' which is the value you for 'S' in
 *   the usual '#Cn Sm @', albeit '#Cn' is not relevant. Note that 'Am' 
 *   can be in place of 'Sm' but is not yet supported (as of 970117) and
 *   is intended for arrays of values.
 */
LOCAL long getChannel( char *bfr ) 
{
  char *pStart;
  long number = 0;

  pStart = bfr;

  while ( pStart 
	  && 
	  (*pStart != 'S' && *pStart != 'A') ) 
    pStart--; 

  if ( !pStart ) {
	    number = -1;	
  }	
  else if ( *(pStart-1) != ' ' ) {
    number = -1;
  } 
  else {
    pStart++;
    sscanf( bfr, "%ld", &number );
  }

  return number;
}


/*
 * ---------------------------------------------------------------------------
 * parseAsciiAddress() - parses a record's parm field into:
 *     a 'filename' (port designation passed to drvAscii
 *     a command/prompt string or special command name
 *     a command response format string
 *     a readback prompt string (output value's initial value request ) 
 *     a readback response format string
 *
 *     Note that only the 'filename' must exist for all record types.
 *     The type of record determines whether or not the command/prompt 
 *     and command response format strings are required. The readback
 *     prompt and response format strings are always optional.
 * 
 *     The resultant strings are copied into the appropriate char array
 *     within 'drvCmndArg *pCmndArgs'
 *
 */
LOCAL long parseAsciiAddress (const char *dtype,
			      const char *pAddr,
			      char       *pFileName,
			      unsigned   maxFileName,
			      drvCmndArg *pCmndArgs,
			      long       *sigNum_arraySize)
{
  int  status;
  long epicsStatus = S_devAscii_OK;
  char format[32];
  char *pStart = NULL,
       *pEnd = NULL;
      
  /* 
   * ensure that all strings are null terminated. This is protection
   * against an undefined string
   */
  pCmndArgs->cmndPrompt[0] =
  pCmndArgs->cmndFormat[0] =
  pCmndArgs->rbvPrompt[0] =
  pCmndArgs->rbvFormat[0] = '\0';
  
  /*
   * parse the 'file' name (ie. the serial link identification string).
   * If it doesn't exist then abort.
   */
  assert( maxFileName>=1 );
  assert( pFileName );
 
  sprintf( format, "%%%ds",maxFileName-1 );
  status = sscanf( pAddr, format, pFileName );

  if ( status < 1 ) {
    epicsStatus = S_drvAscii_badParam;
  }
  else {
    /* 
     * determine the signal (channel) number or array size. This should only 
     * exists for data streams which return multiple values such that the 
     * relevant value can be return (or array of values for analog or integer 
     * input streams).
     *
     * If a signal (channel) number or array size is not specified then
     * only one value is expected, so the first data value will be returned.
     * Note this number is important for binary or multiple binary i/o where
     * the stream is a series of '0' and '1'.
     * 
     * A signal number is specified by 'Sn @' where 'n' is the number.
     *
     * An array size is specified by 'An @' where 'n' is the number of 
     * elements.
     */

    /* locate the first command/prompt string delimiter */
    pStart = strchr( pAddr, '<' );

    /* attempt to locate the signal number/array size delimiter */
    pEnd = strchr( pAddr, '@' );     

    if ( pEnd && !(pEnd > pStart) ) { 
      /* 
       * the @ is not embedded in a command/prompt so assume it delimits a 
       * signal number or array size.
       *
       * NOTE THIS ALL FAILS IF @ IS PART OF THE SERIAL PORT NAME!!!
       */
      *sigNum_arraySize = getChannel( pEnd );

      if ( *sigNum_arraySize < 0 ) {
	*sigNum_arraySize = 0;
	epicsStatus = S_drvAscii_badParam;
      }
    }
 
    /*
     * locate the command/prompt string. If it doesn't exist and the record  
     * is not a string record then abort (ie. all records but string records
     * require a command/prompt string).
     */
    if ( !epicsStatus && pStart ) {
      pEnd = strchr( pAddr, '>' );
      
      if ( !pEnd ) {
	epicsStatus = S_drvAscii_badParam;
      }         
      else {
	copyString( ++pStart,'>',
		     pCmndArgs->cmndPrompt,
		     min( (int)((int)pEnd - (int)pStart),
			  sizeof( pCmndArgs->cmndPrompt )-1 ) ); 
      }
    }
      
    /* find any command response format string */
    if ( !epicsStatus ) {
      pStart = strchr( ++pEnd,'<' );
       
      if ( pStart ) {
	pEnd = strchr (pStart,'>' );
	if ( !pEnd ) {
	    epicsStatus = S_drvAscii_badParam;
	  }         
	else
	  copyString( ++pStart,'>',
		       pCmndArgs->cmndFormat,
		       min( (int)((int)pEnd - (int)pStart),
			    sizeof( pCmndArgs->cmndFormat )-1 ) );
      }
    }
    
    /* 
     * find any readback prompt string. This would typically exist for
     * an output for which an initial value can be obtained.
     */
    if ( !epicsStatus ) {
      pStart = strchr( ++pEnd,'<' );
      
      if (pStart) {
	pEnd = strchr( pStart,'>' );
	if ( !pEnd ) {
	  epicsStatus = S_drvAscii_badParam;
	}         
	else {
	  copyString( ++pStart,'>',
		       pCmndArgs->rbvPrompt,
		       min( (int)((int)pEnd - (int)pStart),
			    sizeof( pCmndArgs->rbvPrompt )-1 ) );
	}
      }
    }
   
    /* 
     * find any readback response format string. This would typically
     * exist for formatting a response for a request for the initial
     * value for an output record
     */
    if ( !epicsStatus ) {
      pStart = strchr( ++pEnd,'<' );
      
      if (pStart) {
	pEnd = strchr( pStart,'>' );
	if (!pEnd) {
	  epicsStatus = S_drvAscii_badParam;
	}         
	else {
	  copyString( ++pStart,'>',
		       pCmndArgs->rbvFormat,
		       min( (int)((int)pEnd - (int)pStart),
			    sizeof( pCmndArgs->rbvFormat )-1 ) );
	}
      }
    }
  }
  
  if ( epicsStatus ) {
    errPrintf ( status, __FILE__, __LINE__, "Supplied was \"%s\"", pAddr );
    errPrintf ( status, __FILE__, __LINE__,
	       "Expected was \"<device (serial port) name> <cmnd prompt> {<cmnd format>{<rbv prompt>{<rbv format}}}\"" );
  }
  return epicsStatus;
}


/*
 * ---------------------------------------------------------------------------
 * devAsciiInitPrivate() - this function validates a record's parm field
 *     and establishes the drvAscii info required for processing the record.
 *     This includes the callback info required for asynchronous completion. 
 */
LOCAL long devAsciiInitPrivate( const char   *dtype, 
				struct link  *pLink, 
				void         *pRec, 
				devAsciiPriv **ppPriv,
				long         *sigNum_arraySize )
{	
  int	       status;
  char	       fileName[sizeof(pLink->value)];
  drvCmndArg   *pCmndArgs;
  devAsciiPriv *pPriv = NULL;

  /* if the record does not have INST_IO defined then abort */
  if ( pLink->type != INST_IO ) {
    status = S_db_badField;
    recGblRecordError ( status, pRec, 
		       __FILE__": Address type must be type \"instrument\"" );
    return status;
  }
 
  /* 
   * allocate the structure which will ultimately be pointed to by the 
   * record's dpvt field 
   */
  pPriv = calloc( 1, sizeof( *pPriv ) );
  if ( pPriv == NULL ) {
    /* 
     * this error may be catastrophic as pPriv being NULL is not 
     * appropriately captured
     */
    *ppPriv = NULL;
    status = S_dev_noMemory;
    recGblRecordError ( status, pRec, 
			__FILE__": no room for device private" );
    return status;
  }

  /* init the scan private info */
  scanIoInit( &pPriv->spvt );

  /* ensure that the record's dpvt pointer is set */
  *ppPriv = pPriv;

  /* allocate space for the strings embedded in the parm field */
  pCmndArgs = calloc( 1,sizeof( drvCmndArg ) );
  if ( !pCmndArgs ) {
    status = S_dev_noMemory;
    recGblRecordError ( status, pRec, 
		       __FILE__": no room for command args" );
    return status;
  }

  /* parse the parm field */
  status = parseAsciiAddress( dtype,
			      pLink->value.instio.string,
			      fileName,
			      sizeof(fileName),
			      pCmndArgs,
			      sigNum_arraySize );
  if ( status ) {
    /* the record's parm field is invalid! */
    free( pCmndArgs );
    recGblRecordError (status, pRec, 
		       __FILE__": Syntax error in type \"instrument\" address");
    return status;
  }

  /*
   * Setup the drvAscii info and ensure a comms link is established with
   * drvSerial
   */
  status = drvAsciiCreateSioLink( dtype,  
				  fileName, 
				  pCmndArgs, 
				  &pPriv->aio.id, 
				  devAsciiUpdate, 
				  pPriv   );
  if ( status ) {
    /* the link was not successfully established */  
    recGblRecordError ( status, pRec, 
		       __FILE__": failed to create serial link" );
    return status;
  } 

  return S_devAscii_OK;
} 


/*
 * ---------------------------------------------------------------------------
 * devAsciiUpdate() 
 */
LOCAL void devAsciiUpdate(void *pArg, 
			  long status, 
			  long value)
{
  devAsciiPriv 	*pPriv = (devAsciiPriv *)pArg;
  
  scanIoRequest( pPriv->spvt );
}

/*
 * ---------------------------------------------------------------------------
 * devAsciiInit() 
 */
LOCAL long devAsciiInit(unsigned pass)
{
  long	status;
  
  switch ( pass ) {
  case devInitPassBeforeDevInitRec: 
    status = S_devAscii_OK;
    break;
    
  case devInitPassAfterDevInitRec: 
    /*
     * initiate scan on all links 
     */
    status = drvAsciiInitiateAll();
    break;
    
  default: /* not expected */
    status = S_devAscii_OK;
    break;
  }
  
  return status;
}


/*
 * ---------------------------------------------------------------------------
 * aiInitRec() - initializes an AI record. This setups up all the drvAscii
 *     info, drvSerial info, and asynchronous completion callback info,
 *     all of which is attached to the record's device private field.
 */     
LOCAL long aiInitRec (struct aiRecord *pAi) 
{
  long           status;
  devAsciiPriv   *pPriv = NULL;

  /* initialize the record and driver */
  status = devAsciiInitPrivate( "REAL_IN", 
				&pAi->inp, pAi, &pPriv, 
				&(pPriv->sigNum_arraySize) );

  /*
   * The prompt must exist but it cannot have a data type specification.
   *
   * The response format can be missing, in which case '%f' is assumed.
   *
   * The response format may be string ('%nc' or '%ns') but 'n' must not
   * be greater than 4. With a string specification the input ascii stream
   * will be converted to an integer value (eg. "abcd" -> 0x61626364 ->
   * 1633837924). 
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pAi,
		       "devAscii: (Ai init_record) Illegal format spec");

    pAi->dpvt = (void *) NULL;

    pAi->udf  = TRUE;
  }
  else {
    pAi->udf  = FALSE;

    pAi->dpvt = (void *) pPriv;

    /* set linear conversion slope*/
    pAi->eslo = pAi->eguf - pAi->egul;

    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB  = aiReadAsyncCompletion;
    pPriv->aio.pArg = pAi;
  }

  return status;
}
int DEBUG_ASCII = 0;
/*
 * ---------------------------------------------------------------------------
 * aiRead() - analog input routine
 */
LOCAL long aiRead (struct aiRecord * pAi)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pAi->dpvt;
  long		status;
  long       	rval;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pAi->pact == TRUE ) return S_devAscii_OK;

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pAi->pact = TRUE;
    pAi->udf  = TRUE;

    return S_devAscii_badRec; 
  }
  
  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo( &pPriv->aio, &rval );

  if ( status == S_drvAscii_AsyncCompletion ) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pAi->pact = TRUE; 

    status = S_devAscii_OK;
  }
  else if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pAi->pact = TRUE;
      pAi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pAi,
			 "devAscii: Illegal format spec" );

      return( S_db_badField );
    }
    else {
      /* read failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pAi, READ_ALARM, MAJOR_ALARM );
      if (DEBUG_ASCII)
	epicsPrintf("devAscii:aiRead: status=%lx\n", status);
    }	
  }
  else {
    /* read completed non-asynchronously */
    pAi->rval = rval;
  }

  return status;
}

/* 
 * ---------------------------------------------------------------------------
 *  aiReadAsyncCompletion() - Asynchronous read completion routine for AI
 */
LOCAL void aiReadAsyncCompletion(void *pArg, long status, long value)
{
  struct aiRecord      *pAi = (struct aiRecord *)pArg;
  struct dbCommon      *pRec = (struct dbCommon *)pArg;
  struct rset          *prset = (struct rset *)(pRec->rset);
  int                   dontProcess = FALSE;

  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pAi->pact = TRUE;
      pAi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pAi,
			 "devAscii: Illegal format spec" );
      dontProcess = TRUE; 
    }
    else {
      /* if the read failed then set the record into alarm */
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
      if (DEBUG_ASCII)
	epicsPrintf("devAscii:aiReadAsyncCompletion: status=%lx\n", status);
    }
  }
  else {
    /* copy the value into the record */
    pAi->rval = value;
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}

/*
 * ---------------------------------------------------------------------------
 * aiLinearConvert() - provides for eslo calc when eguf or egul
 *     are modified.
 */
LOCAL long aiLinearConvert (struct aiRecord *pAi, int after)
{
  if( !after ) {
    return( S_devAscii_OK );
  }
  
  /* set linear conversion slope*/
  pAi->eslo = pAi->eguf - pAi->egul;
  
  return( S_devAscii_OK );
}

/*
 * ---------------------------------------------------------------------------
 * aiGetIoIntInfo() - Used for obtaining the scan private info
 */
LOCAL long aiGetIoIntInfo(int             cmd,
			  struct aiRecord *pAi,
			  IOSCANPVT       *ppvt)
{
  devAsciiPriv *pPriv = (devAsciiPriv *) pAi->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}


/*
 * ---------------------------------------------------------------------------
 * aoInitRec() - initializes AO records. This setups up all the drvAscii
 *     and drvSerial info, all of which is attached to the record's device 
 *     private field.
 */
LOCAL long aoInitRec (struct aoRecord *pAo) 
{
  long          status;
  devAsciiPriv	*pPriv = NULL;
  long		rval;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "REAL_OUT", 
				&pAo->out, pAo, &pPriv,
				&(pPriv->sigNum_arraySize) );

  /*
   * A prompt must exist. However a data type specification need not exist,
   * in which case '%f' is assumed.
   *
   * Responses must not cause assignment (ie. all reponse format strings must 
   * be of the form '%*' ). 
   *
   * Output formats of '%nc' and '%ns' are valid but 'n' must not be greater
   * than 4. That is, a 4 byte integer will be converted to an output string 
   * of 4 ascii chars (eg. 1633837924 -> 0x61626364 -> "abcd"). Note that the
   * resultant ascii chars may not be printable ascii chars 
   * (eg 23200612 -> 0x01620364 -> ^Ab^Cd, where ^A is control A -> 0x01).
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0)  
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pAo,
		       "devAscii: (Ao init_record) Illegal format spec");

    pAo->dpvt = (void *) NULL;

    pAo->udf  = TRUE;
  }
  else {
    pAo->udf  = FALSE;

    pAo->dpvt = pPriv;

    /* 
     * set linear conversion slope
     */
    pAo->eslo = pAo->eguf - pAo->egul;
    
    if ( status == OK
	 && 
	 strlen( pPriv->aio.id.pCmndArg->rbvPrompt ) > 0 )
      /* if possible get an initial value for the record */
      if ( drvAsciiReadOutput( &pPriv->aio.id, &rval ) != OK 
	   || 
	   pAo->pini ) {
	status = S_devAscii_dontConvert;
      }
      else {
	/* set the initial value */
	pAo->rval = (long) rval;
      }
    else status = S_devAscii_dontConvert;
  }

  return status; 
}

/*
 * ---------------------------------------------------------------------------
 * aoWrite() - analog output routine
 */
LOCAL long aoWrite (struct aoRecord * pAo)
{
  devAsciiPriv 	*pPriv = (devAsciiPriv *)pAo->dpvt;
  long		status;
  long          rval;

  /* 
   * if a write is already in progress, or the record is locked-out,
   * then exit 
   */
  if ( pAo->pact == TRUE ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pAo->pact = TRUE;
    pAo->udf  = TRUE;

    return S_devAscii_badRec; 
  }
  
  /* Perform the write */
  rval = pAo->rval;

  status = drvAsciiIntIo( &pPriv->aio, &rval );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pAo->pact = TRUE;
      pAo->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pAo,
			 "devAscii: Illegal data format" );

      return( S_db_badField );
    }
    else
      /* write failed */
      recGblSetSevr( (struct dbCommon *)pAo, WRITE_ALARM, MAJOR_ALARM );
  }

  return( status );
}

/*
 * ---------------------------------------------------------------------------
 * aoLinearConvert() - provides for eslo calc when eguf or egul are modified.
 */
LOCAL long aoLinearConvert (struct aoRecord *pAo, int after)
{
  if( !after ) return( S_devAscii_OK );

  /* set linear conversion slope*/
  pAo->eslo = pAo->eguf - pAo->egul;

  return( S_devAscii_OK );
}


/*
 * ---------------------------------------------------------------------------
 * biInitRec() - initializes BI records. This setups up all the drvAscii
 *     info, drvSerial info, and asynchronous completion callback info, 
 *     all of which is attached to the record's device private field.
 */
LOCAL long biInitRec(struct biRecord *pBi) 
{
  long         status;
  devAsciiPriv *pPriv= NULL;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_IN", 
				&pBi->inp, pBi, &pPriv, 
				&(pPriv->sigNum_arraySize) );
  
  /*
   * The prompt must exist but it cannot have a data type specification.
   *
   * The response format can be missing, in which case '%f' is assumed.
   *
   * The response format may be string ('%nc' or '%ns') but 'n' must not
   * be greater than 4. With a string specification the input ascii stream
   * will be converted to an integer value (eg. "abcd" -> 0x61626364 ->
   * 1633837924). 
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndInfo.dataCnt > 4) ) 
       ||

       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;

    recGblRecordError( status, (void *)pBi,
		       "devAscii: (Bi init_record) Illegal format spec");

    pBi->dpvt = (void *) NULL;

    pBi->udf = TRUE;
  }
  else {
    pBi->udf = FALSE;

    pBi->dpvt = (void *) pPriv;
    
    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB = biReadAsyncCompletion;
    pPriv->aio.pArg = pBi;
    
    pBi->mask=1;
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * Used for obtaining the scan private info
 */
LOCAL long biGetIoIntInfo( int             cmd,
			   struct biRecord *pBi,
			   IOSCANPVT	   *ppvt)
{
  devAsciiPriv *pPriv = (devAsciiPriv *) pBi->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}

/*
 * ---------------------------------------------------------------------------
 * biRead() - BI input routine
 */
LOCAL long biRead (struct biRecord * pBi)
{
  devAsciiPriv 	*pPriv = (devAsciiPriv *)pBi->dpvt;
  long		val;
  long		status;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pBi->pact ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pBi->pact = TRUE;
    pBi->udf  = TRUE;

    return( S_devAscii_badRec );
  }
   
  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo( &pPriv->aio, &val );

  if ( status == S_drvAscii_AsyncCompletion ) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pBi->pact = TRUE;

    status = S_devAscii_OK;
  }
  else if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pBi->pact = TRUE;
      pBi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pBi,
			 "devAscii: Illegal format spec" );

      return( S_db_badField );
    }
    else
      /* read failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pBi, READ_ALARM, MAJOR_ALARM );
  }
  else {
    /* read completed non-asynchronously */
    pBi->rval = val;
  }
  
  return status;
}

/* 
 * ---------------------------------------------------------------------------
 *  biReadAsyncCompletion() - Asynchronous read completion routine for BI
 */
LOCAL void biReadAsyncCompletion(void *pArg, long status, long value)
{
  struct biRecord      *pBi = (struct biRecord *)pArg;
  struct dbCommon      *pRec = (struct dbCommon *)pArg;
  struct rset          *prset = (struct rset *)(pRec->rset);
  int                   dontProcess = FALSE;
  
  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pBi->pact = TRUE;
      pBi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pBi,
			 "devAscii: Illegal data format" );

      dontProcess = TRUE;
    }
    else
      /* if the read failed then set the record into alarm */
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
  }
  else {
    /* copy the value into the record */
    pBi->rval = value;
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}


/*
 * ---------------------------------------------------------------------------
 * boInitRec() - initializes BO records. This setups up all the drvAscii 
 *     and drvSerial info, all of which is attached to the record's  
 *     device private field.
 */
LOCAL long boInitRec(struct boRecord *pBo) 
{
  long          status;
  devAsciiPriv	*pPriv = NULL;
  long		rval;
   
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_OUT", 
				&pBo->out, pBo, &pPriv,
				&(pPriv->sigNum_arraySize) );
   
  /*
   * A prompt must exist. However a data type specification need not exist,
   * in which case '%f' is assumed.
   *
   * Responses must not cause assignment (ie. all reponse format strings must 
   * be of the form '%*' ). 
   *
   * Output formats of '%nc' and '%ns' are valid but 'n' must not be greater
   * than 4. That is, a 4 byte integer will be converted to an output string 
   * of 4 ascii chars (eg. 1633837924 -> 0x61626364 -> "abcd"). Note that the
   * resultant ascii chars may not be printable ascii chars 
   * (eg 23200612 -> 0x01620364 -> ^Ab^Cd, where ^A is control A -> 0x01).
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0)  
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pBo,
		       "devAscii: (Bo init_record) Illegal format spec");

    pBo->dpvt = (void *) NULL;

    pBo->udf  = TRUE;
  }
  else {
    pBo->udf  = FALSE;

    pBo->dpvt = pPriv;
    
    pBo->mask = 1;
    
    if ( status == OK 
	 && 
	 strlen( pPriv->aio.id.pCmndArg->rbvPrompt ) > 0 ) {
      /* if possible get an initial value for the record */
      status = drvAsciiReadOutput( &pPriv->aio.id, &rval );
    
      if ( status == OK ) {
	/* set the initial value */
	pBo->rval = (long) rval;
	
	status = S_devAscii_OK;
      }
    }
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * boWrite() - BO output routine
 */
LOCAL long boWrite (struct boRecord * pBo)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *)pBo->dpvt;
  int		status;
  long		rval;
  
  /* 
   * if a write is already in progress, or the record is locked-out,
   * then exit 
   */
  if ( pBo->pact ) return S_devAscii_OK; 
  
  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pBo->pact = TRUE;
    pBo->udf  = TRUE;

    return S_devAscii_badRec; 
  }
  
  rval = pBo->rval & pBo->mask;

  /* Perform the write */
  status = drvAsciiIntIo( &pPriv->aio, &rval );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pBo->pact = TRUE;
      pBo->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pBo,
			 "devAscii: Illegal data format" );

      return( S_db_badField );
    }
    else
      /* write failed */
      recGblSetSevr( (struct dbCommon *)pBo, WRITE_ALARM, MAJOR_ALARM );
  }

  return status;
}


/*
 * ---------------------------------------------------------------------------
 * mbbiInitRec() - initializes MBBI records. This setups up all the drvAscii 
 *     info, drvSerial info, and asynchronous completion callback info, 
 *     all of which is attached to the record's device private field.
 */
LOCAL long mbbiInitRec(struct mbbiRecord *pMbbi)
{
  devAsciiPriv *pPriv = NULL;
  long         status;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_IN", 
				&pMbbi->inp, pMbbi, &pPriv,
				&(pPriv->sigNum_arraySize) );
  
  /*
   * The prompt must exist but it cannot have a data type specification.
   *
   * The response format can be missing, in which case '%f' is assumed.
   *
   * The response format may be string ('%nc' or '%ns') but 'n' must not
   * be greater than 4. With a string specification the input ascii stream
   * will be converted to an integer value (eg. "abcd" -> 0x61626364 ->
   * 1633837924). 
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pMbbi,
		       "devAscii: (Mbbi init_record) Illegal format spec");

    pMbbi->dpvt = (void *) NULL;

    pMbbi->udf  = TRUE;
  }
  else {
    pMbbi->udf  = FALSE;

    pMbbi->dpvt = pPriv;
    
    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB = mbbiReadAsyncCompletion;
    pPriv->aio.pArg = pMbbi;
    
    pMbbi->shft = 0;
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * mbbiGetIoIntInfo() - Used for obtaining the scan private info
 */
LOCAL long mbbiGetIoIntInfo( int                 cmd,
			     struct mbbiRecord   *pMbbi,
			     IOSCANPVT           *ppvt)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pMbbi->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}

/*
 * ---------------------------------------------------------------------------
 * mbbiRead() - MBBI input routine
 */
LOCAL long mbbiRead (struct mbbiRecord * pMbbi)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pMbbi->dpvt;
  long   	val;
  long		status;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pMbbi->pact ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pMbbi->pact = TRUE;
    pMbbi->udf  = TRUE;

    return S_devAscii_badRec; 
  }
  
  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo(&pPriv->aio, &val);

  if (status == S_drvAscii_AsyncCompletion) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pMbbi->pact = TRUE;

    status = S_devAscii_OK;
  }
  else if (status) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pMbbi->pact = TRUE;
      pMbbi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pMbbi,
			 "devAscii: Illegal data format" );

      return( S_db_badField );
    }
    else
      /* read failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pMbbi, READ_ALARM, MAJOR_ALARM );
  }
  else {
    /* read completed non-asynchronously */
    pMbbi->rval = val;
  }
  
  return status;
}

/* 
 * ---------------------------------------------------------------------------
 * mbbiReadAsyncCompletion() -  Asynchronous read completion routine for BI
 */
LOCAL void mbbiReadAsyncCompletion(void *pArg, long status, long value)
{
  struct mbbiRecord *pMbbi= (struct mbbiRecord *)pArg;
  struct dbCommon   *pRec = (struct dbCommon *)pArg;
  struct rset       *prset = (struct rset *)(pRec->rset);
  int                dontProcess = FALSE;
  
  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pMbbi->pact = TRUE;
      pMbbi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pMbbi,
			 "devAscii: Illegal data format" );


      dontProcess = TRUE;
    }
    else
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
  }
  else {
    pMbbi->rval = value;
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}


/*
 * ---------------------------------------------------------------------------
 * mbboInitRec() - initializes MBBO records. This setups up all the drvAscii 
 *     and drvSerial info, all of which is attached to the record's device 
 *     private field.
 */
LOCAL long mbboInitRec(struct mbboRecord *pMbbo)
{
  long		status;
  devAsciiPriv	*pPriv = NULL;
  long 		rval;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_OUT", 
				&pMbbo->out, pMbbo, &pPriv,
				&(pPriv->sigNum_arraySize) );
  
  /*
   * A prompt must exist. However a data type specification need not exist,
   * in which case '%f' is assumed.
   *
   * Responses must not cause assignment (ie. all reponse format strings must 
   * be of the form '%*' ). 
   *
   * Output formats of '%nc' and '%ns' are valid but 'n' must not be greater
   * than 4. That is, a 4 byte integer will be converted to an output string 
   * of 4 ascii chars (eg. 1633837924 -> 0x61626364 -> "abcd"). Note that the
   * resultant ascii chars may not be printable ascii chars 
   * (eg 23200612 -> 0x01620364 -> ^Ab^Cd, where ^A is control A -> 0x01).
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0)  
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pMbbo,
		       "devAscii: (Mbbo init_record) Illegal format spec");

    pMbbo->dpvt = (void *) NULL;

    pMbbo->udf = TRUE;
  }
  else {
    pMbbo->udf = FALSE;

    pMbbo->dpvt = (void *) pPriv;
    
    pMbbo->shft = 0;
    
    if ( status == OK 
	 && 
	 strlen( pPriv->aio.id.pCmndArg->rbvPrompt ) > 0 ) {
      /* if possible get an initial value for the record */
      status = drvAsciiReadOutput( &pPriv->aio.id, &rval );
      
      if ( status == OK ) {
	/* set the initial value */
	pMbbo->rval = rval;
	
	status = S_devAscii_OK;
      }
    }
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * mbboWrite() - MBBO output routine
 */
LOCAL long mbboWrite (struct mbboRecord *pMbbo)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pMbbo->dpvt;
  int		status;
  long		rval;
  
  /* 
   * if a write is already in progress, or the record is locked-out,
   * then exit 
   */
  if ( pMbbo->pact ) return S_devAscii_OK; 
  
  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pMbbo->pact = TRUE;
    pMbbo->udf  = TRUE;

   return S_devAscii_badRec; 
  }

  /* use of NOBT and MASK not currently implemented */
  rval = pMbbo->rval;

  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo( &pPriv->aio, &rval );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pMbbo->pact = TRUE;
      pMbbo->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pMbbo,
			 "devAscii: Illegal data format" );

      return( S_db_badField );
    }
    else
      /* write failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pMbbo, WRITE_ALARM, MAJOR_ALARM );
  }
  return status;
}
 

/*
 * ---------------------------------------------------------------------------
 * siInitRec() - initializes stringin records. This setups up all the drvAscii 
 *     info, drvSerial info, and asynchronous completion callback info, all of 
 *     which is attached to the record's device private field.
 */
LOCAL long siInitRec(struct stringinRecord *pSi)
{
  devAsciiPriv  *pPriv = NULL;
  long          status = S_devAscii_OK;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "STRING_IN",
				&pSi->inp, pSi, &pPriv,	
				&(pPriv->sigNum_arraySize) );

  /*
   * The prompt must exist. if %s exists in the prompt then the contents
   * of VAL will be used as part or all of the prompt.
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0 
	&&
	pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_STRING ) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED 
	&&
	pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_STRING ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_STRING 
	&&
	pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pSi,
		       "devAscii: (stringIn init_record) Illegal format spec");

    pSi->udf  = TRUE;

    pSi->dpvt = (void *) NULL;
  }
  else {
    pSi->udf  = FALSE;

    pSi->dpvt = pPriv;
    
    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB = siReadAsyncCompletion;
    pPriv->aio.pArg = pSi;
    pPriv->aio.id.respStr[0] = '\0';
    
    pSi->val[0] = '\0';
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * siGetIoIntInfo() - Used for obtaining the scan private info
 */
LOCAL long siGetIoIntInfo(   int                     cmd,
			     struct stringinRecord   *pSi,
			     IOSCANPVT               *ppvt)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pSi->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}

/*
 * ---------------------------------------------------------------------------
 * siRead() - stringin input routine
 */
LOCAL long siRead (struct stringinRecord * pSi)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pSi->dpvt;
  drvAsyncIO    *pDrvAsyncIO = &pPriv->aio;
  long		status;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pSi->pact ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pSi->pact = TRUE;
    pSi->udf  = TRUE;

    /*
     *  Must set the severity as record processing does not do so
     *  for this record type.
     */
    recGblSetSevr( (struct dbCommon *)pSi, UDF_ALARM, INVALID_ALARM );

    return S_devAscii_badRec; 
  }
  
  /* Perform the read and if completion is asynchronous set the pact */
  if ( pDrvAsyncIO->id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING ) 
    /* 
     * we do not check whether or not the val field is null. If it is null
     * then only the current write command terminator will be output. Note
     * that this could be anything by setting the readCMT record.
     */
    status = drvAsciiStringIo( &pPriv->aio, pSi->val );
  else
    status = drvAsciiStringIo( &pPriv->aio, "" );

  if ( status == S_drvAscii_AsyncCompletion ) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pSi->pact = TRUE;

    status = S_devAscii_OK;
  }
  else if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pSi->pact = TRUE;
      pSi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pSi,
			 "devAscii: Illegal data format" );

      /*
       *  Must set the severity as record processing does not do so
       *  for this record type.
       */
      recGblSetSevr( (struct dbCommon *)pSi, UDF_ALARM, INVALID_ALARM );

      return( S_db_badField );
    }
    else
      /* read failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pSi, READ_ALARM, MAJOR_ALARM );
  }
  else {
    /* read completed non-asynchronously */
   strncpy( pSi->val, pDrvAsyncIO->id.respStr, 
	    min( pDrvAsyncIO->id.respStrCnt, sizeof( pSi->val ) ) );

   pSi->udf = FALSE;
  }
  
  return status;
}

/* 
 * ---------------------------------------------------------------------------
 * siReadAsyncCompletion() -  Asynchronous read completion routine for 
 *   stringin records
 */
LOCAL void siReadAsyncCompletion( void *pArg, long status, long value )
{
  struct stringinRecord *pSi = (struct stringinRecord *)pArg;
  struct dbCommon       *pRec = (struct dbCommon *)pArg;
  struct rset           *prset = (struct rset *)(pRec->rset);
  devAsciiPriv          *pDevAsciiPriv = (devAsciiPriv *)(pSi->dpvt);
  drvAsyncIO            *pDrvAsyncIO = &pDevAsciiPriv->aio;
  int                    dontProcess = FALSE;

  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pSi->pact = TRUE;
      pSi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pSi,
			 "devAscii: Illegal data format" );

      /*
       *  Must set the severity as record processing does not do so
       *  for this record type.
       */
      recGblSetSevr( (struct dbCommon *)pSi, UDF_ALARM, INVALID_ALARM );

      dontProcess = TRUE;
    }
    else
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
  }
  else {
    /* copy the input into the record */
   strncpy( pSi->val, pDrvAsyncIO->id.respStr, 
	    min( pDrvAsyncIO->id.respStrCnt, sizeof( pSi->val ) ) );

   pSi->udf = FALSE;
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}



/*
 * ---------------------------------------------------------------------------
 * soInitRec() - initializes stringout records. This setups up
 *     all the drvAscii and drvSerial info, all of which is attached 
 *     to the record's device private field.
 */
LOCAL long soInitRec(struct stringoutRecord *pSo)
{
  long		status;
  devAsciiPriv	*pPriv = NULL;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "STRING_OUT", 
				&pSo->out, pSo, &pPriv,
				&(pPriv->sigNum_arraySize) );
    
  /*
   * A prompt need not exist. However if a prompt does exist and a data type 
   * is specified then the data type must be string (ie. '%ns' is the only
   * valid option). If no data type is specified then '%s' is assumed.
   *
   * Responses must not cause assignment (ie. all reponse format strings must 
   * be of the form '%*' ). 
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  if ( (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_STRING 
	&&
        pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
        (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError(status, (void *)pSo,
		      "devAscii: (stringOut init_record) Illegal format spec");

    pSo->dpvt = (void *) NULL;

    pSo->udf = TRUE;
  }
  else {
    pSo->udf = FALSE;

    pSo->dpvt = (void *) pPriv;
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * soWrite() - string output routine
 */
LOCAL long soWrite (struct stringoutRecord *pSo)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pSo->dpvt;
  int		status;
  
  /* 
   * if a write is already in progress, or the record is locked-out,
   * then exit 
   */
  if ( pSo->pact ) return S_devAscii_OK; 
  
  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pSo->pact = TRUE;
    pSo->udf  = TRUE;

    return S_devAscii_badRec; 
  }
  
  /* Perform the read */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) > 0) 
       &&
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_UNDEFINED ) ) 
    status = drvAsciiStringIo( &pPriv->aio, "" );
  else 
    status = drvAsciiStringIo( &pPriv->aio, pSo->val );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pSo->pact = TRUE;
      pSo->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pSo,
			 "devAscii: Illegal data format" );

      return( S_db_badField );
    }
    else
      /* the write failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pSo, WRITE_ALARM, MAJOR_ALARM );
  }

  return status;
}


/*
 * ---------------------------------------------------------------------------
 * liInitRec()- initializes longin records. This setups up
 *     all the drvAscii info, drvSerial info, and asynchronous completion
 *     callback info, all of which is attached to the record's device
 *     private field.
 */
LOCAL long liInitRec(struct longinRecord *pLi) 
{
  long         status;
  devAsciiPriv *pPriv = NULL;

  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_IN", 
				&pLi->inp, pLi, &pPriv,
				&(pPriv->sigNum_arraySize) );
  
  /*
   * The prompt must exist but it cannot have a data type specification.
   *
   * The response format can be missing, in which case '%f' is assumed.
   *
   * The response format may be string ('%nc' or '%ns') but 'n' must not
   * be greater than 4. With a string specification the input ascii stream
   * will be converted to an integer value (eg. "abcd" -> 0x61626364 ->
   * 1633837924). 
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pLi,
		       "devAscii: (longIn init_record) Illegal format spec");

    pLi->dpvt = (void *) NULL;

    pLi->udf  = TRUE;
  }
  else {
    pLi->udf  = FALSE;

    pLi->dpvt = pPriv;
  
    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB = liReadAsyncCompletion;
    pPriv->aio.pArg = pLi;
  }

  return status;
}

/* 
 * ---------------------------------------------------------------------------
 * liGetIoIntInfo() -  Asynchronous read completion routine for AI
 */
LOCAL long liGetIoIntInfo( int                 cmd,
			   struct longinRecord *pLi,
			   IOSCANPVT	       *ppvt)
{
  devAsciiPriv *pPriv = (devAsciiPriv *) pLi->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}

/*
 * ---------------------------------------------------------------------------
 * liRead() - longin input routine
 */
LOCAL long liRead (struct longinRecord * pLi)
{
  devAsciiPriv 	*pPriv = (devAsciiPriv *)pLi->dpvt;
  long		val;
  long		status;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pLi->pact == TRUE ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pLi->pact = TRUE;
    pLi->udf  = TRUE;
    
    return( S_devAscii_badRec );
  }
   
  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo( &pPriv->aio, &val );
  if ( status == S_drvAscii_AsyncCompletion ) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pLi->pact = TRUE;

    status = S_devAscii_OK;
  }
  else if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pLi->pact = TRUE;
      pLi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pLi,
			 "devAscii: Illegal format spec" );

      return( S_db_badField );
    }
    else {
      /* read failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pLi, READ_ALARM, MAJOR_ALARM );
    }
  }
  else {
    /* read completed non-asynchronously */
    pLi->val = val;
  }
  
  return status;
}

/* 
 * ---------------------------------------------------------------------------
 * liReadAsyncCompletion() -  Asynchronous read completion routine for longin
 */
LOCAL void liReadAsyncCompletion(void *pArg, long status, long value)
{
  struct longinRecord   *pLi = (struct longinRecord *)pArg;
  struct dbCommon       *pRec = (struct dbCommon *)pArg;
  struct rset           *prset = (struct rset *)(pRec->rset);
  int                    dontProcess = FALSE;
  
  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pLi->pact = TRUE;
      pLi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pLi,
			 "devAscii: Illegal format spec" );

      dontProcess = TRUE;
    }
    else {
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
    }
  }
  else {
    /* copy the value into the record */
    pLi->val = value;
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}


/*
 * ---------------------------------------------------------------------------
 * loInitRec()- initializes longout records. This setups up all the drvAscii 
 *     and drvSerial info, all of which is attached to the record's device 
 *     private field.
 */
LOCAL long loInitRec(struct longoutRecord *pLo) 
{
  long          status;
  devAsciiPriv	*pPriv = NULL;
  long		rval;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_OUT", 
				&pLo->out, pLo, &pPriv,
				&(pPriv->sigNum_arraySize) );
   
  /*
   * A prompt must exist. However a data type specification need not exist,
   * in which case '%f' is assumed.
   *
   * Responses must not cause assignment (ie. all reponse format strings must 
   * be of the form '%*' ). 
   *
   * Output formats of '%nc' and '%ns' are valid but 'n' must not be greater
   * than 4. That is, a 4 byte integer will be converted to an output string 
   * of 4 ascii chars (eg. 1633837924 -> 0x61626364 -> "abcd"). Note that the
   * resultant ascii chars may not be printable ascii chars 
   * (eg 23200612 -> 0x01620364 -> ^Ab^Cd, where ^A is control A -> 0x01).
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0)  
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
      ((pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING) 
       &&
        (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pLo,
		       "devAscii: (longOut init_record) Illegal format spec");

    pLo->dpvt = (void *) NULL;

    pLo->udf  = TRUE;
  }
  else {
    pLo->udf  = FALSE;

    pLo->dpvt = pPriv;
    
    /* even on error ensure the record's fields are initialized */
    if ( status == OK 
	 && 
	 strlen( pPriv->aio.id.pCmndArg->rbvPrompt ) > 0 ) {
      /* if possible get an initial value for the record */
      status = drvAsciiReadOutput( &pPriv->aio.id, &rval );

      if ( status == OK ) {
	/* set the initial value */
	pLo->val = rval;

	status = S_devAscii_OK;
      }
    }
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * loWrite() - longout output routine
 */
LOCAL long loWrite (struct longoutRecord * pLo)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *)pLo->dpvt;
  int		status;
  long		rval;
  
  /* 
   * if a write is already in progress, or the record is locked-out,
   * then exit 
   */
  if ( pLo->pact ) return S_devAscii_OK; 
  
  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pLo->pact = TRUE;
    pLo->udf  = TRUE;

    return S_devAscii_badRec; 
  }
  
  rval = pLo->val;

  /* Perform the write */
  status = drvAsciiIntIo( &pPriv->aio, &rval );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pLo->pact = TRUE;
      pLo->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pLo,
			 "devAscii: Illegal data format" );

      return( S_db_badField );
    }
    else
      /* the write failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pLo, WRITE_ALARM, MAJOR_ALARM );
  }
  return status;
}


/*
 * ---------------------------------------------------------------------------
 * mbbiDirectInitRec() - initializes MBBI direct records. This setups up
 *     all the drvAscii info, drvSerial info, and asynchronous completion
 *     callback info, all of which is attached to the record's device
 *     private field.
 */
LOCAL long mbbiDirectInitRec(struct mbbiDirectRecord *pMbbi)
{
  devAsciiPriv *pPriv = NULL;
  long         status;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_IN", 
				&pMbbi->inp, pMbbi, &pPriv,
				&(pPriv->sigNum_arraySize) );
  
  /*
   * The prompt must exist but it cannot have a data type specification.
   *
   * The response format can be missing, in which case '%f' is assumed.
   *
   * The response format may be string ('%nc' or '%ns') but 'n' must not
   * be greater than 4. With a string specification the input ascii stream
   * will be converted to an integer value (eg. "abcd" -> 0x61626364 ->
   * 1633837924). 
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pMbbi,
		    "devAscii: (Mbbi direct init_record) Illegal format spec");

    pMbbi->dpvt = (void *) NULL;

    pMbbi->udf = TRUE;
  }
  else {
    pMbbi->udf = FALSE;

    pMbbi->dpvt = pPriv;
    
    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB = mbbiDirectReadAsyncCompletion;
    pPriv->aio.pArg = pMbbi;
    
    pMbbi->shft = 0;
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * mbbiDirectGetIoIntInfo() - Used for obtaining the scan private info
 */
LOCAL long mbbiDirectGetIoIntInfo( int                     cmd,
				   struct mbbiDirectRecord *pMbbi,
				   IOSCANPVT               *ppvt)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pMbbi->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}

/*
 * ---------------------------------------------------------------------------
 * mbbiDirectRead() - MBBI direct input routine
 */
LOCAL long mbbiDirectRead (struct mbbiDirectRecord * pMbbi)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pMbbi->dpvt;
  long   	val;
  long		status;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pMbbi->pact ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pMbbi->pact = TRUE;
    pMbbi->udf  = TRUE;

    /*
     *  Must set the severity as record processing does not do so
     *  for this record type.
     */
    recGblSetSevr( (struct dbCommon *)pMbbi, UDF_ALARM, INVALID_ALARM );

    return S_devAscii_badRec; 
  }
  
  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo(&pPriv->aio, &val);

  if (status == S_drvAscii_AsyncCompletion) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pMbbi->pact = TRUE;

    status = S_devAscii_OK;
  }
  else if (status) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pMbbi->pact = TRUE;
      pMbbi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pMbbi,
			 "devAscii: Illegal format spec" );

      recGblSetSevr( (struct dbCommon *)pMbbi, UDF_ALARM, INVALID_ALARM );

      return( S_db_badField );
    }
    else
      /* read failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pMbbi, READ_ALARM, MAJOR_ALARM );

  } else {
    /* read completed non-asynchronously */
    pMbbi->rval = val;
  }
  
  return status;
}

/* 
 * ---------------------------------------------------------------------------
 * mbbiDirectReadAsyncCompletion() -  Asynchronous read completion routine 
 *   for MBBI direct
 */
LOCAL void mbbiDirectReadAsyncCompletion(void *pArg, long status, long value)
{
  struct mbbiDirectRecord *pMbbi= (struct mbbiDirectRecord *)pArg;
  struct dbCommon         *pRec = (struct dbCommon *)pArg;
  struct rset             *prset = (struct rset *)(pRec->rset);
  int                      dontProcess = FALSE;
  
  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /* set pact so record processing will never occur */
      pMbbi->pact = TRUE;
      pMbbi->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pMbbi,
			 "devAscii: Illegal format spec" );

      /*
       *  Must set the severity as record processing does not do so
       *  for this record type.
       */
      recGblSetSevr( (struct dbCommon *)pMbbi, UDF_ALARM, INVALID_ALARM );

      dontProcess = TRUE;
    }
    else
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
  }
  else {
    pMbbi->rval = value;
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}


/*
 * ---------------------------------------------------------------------------
 * mbboDirectInitRec() - initializes MBBO direct records. This setups up
 *     all the drvAscii and drvSerial info, all of which is attached 
 *     to the record's device private field.
 */
LOCAL long mbboDirectInitRec(struct mbboDirectRecord *pMbbo)
{
  long		status;
  devAsciiPriv	*pPriv = NULL;
  long 		rval;
  
  /* initialize the record and driver */
  status = devAsciiInitPrivate( "INTEGER_OUT",
				&pMbbo->out, pMbbo, &pPriv,
				&(pPriv->sigNum_arraySize) );
  
  /*
   * A prompt must exist. However a data type specification need not exist,
   * in which case '%f' is assumed.
   *
   * Responses must not cause assignment (ie. all reponse format strings must 
   * be of the form '%*' ). 
   *
   * Output formats of '%nc' and '%ns' are valid but 'n' must not be greater
   * than 4. That is, a 4 byte integer will be converted to an output string 
   * of 4 ascii chars (eg. 1633837924 -> 0x61626364 -> "abcd"). Note that the
   * resultant ascii chars may not be printable ascii chars 
   * (eg 23200612 -> 0x01620364 -> ^Ab^Cd, where ^A is control A -> 0x01).
   *
   * Also arrays of values are not currently supported (Jan. 1997).
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0)  
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll 
       || 
       ((pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType == RBF_STRING) 
	&&
        (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataCnt > 4) ) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pMbbo,
		    "devAscii: (Mbbo direct init_record) Illegal format spec");

    pMbbo->dpvt = (void *) NULL;

    pMbbo->udf  = TRUE;
  }
  else {
    pMbbo->udf  = FALSE;

    pMbbo->dpvt = (void *) pPriv;
    
    pMbbo->shft = 0;
    
    if ( status == OK 
	 && 
	 strlen( pPriv->aio.id.pCmndArg->rbvPrompt ) > 0 ) {
      /* if possible get an initial value for the record */
      status = drvAsciiReadOutput( &pPriv->aio.id, &rval );
    
      if ( status == OK ) {
	/* set the initial value */
	pMbbo->rval = rval;

	status = S_devAscii_OK;
      }
    }
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * mbboDirectWrite() - MBBO direct output routine
 */
LOCAL long mbboDirectWrite (struct mbboDirectRecord *pMbbo)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pMbbo->dpvt;
  int		status;
  long		rval;
  
  /* 
   * if a write is already in progress, or the record is locked-out,
   * then exit 
   */
  if ( pMbbo->pact ) return S_devAscii_OK; 
  
  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pMbbo->pact = TRUE;
    pMbbo->udf  = TRUE;

    /*
     *  Must set the severity as record processing does not do so
     *  for this record type.
     */
    recGblSetSevr( (struct dbCommon *)pMbbo, UDF_ALARM, INVALID_ALARM );

    return S_devAscii_badRec; 
  }

  /* use of NOBT and MASK not currently implemented */
  rval = pMbbo->rval;

  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiIntIo( &pPriv->aio, &rval );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a write in the future.
       */
      pMbbo->pact = TRUE;
      pMbbo->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pMbbo,
			 "devAscii: Illegal data format" );

      /*
       *  Must set the severity as record processing does not do so
       *  for this record type.
       */
      recGblSetSevr( (struct dbCommon *)pMbbo, UDF_ALARM, INVALID_ALARM );

      return( S_db_badField );
    }
    else
      /* write failed so set the record into alarm */
      recGblSetSevr( (struct dbCommon *)pMbbo, WRITE_ALARM, MAJOR_ALARM );
  }
  return status;
}
 

/*
 * ---------------------------------------------------------------------------
 * wfInitRec() - initializes waveform records. The waveform record is used
 *     as a long stringin record, that is form inputting strings that are
 *     greater than default stringin record size (40 bytes).
 *
 *     This setups up all the drvAscii info, drvSerial info, and asynchronous
 *     completion callback info, all of which is attached to the record's 
 *     device private field.
 */
LOCAL long wfInitRec(struct waveformRecord *pWf)
{
  devAsciiPriv  *pPriv = NULL;
  long          status = S_devAscii_OK;
  
  if ( (pWf->ftvl != DBF_CHAR) && (pWf->ftvl != DBF_STRING) ) {
    status = S_db_badField;

    recGblRecordError( status, (void *)pWf,
		       "devAscii: (waveform init_record) Illegal format spec");

    return status;
  }

  /* initialize the record and driver */
  status = devAsciiInitPrivate( "STRING_IN",
				&pWf->inp, pWf, &pPriv,	
				&(pPriv->sigNum_arraySize) );

  /*
   * The prompt must exist but it cannot have a data type specification.
   *
   */
  if ( (strlen(pPriv->aio.id.pCmndArg->cmndPrompt) == 0) 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killCnt 
       ||
       pPriv->aio.id.pCmndArg->cmndPromptInfo.killAll || 
       (pPriv->aio.id.pCmndArg->cmndPromptInfo.dataType != RBF_UNDEFINED) 
       ||
       (pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_STRING 
	&&
        pPriv->aio.id.pCmndArg->cmndInfo.dataType != RBF_UNDEFINED) 
       ||
       (status != S_drvAscii_Ok) ) {
    /*
     *  Something is wrong with the record so mark it for alarms.
     */
    status = S_db_badField;
    recGblRecordError( status, (void *)pWf,
		       "devAscii: (waveform init_record) Illegal format spec");

    pWf->dpvt = (void *) NULL;

    pWf->udf  = TRUE;
  }
  else {
    pWf->udf  = FALSE;

    pWf->dpvt = pPriv;
    
    /* 
     * setup the async callback routine, the arg to which is a pointer back 
     * to the record
     */
    pPriv->aio.pCB = wfReadAsyncCompletion;
    pPriv->aio.pArg = pWf;
    pPriv->aio.id.respStr[0] = '\0';
    
    ((char *)pWf->bptr)[0] = '\0';
  }

  return status;
}

/*
 * ---------------------------------------------------------------------------
 * wfGetIoIntInfo() - Used for obtaining the scan private info
 */
LOCAL long wfGetIoIntInfo(   int                     cmd,
			     struct waveformRecord   *pWf,
			     IOSCANPVT               *ppvt)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pWf->dpvt;
  
  if ( pPriv ) {
    *ppvt = pPriv->spvt;
  }
  else *ppvt = NULL;

  return S_devAscii_OK;
}

/*
 * ---------------------------------------------------------------------------
 * wfRead() - waveform input routine
 */
LOCAL long wfRead (struct waveformRecord * pWf)
{
  devAsciiPriv	*pPriv = (devAsciiPriv *) pWf->dpvt;
  drvAsyncIO    *pDrvAsyncIO = &pPriv->aio;
  long		status;
  
  /* 
   * if a read is already in progress, or the record is locked out, then 
   * exit 
   */
  if ( pWf->pact ) return S_devAscii_OK; 

  /* if the device private structure was never created then quit */
  if ( pPriv == NULL ) {
    /* set pact so record processing will never occur */
    pWf->pact = TRUE;
    pWf->udf  = TRUE;

    /*
     *  Must set the severity as record processing does not do so
     *  for this record type.
     */
    recGblSetSevr( (struct dbCommon *)pWf, UDF_ALARM, INVALID_ALARM );

    return S_devAscii_badRec; 
  }
  
  /* Perform the read and if completion is asynchronous set the pact */
  status = drvAsciiStringIo( &pPriv->aio, "" );
  if ( status == S_drvAscii_AsyncCompletion ) {
    /*
     *  The record will be asynchronously called back when the 
     *  read completes so mark it as such.
     */
    pWf->pact = TRUE;

    status = S_devAscii_OK;
  }
  else if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /*
       *  There is something wrong with the record so mark
       *  it so it will never attempt a read in the future.
       */
      pWf->pact = TRUE;
      pWf->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pWf,
			 "devAscii: Illegal format spec" );

      recGblSetSevr( (struct dbCommon *)pWf, UDF_ALARM, INVALID_ALARM );

      return status;
    }
    else
    /* read failed so set the record into alarm */
    recGblSetSevr( (struct dbCommon *)pWf, READ_ALARM, MAJOR_ALARM );
  }
  else {
    /* read completed non-asynchronously */
    pWf->nord = min( pDrvAsyncIO->id.respStrCnt, pWf->nelm );
    strncpy( pWf->bptr, pDrvAsyncIO->id.respStr, pWf->nord );
  }
  
  return status;
}

/* 
 * ---------------------------------------------------------------------------
 * wfReadAsyncCompletion() -  Asynchronous read completion routine for 
 *   stringin records
 */
LOCAL void wfReadAsyncCompletion( void *pArg, long status, long value )
{
  struct waveformRecord *pWf = (struct waveformRecord *)pArg;
  struct dbCommon       *pRec = (struct dbCommon *)pArg;
  struct rset           *prset = (struct rset *)(pRec->rset);
  devAsciiPriv          *pDevAsciiPriv = (devAsciiPriv *)(pWf->dpvt);
  drvAsyncIO            *pDrvAsyncIO = &pDevAsciiPriv->aio;
  int                    dontProcess = FALSE;
 
  dbScanLock( pRec );

  if ( status ) {
    if ( status == S_drvAscii_badParam ) {
      /* set pact so record processing will never occur */
      pWf->pact = TRUE;
      pWf->udf  = TRUE;

      recGblRecordError( S_db_badField, (void *)pWf,
			 "devAscii: Illegal format spec" );

      /*
       *  Must set the severity as record processing does not do so
       *  for this record type.
       */
      recGblSetSevr( (struct dbCommon *)pWf, UDF_ALARM, INVALID_ALARM );

      dontProcess = TRUE;
    }
    else {
      recGblSetSevr( pRec, READ_ALARM, MAJOR_ALARM );
    }
  }
  else {
    /* copy the input into the record */
    pWf->nord = min( pDrvAsyncIO->id.respStrCnt, pWf->nelm );
    strncpy( pWf->bptr, pDrvAsyncIO->id.respStr, pWf->nord );
  }

  if ( !dontProcess )
    /* cause the record to process. This will clear the pact */
    (*prset->process)( pRec );

  dbScanUnlock( pRec );
}



/*+*********************************************************************
  $Log: devAscii.c,v $
  Revision 1.2  2001/01/30 22:48:32  peregrin
  Added ASCII_DEBUG flag.

  Revision 1.1  2000/04/20 01:37:13  peregrin
  End of April bright time devel.

  Revision 1.3  1999/09/02 01:24:01  kics
  Significant changes to alleviate miscomms between read and write
  tasks. This was instigated by changes to drvCmSx which suffered
  from similar problem (ie. this is derived from that).

  Revision 1.2  1999/07/16 04:20:09  ahoney
  Corrected a bug in mbboInitRec.
  Updated a few statuses.

  Revision 1.1  1998/12/03 23:56:10  ktsubota
  Initial insertion

  Revision 1.13  1998/04/03 23:13:16  ahoney
  Removed the previous mod for string out records, as it changes previous
  behaviour.

  Revision 1.12  1998/03/18 01:39:20  ahoney
  Updated the processing for stringIn records to allows for dynamic
  prompts. This is accomplished by using the VAL field for output when
  prompting and input when a response arrives.

  Revision 1.11  1997/02/08 00:49:21  ahoney
  Removed setting of PACT=1 on failure within mbbireadasynccompletion

  As this would permanently disable the relevant record(s).

 * Revision 1.10  1997/01/23  02:14:03  ahoney
 * Within string input routines I changes strlen( pSi->val ) to
 * sizeof( pSi->val ). These were potential bugs.
 *
 * Revision 1.9  1997/01/21  02:37:55  ahoney
 * Mods to better handle conversion from integer to strings. This is
 * still unacceptable as drvAscii cannot handle null bytes as they
 * appear to be end-of-string terminators.
 *
 * Revision 1.8  1997/01/20  20:50:56  ahoney
 * Extensive mods to accomodate:
 *   -added mbbi and mbbo direct records;
 *   -added waveform records (long stringin);
 *   -added conversion of binary streams '0' and '1', with or without
 *    delimiters
 *   -added conversion from string to numeric 'abcd'->0x61626364->1633837924
 *   -added support for muliple line input strings
 *
 * Revision 1.7  1996/12/18  23:27:30  ahoney
 * Mods to support ao,bi,bo,mbbi,mbbo,longin,longout,stringin, and stringout
 * records. These were necessitated for use with IFSM and chopper.
 *
 * Revision 1.6  1996/09/13  21:04:59  ahoney
 * removed '#define LOCAL' which used during debugging.
 *
 * Revision 1.5  1996/09/13  20:49:50  ahoney
 * Mods to accomodate the flat lamp device.
 *
 * Note this driver was completed only so far as was necessary for the
 * data acquisition systems. The drive will still need a few mods for
 * the IFSM.
 *
 * Revision 1.4  1996/09/11  21:51:48  ahoney
 * Mods to incorporate longin and longout records as needed for the
 * dome flat lamps. Also modified the handling of '%nk' in data formats
 * so that 'n' characters can be 'killed' at the beginning of a data
 * stream - this allows stripping leading NULLs,...
 *
 * Revision 1.3  1996/08/14  18:37:55  ahoney
 * Modified all async completion routines so that PACT is set false on
 * errors. This was done to correct the problem when the serial link is
 * down on startup.
 *
 * Revision 1.2  1996/07/13  00:38:03  ahoney
 * Removed some debugging printf's.
 *
 * Revision 1.1  1996/07/12  23:26:22  ahoney
 * drvAscii is a new directory for support for serial comms to remote
 * devices via ascii strings
 *
 *
***********************************************************************/
 


