/* *****************************************************************************
**
**	Module:		drvGsIPIC.c
**	Purpose:	Source code file for GreenSpring IPIC616 IP carrier board
**
**		This code file is based loosely on Kay-Uwe Kasemir's
**		4-9-96 code.  The major changes made involved separating
**		as much as possible from the carrier board code and
**		inserting it into each modules code.
**		This file contains the common code which maps the IP modules
**		into a common array of structures which will be accessed by 
**		the individual module drivers.
**     
**		The main purpose of this module is to provide a common storage
**		area for all of the IP carriers and modules for an IOC rack.
**		Carriers are number from 0 - 16 as they are found in the address
**		ranges of 0x0000 through 0xF300.  Module are defined as 0 being 
**		in Slot A, 1 being in Slot B, 2 being in Slot C, and 3 being in
**		Slot D.
**		Each independent driver for the different modules will use this
**		common storage area to locate the modules and then assign the 
**		the EPICS card number with separate card number series for each 
**		card type.
**		Example:
**				Carrier board 0 -
**					Module 0 - IP16ADC Card #0
**			    	Module 1 - IP16ADC Card #1
**					Module 2 - IPUnidig-I-HV Card #0
**					Module 3 - IPRelay Card #0
**				Carrier board 1 -
**					Module 0 - IPUndidig-I-HV Card #1
**					Module 1 - IPDACSU Card #0
**					Module 2 - IPDACSU Card #1
**					Module 3 - IPRelay Card #1
**
**		Jumpers:
**
**		Address set by placing or removing the jumpers on E3 and E7.  Shorting
**		a terminal between the two represents a '0' and openning represents a '1'.
**      The addresses for the cards should always fall onto a 0x1000 boundary
**		in between the range of 0x0000 - 0xF000.  Address jumpers settings are
**		as follows:
**		
**						Address			E3 - E7 Jumper Pins
**								     7   6 	 5   4   3   2   1 		   			    
**						0X0000	    IN  IN 	IN  IN  IN  IN  IN 		   			    
**						0X1000	    IN  IN  IN  OUT IN  IN  IN 	   			        
**						0X2000	    IN  IN  OUT IN  IN  IN  IN 		   			    
**						0X3000	    IN  IN  OUT OUT IN  IN  IN 	   			        
**						0X4000	    IN  OUT IN  IN  IN  IN  IN 		   			    
**						0X5000	    IN  OUT IN  OUT IN  IN  IN 	   			        
**						0X6000	    IN  OUT OUT IN  IN  IN  IN 		   			    
**						0X7000	    IN  OUT OUT OUT IN  IN  IN 	   			        
**						0X8000	    OUT IN  IN  IN  IN  IN  IN 	   			        
**						0X9000	    OUT IN 	IN  OUT IN  IN  IN 	   			        
**						0XA000	    OUT IN  OUT IN  IN  IN  IN 		   			    
**						0XB000	    OUT IN  OUT OUT IN  IN  IN 	   			        
**						0XC000	    OUT OUT IN  IN  IN  IN  IN 		   			    
**						0XD000	    OUT OUT IN  OUT IN  IN  IN 	   			        
**						0XE000	    OUT OUT OUT IN  IN  IN  IN 		   			    
**						0XF000	    OUT OUT	OUT OUT IN  IN  IN 	   			        
**
**		Jumper E1.A32 should be removed.
**
**
**		All interrupt jumpers are to be left at the Factory Default positions.
**					Module	-	Int0	Int1
**					  0			 1		 2
**					  1			 3		 4
**					  2			 5		 6
**					  3			 7		 0 (0 means not available)
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
**	Functions Included:
**
**				long IPIC_report(int level)
**					This function provides the reporting for the carrier
**					boards.  All of the information is provided for each
**					module in the carriers.  It is mapped into the EPICS
**					drivers calls.
**
**				const IPIC_Info *IPIC_get_IPID(void)
**					This function will be called by the init functions of
**					each module type.  When called, this function will fill
**					the device structures for all of the modules on all
**					carriers in the rack and return the pointer to the
**					beginning of the device array.  Any other calls to it
**					will return the pointer to the beginning of the device array.
**
** *****************************************************************************
**
**	  Date		Revisor	Ver #	Modification Description
**	________	_______	_____  	_________________________________________
**							   	 		
**	05/05/98	  GAD	1.0		Original code	
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


#include <vxWorks.h>
#include <sysLib.h>
#include <taskLib.h>
#include <iv.h>
#include <vme.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <drvGsIPIC.h>

#ifndef NO_DEBUG
int	verbose_IP_code = 0;
#endif

/* define a static array to hold the information for the IPs in an IPIC_Info*/
/* structure.  Make it large enough for 64 IPs.								*/
static IPIC_Info	device[64];

/* static variable to tell me that IPIC_get_IPID has been run before */
static int			getid=0;

/*
 *	EPICS driver entry point table
 */
struct {
	long	number;
	long	(* report)	(int level);
	long	(* init)	();
}	drvGsIPIC =	{
		1,
		IPIC_report,
		0
	};

