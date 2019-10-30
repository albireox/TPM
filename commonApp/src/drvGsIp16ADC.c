/*	drvGsIp16ADC.c
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

#include <vxWorks.h>
#include <sysLib.h>
#include <tickLib.h>
#include <stdio.h>
#include <vme.h>
#include <iv.h>
#include <assert.h>
#include <drvGsIp16ADC.h>

#ifndef PURE_VXWORKS_DRIVER
#include <devLib.h>
#include <drvSup.h>
#endif

#define DEBUG(anycode) /* */
#define L_DEBUG(lebel, anycode) /* */

#define IPC_NUM_IPS 4

static IP16ADC	*device[IPC_NUM_IPS];


static double IP16ADC_voltage (IP16ADC_V_range range, Word raw_value)
{
	switch (range)
	{
	case ip16_0_5:
			return raw_value * 5.0 / 0x10000;
	case ip16_0_10:
			return raw_value * 10.0 / 0x10000;
	case ip16_m5_5:
			return (raw_value * 10.0 / 0x10000) - 5.0;
	case ip16_m10_10:
			return (raw_value * 20.0 / 0x10000) - 10.0;
	}
	return 0.0;
}
	

static long report (int level)
{
	int	ip_num, j;

	for (ip_num=0; ip_num<IPC_NUM_IPS; ++ip_num)
	{
		if (device[ip_num])
		{
			printf ("IP #%d at 0x%04X\n",
				ip_num, (unsigned) device[ip_num]->regs);
			if (level > 3)
			{
				printf ("\tdata interrupt vector %d\n",
						device[ip_num]->data_vec);
				printf ("\tchange window vector %d\n",
						device[ip_num]->win_vec);

				printf ("\tmutex-semaphore id: 0x%X\n",
					(unsigned) device[ip_num]->mutex);
				printf ("\tnew-data-semaphore id: 0x%X\n",
					(unsigned) device[ip_num]->new_data);
				printf ("\tdriver task id: 0x%X\n",
					(unsigned) device[ip_num]->driver_task);
				printf ("\tdelay ticks: %d\n",
					device[ip_num]->delay_ticks);
				printf ("\tdriver sync flag: %d\n",
					device[ip_num]->sync_flag);
				
				switch (device[ip_num]->range)
				{
				case ip16_0_5:		printf ("\tinput range: 0..5 V\n");
									break;
				case ip16_0_10:		printf ("\tinput range: 0..10 V\n");
									break;
				case ip16_m5_5:		printf ("\tinput range: -5..5 V\n");
									break;
				case ip16_m10_10:	printf ("\tinput range: -10..10 V\n");
									break;
				}
			}
			if (level > 1)
			{
				for (j=0; j<IP16ADC_NUM_CHANNELS; ++j)
					printf ("\tch. %d data: 0x%04X = %+7.3f V\n",
						j, (unsigned) device[ip_num]->data[j],
						IP16ADC_voltage (device[ip_num]->range,
								 device[ip_num]->data[j]) );
			}
		}
	}

	return 0;
}

/*
 *	initiate a hardware calibration
 *
 *	This takes some seconds,
 *	routine is also called from IP16ADC_init
 */
int IP16ADC_calibrate (IP16ADC *ip)
{
	int		quartsec = sysClkRateGet() / 4;
	int		interval = 0;
#define PATIENCE	42 /* ! */

	if (semTake (ip->mutex, NO_WAIT) == ERROR)
	{
		logMsg ("IP16ADC_calibrate: mutex semaphore not available\n");
		return 1;
	}
	/*	issue "reset" signal for hardware calibration */
	ip->regs->ctrl_stat = ip->base_cmd & (~CS_NO_SWRESET);
	taskDelay (1);
	ip->regs->ctrl_stat = ip->base_cmd;

	L_DEBUG(1, printf ("IP16ADC_calibrate: hardware calibration "); )

	while ((ip->regs->ctrl_stat & CS_NOSTDBY) == 0)
	{
		++interval;
		if (interval > PATIENCE)
		{
			logMsg ("IP16ADC_calibrate: never ending calibration? I Quit.\n");
			logMsg ("IP16ADC_calibrate: Common cause: missing external voltage of +-15V!\n");
			semGive (ip->mutex);

			return 1;
		}
		taskDelay (quartsec);
	}
	L_DEBUG(1, printf ("ok after %g secs.\n", (double) interval/4.0); )

	ip->regs->ctrl_stat = ip->base_cmd;
	ip->regs->trigger = 0xFFFF;

	semGive (ip->mutex);

#undef PATIENCE
	return 0;
}

