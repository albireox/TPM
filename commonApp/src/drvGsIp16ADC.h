/*	drvGsIp16ADC.h
 *
 *		GreenSpring IP-16ADC driver
 *
 *      Original Author: Kay-Uwe Kasemir
 *      Date:            4-9-96
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1996, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  04-09-96        kuk		--
 * .02  11-20-96        kuk		PC port
 *      ...
 */

#include <semLib.h>
#include <taskLib.h>
#include "drvIpac.h"
#include "ipic.h"

/*
 *	Define PURE_VXWORKS_DRIVER if you want to test
 *	the driver without EPICS, i.e. without devRegsiterAddress() etc.
 */
#define PURE_VXWORKS_DRIVER


#ifndef PURE_VXWORKS_DRIVER
#include <task_params.h>
#endif

/*
 *	If the driver's task parameters are not already defined
 *	in <task_params.h>, define them here:
 */
#ifndef IP16ADC_NAME

#define IP16ADC_NAME		"drvIp16ADC"
#define IP16ADC_PRI			32  /* IP16-ADC driver task - interrupt */
#define IP16ADC_OPT			VX_FP_TASK | VX_STDIO
#define IP16ADC_STK			2000

#endif

/*
 *	IP-16 ADC register map
 *
 *	because there is a byte register, this structure
 *	is different for PC or VME bus!
 */

typedef unsigned char Byte;
typedef unsigned short Word;

typedef struct
{
/* $00 */	Word			ctrl_stat;	/* R/W, Control and Status */
/* $02 */	Byte pad0[2];
/* $04 */	Word			data;		/* R,   A/D data           */
/* $06 */	Byte pad1[2];

#ifdef PC_VERSION
/* $08 */	Byte			vector;		/* R/W, Vector             */
/* $09 */	Byte pad2;
#else
/* $08 */	Byte pad2;
/* $09 */	Byte			vector;		/* R/W, Vector             */
#endif

/* $0A */	Byte pad3[2];
/* $0C */	Word			trigger;	/* W,   Trigger            */
}	IP16ADCregs;

typedef enum
{
	ip16_0_5,
	ip16_0_10,
	ip16_m5_5,		/* -5 .. 5 V */
	ip16_m10_10
}	IP16ADC_V_range;

typedef void (* ISR_DONE_FUNC) (void *ISR_done_arg, int channel, Word data);

#define IP16ADC_NUM_CHANNELS	8
int IP16ADC_READ_COUNT=2;	

/*
 *	information for one single IP module,
 *	internal driver usage only
 */
typedef struct
{
	SEM_ID					mutex;			/* allow ony mutual excl. access */
	Byte					data_vec;		/* new data intr. vector */
	Byte					win_vec;		/* chg. wind. intr. vector */
	int						driver_task;	/* driver's task id */
	int						delay_ticks;
	IP16ADC_V_range			range;
	int						channel;		/* ISR: active channel */
	int						read_count;		/* ISR: read counter   */
	Word					base_cmd;		/* ISR: command register bits	*/
	SEM_ID					new_data;		/* ISR: all data updated! */
	Word					channel_cmd;
	Word					data[IP16ADC_NUM_CHANNELS];
	volatile IP16ADCregs	*regs;
	int 					sync_flag;	/* Force a read on a get_channel call. */
}	IP16ADC;

/*
 *	Control and Status Bits
 */
#define CS_CHGINTEN			(1 << 15)
#define CS_DATINTEN			(1 << 14)
#define CS_NO_SWRESET		(1 << 13)
#define CS_NOSTDBY			(1 << 13)
#define CS_SDL				(1 << 12)
#define CS_NO_CHGINTRQ		(1 << 11)
#define CS_NO_DATINTRQ		(1 << 10)
#define CS_NO_FREERUN		(1 <<  8)
#define CS_BIPOLAR			(1 <<  6)
#define CS_UNITY_GAIN		(1 <<  5)
#define CS_INSL1			(1 <<  4)
#define CS_INSL0			(1 <<  3)

/*	conversion-begun,you-may-change-input interrupt: */
#define CHG_IRQ_IS_PENDING(statreg)	(((statreg) & CS_NO_CHGINTRQ) == 0)

/*	data-available interrupt: */
#define DAT_IRQ_IS_PENDING(statreg)	(((statreg) & CS_NO_DATINTRQ) == 0)

/*	input range */
#define CS_10_10V			CS_BIPOLAR
#define CS_0_10V			0
#define CS_5_5V				(CS_BIPOLAR | CS_UNITY_GAIN)
#define CS_0_5V				CS_UNITY_GAIN

/*	input channel select: c must be in [0..7] */
#define CS_DIFF_CHANNEL_SEL(c)	(CS_INSL1 | CS_INSL0 | (c))
#define CS_GROUND_CALIB_SEL		0
#define CS_VREF_CALIB_SEL		1

/*
 *	initialize IP-16ADC
 *
 *	card: card number for drvGsIPIC
 *
 *	sync_flag: 1 = call IP16ADC_IRQ_read for every get_channel,
 *		   0 = use driver() task mechanism.
 *	
 *	returns 0 on error
 */
IP16ADC *IP16ADC_init (unsigned card, IP16ADC_V_range range, int sync_flag);

/*
 *	initiate a hardware calibration
 *
 *	This takes some seconds,
 *	this routine is also called from IP16ADC_init
 */
int IP16ADC_calibrate (IP16ADC *ip);

/*
 *	Read data for given channel.
 *	If driver task is not running, restart it.
 *
 *	result: 0 = OK, 1: restarted driver, 2: error
 */
int IP16ADC_get_channel (IP16ADC *ip, unsigned channel, Word *data, unsigned max_ticks);

/*	EOF drvGsIp16ADC.h */