/**/
/***********\
**
**  FUNCTION:       IPIC_report - EPICS driver report function
**
**  SYNTAX:         long IPIC_report(int type)
**
**  ARGUMENTS:      int type - included for EPICS compatiblity, not implemented
**
**  RETURN VALUES:  0 - (OK)
**
**  DESCRIPTION:    This function can be called from the VxWorks IOC prompt and
**                  as part of the EPICS driver-table call via dbior().  It
**					displays the IP modules installed and information about 
**					those modules.
**
\**********/
long IPIC_report (int type)	{
	int	carrier, mod;

	printf ("carrier module  base  int0 int1: local addr manuf model  rev driver \n");
	for (carrier=0; carrier<IPIC_NUM_CARRIERS; carrier++){
		for (mod=0; mod<4; mod++){
			if (device[(carrier*4)+mod].local){
				printf ("  %2d      %2d   0x%04X  %2d   %2d : 0x%X  0x%02X  0x%02X 0x%02X 0x%04X\n",
					carrier,mod, device[(carrier*4)+mod].base,
					device[(carrier*4)+mod].int_level0,
					device[(carrier*4)+mod].int_level1,
				   	device[(carrier*4)+mod].local,
				   	device[(carrier*4)+mod].manufacturer,
				   	device[(carrier*4)+mod].model,
				   	device[(carrier*4)+mod].revision,
				   	device[(carrier*4)+mod].driver);
			}
		}
	}

	return 0;
}

/**/
/***********\
**
**  FUNCTION:       IPIC_get_IPID - Identify IP modules on the IOC
**
**  SYNTAX:         const IPIC_Info *IPIC_get_IPID (void)
**
**  ARGUMENTS:      none
**
**  RETURN VALUES:  const IPIC_Info * - pointer to the beginning of the device
**										structure (device[0])
**									  -	0 for error
**
**  DESCRIPTION:    This function is called by every Greenspring IP module driver and is used
**                  to fill the IPIC_Info structures.  The first time that it is called it 
**                  will fill the structures and return a pointer to the first device structure.
**					Subsequent calls will only return a pointer to the first device structure.
**					This function can also be used in testing to fill the structures to see
**					what devices are in the IOC rack.
\**********/
const IPIC_Info *IPIC_get_IPID (void) {
	IPIC_Info		*info;
	IDPROM			*prom;
	const Byte		*busaddr,*address,*fulladdr,*busprom;
	Byte			b, l;
	int				carrier, mod, flag;

/*	Search for an IP module on every carrier card beginning at addess 0x0000
	for the first carrier to 0xF000 for the last carrier.
	for (address=(Byte *)0x0000, carrier=0 ; address<=(Byte *)0xF000 && getid==0 ; address += 0x1000) {
*/
	for (address=(Byte *)0x0000, carrier=0 ; address<=(Byte *)0x0000 && getid==0 ; address += 0x1000) {

	/*	There are 4 possible IP module addresses for each carrier 0xn100
		through 0xn300, where n=0-F.  Search through these possibilities
		and record the modules that are found
	*/
		for (fulladdr=address, mod=0, flag=0; fulladdr<=address + 0x0300 ; fulladdr += 0x0100, mod++) { 
/*
			if (OK != sysBusToLocalAdrs (VME_AM_USR_SHORT_IO,
											fulladdr,
											(char**)&busaddr)) {
				logMsg ("IPIC_get_IPID: sysBusToLocalAdrs failed converting 0x%X\n",
								fulladdr);
				return 0;
			}
*/
		        busaddr = fulladdr + 0xfff58000;

			busprom = (Byte *)(busaddr + 0x80);	/* Ptr to 1st byte of IDPROM */
			prom = (IDPROM *)busprom;	/* Cast to IDPROM struct pointer */

			if ( OK == vxMemProbe (busprom+1, READ, 1, &b)  &&  b == 'I') {
				if ( OK == vxMemProbe (busprom+3, READ, 1, &b)  &&  b == 'P') {
					if ( OK == vxMemProbe (busprom+5, READ, 1, &b)  &&  b == 'A') {
						if ( OK == vxMemProbe (busprom+7, READ, 1, &b)  &&  b == 'C') {
							flag=1;	/* set to 1 for the 1st module found on a carrier */

							/* set base address in device structure */
							device[(carrier*4)+mod].base = fulladdr;

							/* fill IPIC_Info structure using ptr - info */
							info = &device[(carrier*4)+mod];
							info->manufacturer = prom->manid;
							info->model = prom->model;
							info->revision = prom->rev;
							info->local = busaddr;

							b = prom->driverh;
							l = prom->driverl;
							info->driver = (b << 8)  | l;

							/*
								The module number determines the interrupt
								level used.  This matches the default
								jumper settings on the board.
							*/
							switch (mod) {
								case 0:
									info->int_level0 = 1;
									info->int_level1 = 2;
									break;
								case 1:
									info->int_level0 = 3;
									info->int_level1 = 4;
									break;
								case 2:
									info->int_level0 = 5;
									info->int_level1 = 6;
									break;
								case 3:
									info->int_level0 = 7;
									info->int_level1 = 0;
									break;
								default:
									info->int_level0 = 0;
									info->int_level1 = 0;
									break;
							}
						}
					}
				}
			}
		}
		if (flag != 0)		
			carrier++;		/* if a module was found increment carrier # */
	}
	getid=1;				/* let me know that this has been run */
	return &device[0];
}


/*	EOF drvGsIPIC.c */
