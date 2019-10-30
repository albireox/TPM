#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "ip470.h"
#include <vxWorks.h>

/* 
    To demonstrate the interrupt capability of the IP470, it is necessary to
    supply the demo program with a trap number that is not being used by the
    system.  Trap numbers 0,13 and 15 are reserved by OS-9 and should not be
    used.  The trap number is used temporarily when attaching and detaching
    the interrupt service routine in the demo program.
*/

#define TRAPNUM 6



/*
{+D}
    SYSTEM:         Software

    FILENAME:       drvr470.c

    MODULE NAME:    main - main routine of example software.

    VERSION:        A

    CREATION DATE:  10/16/95

    DESIGNED BY:    FJM

    CODED BY:       FJM

    ABSTRACT:       This module is the main routine for the example program
		    which demonstrates how the IP470 Library is used.

    CALLING
	SEQUENCE:   

    MODULE TYPE:    void

    I/O RESOURCES:  

    SYSTEM
	RESOURCES:  

    MODULES
	CALLED:     

    REVISIONS:      

  DATE    BY        PURPOSE
-------  ----   ------------------------------------------------


{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

	This module is the main routine for the example program
	which demonstrates how the IP470 Library is used.
*/




void ip470_test()
{


/*
    DECLARE EXTERNAL DATA AREAS:
*/

/*
    DECLARE LOCAL DATA AREAS:
*/

    char cmd_buff[40];  /* command line input buffer */
    int item;           /* menu item selection variable */
    long status;        /* returned status of driver routines */
    int hstatus;        /* interrupt handler returned status */
    unsigned finished;  /* flag to exit program */
    unsigned finished2; /* flag to exit a loop */
    unsigned int addr;  /* long to hold board address */
    int flag;           /* general flag for exiting loops */
    int i;              /* loop index */
    int point;          /* I/O point number */
    int port;           /* I/O port number */
    int val;            /* value of port or point */
    int hflag;          /* interrupt handler installed flag */
    struct cblk470 c_block; /* configuration block */
    struct handler_data hdata;  /* interrupt handler data structure
				   (see exception.h) */

/*
    DECLARE MODULES CALLED:
*/

/*
    Carrier.c - configures the Acromag carrier board to cause interrupts.
    Other carrier boards may require different configurations to cause
    interrupts.  The user should make the required modifications in "carrier.c"
    and in the interrupt service routines, if needed.
*/
    
    void carrier();     /* routine to set up the Acromag carrier board */
    void scfg470();     /* routine to set up the configuration param. block */
    void psts470();     /* routine which calls the Read Status Command */
    void cnfg470();     /* DRIVER - routine to configure the board */
    void rsts470();     /* DRIVER - routine to read status information */
    long rpnt470();     /* DRIVER - routine to read a input point */
    long rprt470();     /* DRIVER - routine to read the input port */
    long wpnt470();     /* DRIVER - routine to write to a output point */
    long wprt470();     /* DRIVER - routine to write to the output port */
    
/*
    declare interrupt handlers
*/

    void cos();
    void lev();
    int attach_ihandler();  /* attach interrupt handler */
    int detach_ihandler();  /* detach interrupt handler */

/*
    ENTRY POINT OF ROUTINE:
    INITIALIZATION
*/

    flag = 0;         /* indicate board address not yet assigned */
    finished = 0;     /* indicate not finished with program */
    hflag = 0;        /* indicate interrupt handler not installed yet */

/*
    Initialize the Configuration Parameter Block to default values.
*/

    c_block.param = 0;          /* parameter mask */
    c_block.e_mode = 0;         /* e_mode flag */
    c_block.mask_reg = 0;       /* output mask register */
    c_block.deb_control = 0;    /* debounce control register */
    c_block.deb_clock = 1;      /* debounce control register */
    c_block.enable = 0;         /* interrupt enable register */
    c_block.vector = 176;       /* interrupt vector base */
    
    for(i = 0; i != 2; i++)
    {
      c_block.ev_control[i] = 0;/* event control registers */
      c_block.deb_duration[i] = 0;/* debounce duration registers */
    }

    c_block.last_chan = 0;      /* init last interrupt channel */
    c_block.last_state = 0;     /* init last interrupt state */
    
/*
    Enter main loop
*/      

    while(!finished) {

        printf("\n\nIP470 Library Demonstration Version A\n\n");

	printf(" 1. Exit this Program\n");
	printf(" 2. Set IP Module Base Address\n");
        printf(" 3. Set Up Configuration Parameters\n");
	printf(" 4. Configure Board Command\n");
	printf(" 5. Read Status Command and ID\n");
	printf(" 6. Attach Exception Handler\n");
	printf(" 7. Detach Exception Handler\n");
	printf(" 8. Read Input Point\n");
	printf(" 9. Read Input Port\n");
	printf("10. Write Output Point\n");
	printf("11. Write Output Port\n");

	printf("\nSelect: ");
	scanf("%d",&item);

/*
    perform the menu item selected.
*/  
	switch(item) {

	case 1: /* exit program command */

	    printf("Exit program(y/n)?: ");
	    scanf("%s",cmd_buff);
	    if( cmd_buff[0] == 'y' || cmd_buff[0] == 'Y' )
		finished++;
	    break;
	
	case 2: /* set IP address command */

	    do {
		if(flag == 0)
		{
		    printf("\n\nenter base address of IP board in hex: ");
		    scanf("%x",&addr);
		}
		printf("address: %x\n",addr);
		printf("is this value correct(y/n)?: ");
		scanf("%s",cmd_buff);
		if( cmd_buff[0] == 'y' || cmd_buff[0] == 'Y' )
		{
/*
    Now store this address in the Configuration Block
    This address will now be universally used when reading and writing
    the board's registers.
*/

		    c_block.brd_ptr = (struct map470 *)addr;
                    hdata.hd_ptr = (char *)&c_block;
                    
		    i = ((addr >> 8) & 3);      /* form IP position */
		    switch(i)           /* convert to IP under service code */
		    {
			case 1:
                                c_block.ip_pos = 1;   /* IPB0 under service */
			break;

			case 2:
                                c_block.ip_pos = 2;   /* IPC0 under service */
			break;

			case 3:
                                c_block.ip_pos = 3;   /* IPD0 under service */
			break;

			default:
                                c_block.ip_pos = 0;   /* IPA0 under service */
			break;
		    }
		    flag = 1;
		}
		else
		    flag = 0;
		
	    }while( cmd_buff[0] != 'y' && cmd_buff[0] != 'Y' );
	    break;

	case 3: /* set up configuration parameters */

            /*
             * Fix bug in demo as delivered from Acromag by not allowing
             * reconfiguration with interrupt handler attached.
             * Without this prohibition a user could attach at one vector,
             * change c_block.vector by reconfiguring, then do a detach
             * on the new vector.  Thus detaching the existing interrupt
             * handler at the new vector instead of the correct interrupt
             * handler at the old vector.
             */
	    if(hflag)
	       printf("Reconfiguration not allowed with interrupt handlers "
	          "attached.\n");
	    else
               scfg470(&c_block);
	    break;

	case 4: /* configure board command */
	
	    if(flag == 0)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
/*
    Check the "interrupt handler attached" flag. If interrupt handlers
    are not attached, then print an error message.
*/

              if( c_block.enable == 1 && hflag == 0 )
		   printf(">>> ERROR: INTERRUPT HANDLER NOT ATTACHED <<<\n");
	       else  /* select enhanced mode & configure */
		   cnfg470(&c_block); /* configure the board */
	    }
	    break;

	case 5:     /* read board status command */
	
	    if(flag == 0)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
		psts470(&c_block); /* read board status */
	    break;

	case 6:     /* attach exception handler */
	    finished2 = 0;
	    while(!finished2)
	    {
	      printf("\n\nAttach Exception Handler\n\n");
	      printf("1. Return to Previous Menu\n");
	      printf("2. Attach Handler for Change of State\n");
              printf("3. Attach Handler for Level Interrupts\n");
	      printf("\nSelect: ");
	      scanf("%x",&item);

	      switch(item){
		
	      case 1:   /* return to previous menu */
		finished2++;
		break;

	      case 2:   /* attach handler for Change of State Method */

	       if(hflag)
		 printf("\n>>> ERROR: HANDLERS ARE ATTACHED <<<\n");
	       else
	       {

/*
    attach a common handler for each point
*/

		hstatus = attach_ihandler(TRAPNUM,c_block.vector,0,cos,&hdata);

/*
    If there were any errors, then detach the handler and print error
    messages.  If no errors, then set "handler attached" flag.
*/
		if(hstatus)
		{
		  detach_ihandler(TRAPNUM,c_block.vector,&hdata);
		  printf(">>> ERROR: %d WHEN ATTACHING COS HANDLER<<<\n",hstatus);
		}
		else
		{
/*
    Carrier.c - configures the Acromag carrier board to cause interrupts.
    Other carrier boards may require different configurations to cause
    interrupts.  The user should make the required modifications in "carrier.c"
    and in the interrupt service routines, if needed.
*/
		  carrier( addr );
		  hflag = 1;
		  printf("Handlers are now attached\n");
		}
	       } /* end of if/else */
	       break;

              case 3:   /* attach handler for Level Interrupts */

	       if(hflag)
		 printf("\n>>> ERROR: HANDLERS ARE ATTACHED <<<\n");
	       else
	       {

/*
    attach a common handler for each point
*/

                hstatus = attach_ihandler(TRAPNUM,c_block.vector,0,lev,&hdata);

/*
    If there were any errors, then detach the handler and print error
    messages.  If no errors, then set "handler attached" flag.
*/
		if(hstatus)
		{
		  detach_ihandler(TRAPNUM,c_block.vector,&hdata);
                  printf(">>> ERROR: %d WHEN ATTACHING LEV HANDLER<<<\n",hstatus);
		}
		else
		{
/*
    Carrier.c - configures the Acromag carrier board to cause interrupts.
    Other carrier boards may require different configurations to cause
    interrupts.  The user should make the required modifications in "carrier.c"
    and in the interrupt service routines, if needed.
*/
		  carrier( addr );
		  hflag = 1;
		  printf("Handlers are now attached\n");
		}
	       } /* end of if/else */
	       break;
	      } /* end of switch */
	    } /* end of while */
	    break;

	case 7: /* detach exception handlers */

	 if(!hflag)
	   printf(">>> ERROR: HANDLERS ALREADY DETACHED <<<\n");
	 else
	 {
	   hflag = 0;
	   hstatus = detach_ihandler(TRAPNUM,c_block.vector,&hdata);
/*
    If no errors then say " . . now detached ..", otherwise report errors.
*/
	   if(hstatus == 0)
	     printf("Handlers are now detached\n");
	   else
	     printf(">>> ERROR: %d WHEN DETACHING HANDLER <<<\n",hstatus);
	 }
	 break;

	 case 8: /* Read Digital Input Point */

	    if(flag == 0)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter Input port number   (0 - 5): ");
		scanf("%d",&port);
		printf("\nEnter Input point number  (0 - 7): ");
		scanf("%d",&point);
		status = rpnt470(&c_block,port,point);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
		else
		    printf("\nValue of port %d point %d: %x\n",port,point,status);
	    }
	    break;

	case 9: /* Read Digital Input Port */

	    if(flag == 0)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter Input port number  (0 - 5): ");
		scanf("%d",&port);
		status = rprt470(&c_block,port);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
		else
		    printf("\nValue of port %d: %x\n",port,status);
	    }
	    break;

	case 10: /* Write Digital Point */

	    if(flag == 0)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter Output port number (0 - 5): ");
		scanf("%d",&port);
		printf("\nEnter I/O point number   (0 - 7): ");
		scanf("%d",&point);
		printf("\nEnter point value (0 - 1): ");
		scanf("%x",&val);
		status = wpnt470(&c_block,port,point,val);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
	    }
	    break;

	case 11: /* Write Digital Port */

	    if(flag == 0)
		printf("\n>>> ERROR: BOARD ADDRESS NOT SET <<<\n");
	    else
	    {
		printf("\nEnter Output port number  (0 - 5):  ");
		scanf("%d",&port);
		printf("\nEnter output value in hex (0 - FF): ");
		scanf("%x",&val);
		status = wprt470(&c_block,port,val);
		if(status == -1)
		    printf("\n>>> ERROR: PARAMETER OUT OF RANGE <<<\n");
	    }
	    break;
	}   /* end of switch */
    }   /* end of while */