/*
 *	initiate read using interrupts
 *
 *	Note that this routine takes the mutex,
 *	which is given back in the last ISR.
 *	-> If no interrupts are generated,
 *	the mutex becomes unavailable.
 */
static int IP16ADC_IRQ_read (IP16ADC *ip)
{
	if (semTake (ip->mutex, NO_WAIT) == ERROR)
	{
		logMsg ("IP16ADC_IRQ_read: mutex semaphore not available\n");
		return 1;
	}
	L_DEBUG(10, logMsg("IP16ADC_IRQ_read\n"); )
	ip->channel			= 0;
	ip->channel_cmd		= ip->base_cmd | CS_DIFF_CHANNEL_SEL(ip->channel);

	ip->regs->vector	= ip->win_vec;
	ip->regs->ctrl_stat	= ip->channel_cmd | CS_CHGINTEN;
	ip->regs->trigger	= 0xFFFF;

	/* now we should get a 'change window' interrupt */

	return 0;
}

/*
 *	handle 'change window' interrupt
 */
static void ChgWin_ISR (void *arg)
{
	IP16ADC							*ip = (IP16ADC *) arg;
	register volatile IP16ADCregs	*regs = ip->regs;

	/* intr. ack. */
	regs->ctrl_stat = ip->channel_cmd;

	ip->read_count  = IP16ADC_READ_COUNT;

	L_DEBUG(20, logMsg("ChgWin_ISR: start read intrs.\n"); )

	/*	set new channel, enable data IRQ */
	regs->vector	= ip->data_vec;
	regs->ctrl_stat = ip->channel_cmd | CS_DATINTEN;
}

/*
 *	handle 'new data' interrupt
 */
static void NewData_ISR (void *arg)
{
	IP16ADC							*ip = (IP16ADC *) arg;
	register volatile IP16ADCregs	*regs = ip->regs;

	/* int ack. */
	regs->ctrl_stat = ip->channel_cmd;

	if (--ip->read_count > 0)
	{
		/*	read again to allow multiplexer to settle	*/
		regs->ctrl_stat = ip->channel_cmd | CS_DATINTEN;
		return;
	}

	/*	else: prepare next channel	*/
	ip->data[ip->channel] = regs->data;
	if (++ip->channel < IP16ADC_NUM_CHANNELS)
	{
		ip->channel_cmd = ip->base_cmd | CS_DIFF_CHANNEL_SEL(ip->channel);
		regs->vector	= ip->win_vec;
		regs->ctrl_stat = ip->channel_cmd | CS_CHGINTEN;
		L_DEBUG(20, logMsg("NewData_ISR: read, switch to channel %d\n",
			ip->channel);
		)
		return;
	}

	/* ok, ISR done */
	semGive (ip->mutex);
	semGive (ip->new_data);
	L_DEBUG(10, logMsg("NewData_ISR done\n"); )
}



/*
 *	task that calls IP16ADC_IRQ_read periodically
 */
static void IP16ADC_driver (IP16ADC *ip)
{
	unsigned long	start, end;
	int				elapsed;


	while (1)
	{
		start = tickGet();
		/* initiate a read cycle for all channels: */
		if (IP16ADC_IRQ_read (ip)) {
			exit (1); 
		}
		/* wait until this finishes: */
		if (semTake (ip->new_data, ip->delay_ticks) == ERROR)
		{
			logMsg ("IP16ADC_driver: timeout\n");
		}
		end = tickGet();
		elapsed = (end > start) ? end - start : 0;
		taskDelay (ip->delay_ticks - elapsed);
	}
}

