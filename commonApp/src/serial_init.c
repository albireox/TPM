/*=============================================================================
 SERIAL_INIT.c 
=============================================================================*/
#include <vxWorks.h>
#include <taskLib.h>
#include <tickLib.h>
#include <stdio.h>
#include <errnoLib.h>
#include <rebootLib.h>
#include <mv162IndPackInit.h>
#include <ipOctalSerial.h>

static OCT_SER_DEV *pOctSerDv;

static void serialInit(unsigned char*, unsigned short, unsigned int, short);

/*
  IpDevCreate 

  Initializes the TPM IpOctalSerial communications.
-----------------------------------------------------------------------------*/
void 
IpDevCreate(void)
{
  /* Initialize the octal serial card. */

  serialInit((unsigned char*) 0xfff58000, 0xf022, 0xAA, 2);

}
static void 
serialRebootHook(int type)
{
    printf("serialRebootHook\r\n");
    octSerHrdInit(pOctSerDv);
}

static void serialInit(unsigned char *ip_base, unsigned short model, 
		       unsigned int vec, short ch)
{
  extern int VME162_IP_Memory_Enable(unsigned char*, int, char*);
  extern int VME_IP_Interrupt_Enable(unsigned char*, int);
  extern int sysIntEnable(int);
  static int serialInitDone = 0;
  struct IPACK ip;
  int ii;

  if (serialInitDone)
    return;
  serialInitDone = 1;

  VME162_IP_Memory_Enable((unsigned char*) 0xfff58000, 3, (char*) 0x72000000);

  Industry_Pack(ip_base, model, &ip);

  for (ii = 0; ii < MAX_SLOTS; ++ii) {
    static char devName[32];

    printf("serialInit: ii=%d ip.adr=0x%lx\n", ii, ip.adr[ii]);

    if (ip.adr[ii] != 0) {
      unsigned long const base = *(short*)(IP_MEM_BASE_BASE + ii * 2);

      pOctSerDv = octSerModuleInit((char*)(((long) base << 16) + 1),
				   ip.adr[ii], vec);

      printf("pOctSerDv = 0x%lx\n", pOctSerDv);


      if (octSerDrv() == ERROR) {}
      else {
	int jj;

        rebootHookAdd(serialRebootHook);
/*
 * Initialize all 8 serial ports as vxworks devices.
*/
	for (jj = 0; jj < 8; ++jj) {
	  sprintf(devName,"/tyCo/%d", ch + jj);
	  printf("jj=%d devName=%s\n", jj, devName);
	  if (octSerDevCreate(&pOctSerDv[jj], devName, 1024, 1024)) {
	    break;
	  } else {}
	}
      }

      (void) VME_IP_Interrupt_Enable(ip.adr[ii], 5);
      (void) sysIntEnable(5);

      break;
    }
  }
}