/*
    disable interrupts from IP module
*/
    if(flag != 0)                 /* module address was set */
    {
      c_block.param = RESET_INTEN; /* parameter mask */
      c_block.enable = 0;
      c_block.enable |= RESET;
      cnfg470(&c_block);          /* configure the board */
    }

/*
    if a handler is installed, then detach it.
*/
    if(hflag != 0)
      detach_ihandler(TRAPNUM,c_block.vector,&hdata);

    printf("\nEXIT PROGRAM\n");
}   /* end of main */



/*
{+D}
    SYSTEM:         Software

    FILENAME:       drvr470.c

    MODULE NAME:    get_param - get a parameter from the console

    VERSION:        A

    CREATION DATE:  10/16/95

    DESIGNED BY:    RTL

    CODED BY:       RTL

    ABSTRACT:       Routine which is used to get parameters

    CALLING
	SEQUENCE:   get_param();

    MODULE TYPE:    long

    I/O RESOURCES:  

    SYSTEM
	RESOURCES:  

    MODULES
	CALLED:     

    REVISIONS:      

  DATE    BY        PURPOSE
-------  ----   ------------------------------------------------

{-D}
*/


long get_param()
{

/*
    declare local storage.
*/

    long temp;

    printf("enter hex parameter: ");
    scanf("%x",&temp);
    printf("\n");
    return(temp);
}