/*
 *	Read data for given channel.
 *
 * 	If sync_flag is 0 and the driver task is not running, then
 *	restart it.
 *
 *	If sync_flag is not 0 then manually perform an IRQ_read.
 *
 *	If all cases, if max_ticks is less than 0 then just
 *	return the existing data (no convert).
 *
 *	result: 0 = OK, 1: restarted driver, 2: error
 */
int IP16ADC_get_channel (IP16ADC *ip, unsigned channel, Word *data, unsigned max_ticks)
{
	int	result = 0;
        static int mutex_errorcnt = 0;

	assert (channel < IP16ADC_NUM_CHANNELS);

	if (max_ticks >= 0 &&
	    ip->sync_flag == 0 && 
	    taskIdVerify (ip->driver_task) == ERROR)
	{
		L_DEBUG(2, logMsg ("IP16ADC_get_channel: restarting driver task\n"); )
		
		if (ip->delay_ticks > max_ticks   &&   max_ticks > 0)
			ip->delay_ticks = max_ticks;
		ip->driver_task = taskSpawn (IP16ADC_NAME, IP16ADC_PRI,
			IP16ADC_OPT, IP16ADC_STK,
			(FUNCPTR) IP16ADC_driver, 
			(int) ip, 0, 0, 0, 0, 
			0, 0, 0, 0, 0);
		logMsg("taskSpawn return = %lx\n", ip->driver_task);
		logMsg("taskSpawn errno = %d\n", errno);

		result = 1;
		if (ip->driver_task == ERROR) {
			return 2;
		}
	}


	/*
	* Force a read in the sync_flag is set.
	*/
        if (max_ticks >= 0 && ip->sync_flag != 0) 
		IP16ADC_IRQ_read(ip);

	/*	Wait (2 ticks should be permissible) until driver delivers new data.
	 *	This should, however, only be necess. if the driver was _just_ restarted.
	 */
	    if (semTake (ip->mutex, 2) == ERROR)
	    {
		logMsg ("IP16ADC_get_channel: mutex semaphore not available\n");
		logMsg ("a common cause: device cannot generate interrupts, check for conflicts!\n");

	        if (++mutex_errorcnt > 10) {
		    mutex_errorcnt = 0;
		    logMsg ("IP16ADC_get_channel: Got 10 mutex errors\n");
		    logMsg ("Clearing mutex to free up window change/new data logic.\n");
		    semGive(ip->mutex);
		}
	
		return 2;
	    }

	/*	adjust delay if record is faster than we are now */
	if (ip->delay_ticks > max_ticks && max_ticks > 0)
		ip->delay_ticks = max_ticks;

	*data = ip->data[channel];
	semGive (ip->mutex);

	return result;
}


/*
 *	initialize IP-16ADC
 *
 *	ip_num: ip_num number for drvGsIPIC
 *	
 *	returns 0 on error
 */
