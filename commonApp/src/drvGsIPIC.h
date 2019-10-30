/* *****************************************************************************
**
**	Module:		drvGsIPIC.h
**	Purpose:	Header file for GreenSpring IPIC616 IP carrier board
**
**		This header file is based loosely on Kay-Uwe Kasemir's
**		4-9-96 code.  The major changes made involved separating
**		as much as possible from the carrier board code and
**		inserting it into each modules code.
**		This file contains the memory mapping of the ID-PROM
**		area of the IP modules, manifest constants, and function
**		prototypes.
**     
**		This include file needs to be referenced in all of the 
**		Greenspring module includes for the IP16ADC, IPDACSU, 
**		IPUnidig-I-HV, IPUnidig-I-E, and the IPRelay that were
**		were written by GAD beginning on 5/1/98.
**		
** *****************************************************************************
**
**	Written By:		G.A. Domer, AlliedSignal FM&T
**	Started On:		05/05/98
**	Project Title:		EPICS - Driver
**	Commissioned by:	LANL
**
** *****************************************************************************
**
**	  Date		Revisor	Ver #	Modification Description
**	________	_______	_____  	_________________________________________
**							   	 		
**	05/01/98	  GAD	1.0		Original code	
**
**										
**
**
**										
**
**
**										
**
**
** ****************************************************************************/

typedef unsigned char	Byte;
typedef unsigned short	Word;

/* IDPROM structure for Greenspring IP modules.  Common for all modules*/
typedef struct {
	Byte	blank0;		/* all even # bytes are not used 		*/
	Byte	leti;		/* Letter 'I'							*/
	Byte	blank2;
	Byte	letp;		/* Letter 'P'							*/
	Byte	blank4;
	Byte	leta;		/* Letter 'A'							*/
	Byte	blank6;
	Byte	letc;		/* Letter 'C'							*/
	Byte	blank8;
	Byte	manid;		/* Manufacturer ID						*/
	Byte	blank10;
	Byte	model;		/* Model number							*/
	Byte	blank12;
	Byte	rev;		/* Revision number						*/
	Byte	blank1416[3];/* Bytes 14,15, & 16					*/
	Byte	driverl;	/* Driver Low Byte						*/
	Byte	blank18;
	Byte	driverh;	/* Driver High Byte						*/
	Byte	blank20;
	Byte	bytes;		/* Number of Bytes Used					*/
	Byte	blank22;
	Byte	crc;		/* CRC									*/
}	IDPROM;



/* Structure to hold all of the information for Greenspring IP modules */
/* loaded into an IOC.  Common to all IP modules.					   */
typedef struct {
	Byte	*base;			/* base address in VME short I/O */
	Byte	int_level0;		/* interrupt level for IP's int0 */
	Byte	int_level1;		/* interrupt level for IP's int1 */
	Byte	*local;			/* base address in local memory  */
	Byte	manufacturer;	/* manufacturer ID - F0 for Greenspring */
	Byte	model;			/* model ID - unique for each module type */
	Byte	revision;		/* prom revision */
	Word	driver;			/* driver version */
}	IPIC_Info;
/*
 *	This is the first interrupt vector the IPs will use.
 *	If this is already used, the next one will be tried
 */
#define IP_INT_VECTOR_BASE	70

/*
 *	This is the maximum number of IP modules of any type that can be installed.
 */
#define IPIC_NUM_CARDS 64

/*
 *	This is the maximum number of IP carriers that can be installed.
 */
#define IPIC_NUM_CARRIERS 16

/*
 * function prototypes 
 */
long IPIC_report (int type);

const IPIC_Info *IPIC_get_IPID (void);

/* defined constants to print DEBUG messages in code. */
#ifdef NO_DEBUG

#define DEBUG(anycode)				/*  */

#else

extern int	verbose_IP_code;

#define DEBUG(anycode)					\
	if (verbose_IP_code) {				\
		anycode							\
	}

#endif


/*	EOF	drvGSIPIC610.h */