/*
{+D}
    SYSTEM:         Software

    FILENAME:       drvr470.c

    MODULE NAME:    scfg470 - set configuration block contents.

    VERSION:        A

    CREATION DATE:  10/16/95

    DESIGNED BY:    FJM

    CODED BY:       FJM

    ABSTRACT:       Routine which is used to enter parameters into
		    the Configuration Block.
 
    CALLING
	SEQUENCE:   scfg470(c_block)
		    where:
			c_block (structure pointer)
			  The address of the configuration param. block

    MODULE TYPE:    void

    I/O RESOURCES:  

    SYSTEM
	RESOURCES:  

    MODULES
	CALLED:     get_param()     input a parameter from console

    REVISIONS:      

  DATE    BY        PURPOSE
-------  ----   ------------------------------------------------

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:
*/

void scfg470(c_blk)

struct cblk470 *c_blk;
{

/*
    DEFINITIONS:
*/


/*
    DECLARE EXTERNAL DATA AREAS:
*/



/*
    DECLARE LOCAL DATA AREAS:
*/
    int item;                       /* menu item variable */
    unsigned finished,finished2;    /* flags to exit loops */
    int i, j;                       /* loop index */

/*
    DECLARE MODULES CALLED
*/
    long get_param();   /* input a parameter */
    
/*
    ENTRY POINT OF ROUTINE:
*/
    finished = 0;
    while(!finished)
    {
        printf("\n\nConfiguration Parameters\n\n");

	printf(" 1. Return to Previous Menu\n");
	printf(" 2. Board Pointer:    %04x\n",c_blk->brd_ptr);
        printf(" 3. Parameter Mask:     %02x\n",c_blk->param);
        printf(" 4. Enhanced Mode Flag: %02x\n",c_blk->e_mode);
        printf(" 5. Write Mask:         %02x",c_blk->mask_reg);
        printf("\n 6. Event Control:      ");
	for(i = 0; i != 2; i++)
	   printf("%02x  ",c_blk->ev_control[i]); /* event control registers */
        printf("\n 7. Debounce Clock:     %02x",c_blk->deb_clock);
        printf("\n 8. Debounce Control:   %02x",c_blk->deb_control);
        printf("\n 9. Debounce Duration:  ");
	for(i = 0; i != 2; i++)
	   printf("%02x  ",c_blk->deb_duration[i]); /* debounce duration registers */
        printf("\n10. Reset/Int. Enable:  %02x",c_blk->enable);
        printf("\n11. Interrupt Vector:   %02x\n",c_blk->vector);
    
	printf("\nSelect: ");
	fflush(stdout);
	scanf("%d",&item);

	switch(item)
	{
	
	case 1: /* return to previous menu */
	    finished++;
	    break;

	case 2: /* board address */
	    printf("BOARD ADDRESS CAN BE CHANGED ONLY IN THE MAIN MENU\n");
	    break;

        case 3: /* parameter mask */
	    c_blk->param = (word)get_param();
	    break;

        case 4: /* enhanced mode */
            c_blk->e_mode = (word)get_param();
	    break;

        case 5: /* write mask */
	    c_blk->mask_reg = (byte)get_param();
	break;

        case 6: /* event control */
	    finished2 = 0;
	    while( !finished2 )
	    {
	      printf("\nSet Up Event Control Array\n\n");
	      printf("1. Return to Previous Menu\n");
	      for(i = 0; i < 2; i++)
	      {
		printf("%d. Event Control Code for Array Index %d:%02x\n",
		   (i + 2),i,c_blk->ev_control[i]);
	      }
	      printf("\nselect: ");
	      scanf("%d",&item);
	      switch(item)
	      {
	       case 1: /* return to previous menu */
		    finished2++;
	       break;
		    
	       default:
		    if(item > 1 && item < 4)
		    {
		       printf("\nControl code - ");
		       c_blk->ev_control[item - 2] = (byte)get_param();
		    }
	       break;
	      }
	    }
	break;

        case 7: /* debounce clock */
	    c_blk->deb_clock = (byte)get_param();
	break;

        case 8: /* debounce control */
	    c_blk->deb_control = (byte)get_param();
	break;

        case 9: /* debounce duration */
	    finished2 = 0;
	    while( !finished2 )
	    {
	      printf("\nSet Up Debounce Duration Array\n\n");
	      printf("1. Return to Previous Menu\n");
	      for(i = 0; i < 2; i++)
	      {
		printf("%d. Debounce Duration Code for Array Index %d:%02x\n",
		   (i + 2),i,c_blk->deb_duration[i]);
	      }
	      printf("\nselect: ");
	      scanf("%d",&item);
	      switch(item)
	      {
	       case 1: /* return to previous menu */
		    finished2++;
	       break;
		    
	       default:
		    if(item > 1 && item < 4)
		    {
		       printf("\nDebounce Duration code - ");
		       c_blk->deb_duration[item - 2] = (byte)get_param();
		    }
	       break;
	      }
	    }
	break;

        case 10: /* interrupt enable reg. */
	    c_blk->enable = (byte)get_param();
	    break;

        case 11: /* exception vector number */
	    c_blk->vector = (byte)get_param();
	    break;
	}
    }
}