IP16ADC *IP16ADC_init (unsigned ip_num, IP16ADC_V_range range, int sync_flag)
{
	IP16ADC			*ip;
	int 			status;

	if (ip_num > IPC_NUM_IPS)
	{
		logMsg ("IP16ADC_init: ip_num number not in 0..%d\n", IPC_NUM_IPS);
		return 0;
	}

	if (! device[ip_num])	/* first call for this ip_num ? */
	{
		if ((status = ipmValidate(0, ip_num, 0xF0, 0x36)) != OK)
		{
			logMsg ("IP16ADC_init: ipmValidate returned %lx\n", status);
			logMsg ("IP16ADC_init: IP #%d is no Greenspring IP-16ADC\n", ip_num);
			return 0;
		}

		L_DEBUG(1,
		printf ("IP16ADC_init: found Greenspring IP-16ADC at 0x%X\n",
			(unsigned) info.local);
		)

		if (! (ip = (IP16ADC *) calloc (1, sizeof (IP16ADC))))
		{
			logMsg ("IP16ADC_init: calloc (IP16ADC) failed\n");
			return 0;
		}

		if (! (ip->mutex = semBCreate (SEM_Q_PRIORITY, SEM_FULL)))
		{
			logMsg ("IP16ADC_init: cannot create 'mutex' semaphore\n");
			return 0;
		}

		if (! (ip->new_data = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY)))
		{
			logMsg ("IP16ADC_init: cannot create 'new_data' semaphore\n");
			return 0;
		}
		ip->regs        = (IP16ADCregs *) ipmBaseAddr(0, ip_num, ipac_addrIO);
		ip->base_cmd    = CS_NO_SWRESET | CS_DIFF_CHANNEL_SEL(0);
		ip->range      	= range;
		ip->delay_ticks = sysClkRateGet() * 1; /* 1 secs */
		ip->driver_task = 0;
		ip->sync_flag 	= sync_flag;

		switch (ip->range)
		{
		case ip16_0_5:		ip->base_cmd |= CS_0_5V;
							break;
		case ip16_0_10:		ip->base_cmd |= CS_0_10V;
							break;
		case ip16_m5_5:		ip->base_cmd |= CS_5_5V;
							break;
		case ip16_m10_10:	ip->base_cmd |= CS_10_10V;
							break;
		}

		if (IP16ADC_calibrate (ip))
		{
			logMsg ("IP16ADC_init: cannot calibrate, this is fatal\n");
			return 0;
		}
		if (ipmIntConnect(0, ip_num, 70 + 2*ip_num,
			NewData_ISR, ip) != OK)
		{
			logMsg ("IP16ADC_init: cannot connect ISR\n");
			return 0;
		} 
		ip->data_vec = 70 + 2*ip_num;

		if (ipmIntConnect(0, ip_num, 70 + 2*ip_num + 1,
			ChgWin_ISR, ip) != OK)
		{
			logMsg ("IP16ADC_init: cannot connect ISR\n");
			return 0;
		} 
		ip->win_vec = 70 + 2*ip_num +1;

		if (ipmIrqCmd(0, ip_num, 0, ipac_irqEnable) != OK)
		{
			logMsg ("IP16ADC_init: cannot enable int0\n");
			return 0;
		}
		if (ipmIrqCmd(0, ip_num, 0, ipac_irqLevel3) != OK)
		{
			logMsg ("IP16ADC_init: cannot set int0 to level3\n");
			return 0;
		}
		logMsg("level 0 = %d\n", ipmIrqCmd(0, ip_num, 0, ipac_irqGetLevel));
		if (ipmIrqCmd(0, ip_num, 1, ipac_irqEnable) != OK)
		{
			logMsg ("IP16ADC_init: cannot enable int1\n");
			return 0;
		}
		if (ipmIrqCmd(0, ip_num, 1, ipac_irqLevel4) != OK)
		{
			logMsg ("IP16ADC_init: cannot set int1 to level4\n");
			return 0;
		}
		logMsg("level 1 = %d\n", ipmIrqCmd(0, ip_num, 1, ipac_irqGetLevel));
		device[ip_num] = ip;
	}
	else	/* re-initialisation of same IP */
	{
		DEBUG(
		printf ("IP16ADC_init: ip_num #%d is already init.\n", ip_num);
		)
		if (range != device[ip_num]->range)
		{
			logMsg ("IP16ADC_init: different range for initialized IP #%d requested",
				ip_num);
			return 0;
		}
	}

	/*	setup hardware with interrupts disabled, continuous mode,
	 *	driver task will be (re)started on read ...
	 */
	device[ip_num]->regs->ctrl_stat = device[ip_num]->base_cmd;
	device[ip_num]->regs->trigger = 0xFFFF;

	return device[ip_num];
}

/* driver DSET */
struct
{
	long	number;
	long	(* report) (int level);
	long	(* init) ();
}	drvGsIp16ADC =
{
	2,
	report,
	NULL
};


#ifdef PURE_VXWORKS_DRIVER
int IP16ADC_report (int level)
{
	return report (level);
}

int IP16ADC_read (IP16ADC *ip)          
{
     	return IP16ADC_IRQ_read(ip);
}
#endif

/*	EOF drvGsIp16ADC.c */