/*
{+D}
    SYSTEM:         Software

    FILENAME:       drvr470.c

    MODULE NAME:    psts470 - print board status information

    VERSION:        A

    CREATION DATE:  10/16/95

    DESIGNED BY:    FJM

    CODED BY:       FJM

    ABSTRACT:       Routine which is used to cause the "Read Board Status"
		    command to be executed and to print out the results to
		    the console.

    CALLING
	SEQUENCE:   psts470(&c_block)
		    where:
			c_block (structure pointer)
			  The address of the configuration param. block
			 
    MODULE TYPE:    void
    
    I/O RESOURCES:  
    
    SYSTEM
	RESOURCES:  
	
    MODULES
	CALLED:     rsts470()      Read Board Status Command.

    REVISIONS:      
    
  DATE    BY        PURPOSE
-------  ----   ------------------------------------------------

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:
    
*/

void psts470(c_blk)
struct cblk470 *c_blk;
{

/*
    DEFINITIONS:
*/


/*
    DECLARE LOCAL DATA AREAS:
*/
    int item;           /* menu item variable */
    unsigned finished;  /* flags to exit loops */
    int i;

/*
    DECLARE MODULES CALLED
*/
    void rsts470();    /* Read Board Status Command */

/*
    ENTRY POINT OF ROUTINE:
*/
    finished = 0;
    while(!finished)
    {
	rmid470(c_blk); /* get board's ID info into structure */
	rsts470(c_blk); /* get board's status info into structure */
	printf("\n\nBoard Status Information\n");
        printf("\nInterrupt Enable Register:   %02x",c_blk->enable);
	printf("\nInterrupt Vector Register:   %02x",c_blk->vector);
        printf("\nLast Interrupting Channel:   %02x",c_blk->last_chan);
        printf("\nLast Interrupting State:     %02x",c_blk->last_state);
	
	printf("\n\nBoard ID Information\n");
	printf("\nIdentification:              ");
	for(i = 0; i < 4; i++)          /* identification */
	   printf("%c",c_blk->id_prom[i]);
	printf("\nManufacturer's ID:           %x",(byte)c_blk->id_prom[4]);
	printf("\nIP Model Number:             %x",(byte)c_blk->id_prom[5]);
	printf("\nRevision:                    %x",(byte)c_blk->id_prom[6]);
	printf("\nReserved:                    %x",(byte)c_blk->id_prom[7]);
	printf("\nDriver I.D. (low):           %x",(byte)c_blk->id_prom[8]);
	printf("\nDriver I.D. (high):          %x",(byte)c_blk->id_prom[9]);
	printf("\nTotal I.D. Bytes:            %x",(byte)c_blk->id_prom[10]);
	printf("\nCRC:                         %x",(byte)c_blk->id_prom[11]);

	printf("\n\n1. Return to Previous Menu");
	printf("\n2. Read Status Again\n\nselect: ");
	fflush(stdout);
	scanf("%d",&item);

	switch(item){

	case 1: /* return to previous menu */
	    finished++;
	    break;
	}
    }
}
