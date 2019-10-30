#include <vxWorks.h>
#include <types.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <string.h>
#include <taskLib.h>
#include <dbDefs.h>
#include <subRecord.h>
#include <dbCommon.h>
#include <recSup.h>

#include "data_collection.h"
#include "tpmSub.h"

/* This pointer get us to the shared data structure */
#define SHARED_MEMORY ((char volatile*) 0x02800000)

int SHMEM_MAXTRY = 2;
int SHMEM_MAXCNT = 0;
int SHMEM_DEBUG = 0;

/*int CRCCHECK_MAXTRY = 3;*/
int CRCCHECK_MAXTRY = 10;
int CRCCHECK_FORCE = 0;

/*****************************************************************************/
/*
 * Calculate a CRC for a string of characters. You can call this routine
 * repeatedly to build up a CRC. Only the last 16bits are significant.
 *
 * e.g.
 *   crc = phCrcCalc(0, buff, n) & 0xFFFF;
 * or
 *   crc = 0;
 *   crc = phCrcCalc(crc, buff0, n);
 *   crc = phCrcCalc(crc, buff1, n);
 *   crc = phCrcCalc(crc, buff2, n);
 *   crc &= 0xFFFF;
 *
 * Routine taken from photo/src/utils.c
 */
long
phCrcCalc(long crc,			/* initial value of CRC (e.g. 0) */
	  const char *buff,		/* buffer to be CRCed */
	  int n)			/* number of chars in buff */
{
   register long c;
   static long crcta[16] = {
      0L, 010201L, 020402L, 030603L, 041004L,
      051205L, 061406L, 071607L, 0102010L,
      0112211L, 0122412L, 0132613L, 0143014L,
      0153215L, 0163416L, 0173617L
   };					/* CRC generation tables */
   static long crctb[16] = {
      0L, 010611L, 021422L, 031233L, 043044L,
      053655L, 062466L, 072277L, 0106110L,
      0116701L, 0127532L, 0137323L, 0145154L,
      0155745L, 0164576L, 0174367L
   };
   const char *ptr = buff;		/* pointers to buff */
   const char *const end = buff + n;	/*        and to end of desired data */

   for(;ptr != end;ptr++) {
      c = crc ^ (long)(*ptr);
      crc = (crc >> 8) ^ (crcta[(c & 0xF0) >> 4] ^ crctb[c & 0x0F]);
   }

   return(crc);
}

/*
 * The MCP supports two different protocols during
 * February 2001 so the TPM must be capable of responding
 * to both of them.
 * OLD: 1 => MCP is updating shmem.
 *	0 => MCP is not updating shmem.
 * NEW: 0 => MCP is updating shmem.
 *	1-65535 => MCP is not updating shmem (counter).
 *
 * The TPM determines which protocol is in effect by 
 * saving the historical maximum of the MCP flag.
 * MAX < 2 => OLD
 * MAX >= 2 => NEW
 *
 * In both protocols the MCP flag must be the same before
 * and after the TPM's read of shared memory. 
 *
 * We have to ignore the flag value of 65535 (all bits set)
 * since we see this happen sometimes even when the old
 * protocol should be alternating between 0 and 1.
*/
void shmem_maxcnt()
{
    printf("SHMEM_MAXCNT=%d\n", SHMEM_MAXCNT);
}

static long read_shmem(int offset, int len, void *buf)
{
    struct SDSS_FRAME *data = (struct SDSS_FRAME *)(SHARED_MEMORY + 2);
    unsigned short start_mcp_count, finish_mcp_count;
    int i;
    int j, k;

    for (i=0; i < SHMEM_MAXTRY; i++) {
        start_mcp_count = *(unsigned short *)SHARED_MEMORY;
	tpm_MCPCOUNT = start_mcp_count;
        if (start_mcp_count < 65535 && start_mcp_count > SHMEM_MAXCNT) {
	    SHMEM_MAXCNT = start_mcp_count;
	    if (SHMEM_DEBUG) printf("Increased max (start) to %d\n", SHMEM_MAXCNT);
	}

	/* 
	Check if the MCP is currently updating shared memory 
	If it is, then do a short delay and retry.
	*/
	if ((SHMEM_MAXCNT < 2 && start_mcp_count == 1) ||
	    (SHMEM_MAXCNT >= 2 && start_mcp_count == 0)) {
	    for (j=0; j<1000; j++) k = j+1;
	    continue;
	}
	
        /* Go get it */
        memcpy((char *)buf, (char *)(SHARED_MEMORY + offset + 2), len);
        finish_mcp_count = *(unsigned short *)SHARED_MEMORY;
        if (finish_mcp_count < 65535 && finish_mcp_count > SHMEM_MAXCNT) {
	    SHMEM_MAXCNT = finish_mcp_count;
	    if (SHMEM_DEBUG) printf("Increased max (finish) to %d\n", SHMEM_MAXCNT);
	}

        /* Recheck */
	if (finish_mcp_count == start_mcp_count)  {
	    if (SHMEM_DEBUG) printf("finish=start %d\n", i);
	    tpm_SHMLOOPS += i;
	    return i;
	}
    }
    tpm_SHMLOOPS += SHMEM_MAXTRY;
    return -1;
}

void shmemt(int n, int dt, int report_sync) {
    struct SDSS_FRAME *data = (struct SDSS_FRAME *)(SHARED_MEMORY + 2);
    int m2, old_m2 = -1;
    int i;
    I1_L10 i1_l10;
    unsigned char tbuf[4];
    double rate;
    int status;

    rate = sysClkRateGet();

    for (i=0; i<n; i++) {
        status = read_shmem((char *)&(data->status.i1.il10) - (char *)data, 
		sizeof(I1_L10), &i1_l10);
	if (status < 0 && report_sync) printf("%.2f no sync\n", i*dt/rate);
        if (status >= 0) {
            m2 = i1_l10.sec_mir_force_limits;
        }
	if (m2 != old_m2) {
	    memcpy((char *)tbuf, (char*)&i1_l10, 4);	
	    printf("%.2f %d->%d %x %x %x %x\n", i*dt/rate, old_m2, m2,
		tbuf[0], tbuf[1], tbuf[2], tbuf[3]);
	    old_m2 = m2;
	}
	taskDelay(dt);
    }
}

void read_shmem_with_crc() {
    struct SDSS_FRAME *data = (struct SDSS_FRAME *)(SHARED_MEMORY + 2);
    struct SDSS_FRAME *sdssdc = &TPM_SDSS_FRAME;
    int i;
    unsigned short CRC;         /* CRC of data starting offset
                                           bytes into struct */
    int offset;
    int status;

    offset = (char *)&(data->ctime) - (char *)data;

    tpm_SHMLOOPS = 0;
    for (i = 0; i < CRCCHECK_MAXTRY; i++) {
	status = read_shmem(0, sizeof(struct SDSS_FRAME), sdssdc);
	if (status < 0) {
	    continue;
	}

        CRC = phCrcCalc(0, (char *)(sdssdc) + offset,
                            (signed)sizeof(struct SDSS_FRAME) - offset) & 0xFFFF;

        if (sdssdc->CRC == CRC) break; 
    }
    tpm_CRCLOOPS = i;
    if (CRCCHECK_FORCE == 1) tpm_CRCLOOPS = 0;
}


/* 
 * Sign extend 12-bit signed quantities 
 * specifically motor currents
*/
long extend12bit(int x)
{
/*
    int low11 = x & 0x07ff;
    int sgn = (x & 0x0800) ? -1 : 1;
*/

    return (x > 2047 ? (x - 4096) : x);
}


long tpmSubInit(struct subRecord *psub)
{
    strncpy(tpm_VERSION, tpmVersion(0,0), MAX_STRING_SIZE);

    return (0);
}

void tpmIlockDump(int sword, int eword) 
{
    int i;
    struct SDSS_FRAME *data = (struct SDSS_FRAME *)(SHARED_MEMORY + 2);
    unsigned short * tsf_i1 = (unsigned short *)&(TPM_SDSS_FRAME.status.i1);
    unsigned short * sf_i1 = (unsigned short *)&(data->status.i1);
    unsigned short start_mcp_count = *(unsigned short *)SHARED_MEMORY;

    printf("MCP_COUNT = %d\n", start_mcp_count);

    for (i = sword; i <= eword; i++) {
	printf("ILOCK(%d) = %04x ", i, tpm_ILOCK[i]);
	printf("%04x ", *(tsf_i1 + i));
	printf("%04x\n", *(sf_i1 + i));
    }
}

/*
 * At a regular rate (1-20Hz) scan the TPM data into global VxWorks variables
 * so we can access them using EPICS symbolic device support.
*/
long tpmSub1HzProcess(struct subRecord *psub)
{
    if (tpm_CRCLOOPS >= CRCCHECK_MAXTRY) return (0);

    tpm_M2FORCE = TPM_SDSS_FRAME.status.i1.il10.sec_mir_force_limits;
    memcpy(tpm_ILOCK, (void *)&TPM_SDSS_FRAME.status.i1, 190*sizeof(unsigned short));

    tpm_ALBRKENSTAT = TPM_SDSS_FRAME.status.i9.il0.alt_brake_en_stat;
    tpm_ALBRKDISSTAT = TPM_SDSS_FRAME.status.i9.il0.alt_brake_dis_stat;
    tpm_AZBRKENSTAT = TPM_SDSS_FRAME.status.i9.il0.az_brake_en_stat;
    tpm_AZBRKDISSTAT = TPM_SDSS_FRAME.status.i9.il0.az_brake_dis_stat;

    tpm_ALBRKENCMD = TPM_SDSS_FRAME.status.o12.ol0.alt_brake_en;
    tpm_ALBRKDISCMD = TPM_SDSS_FRAME.status.o12.ol0.alt_brake_dis;
    tpm_AZBRKENCMD = TPM_SDSS_FRAME.status.o12.ol0.az_brake_en;
    tpm_AZBRKDISCMD = TPM_SDSS_FRAME.status.o12.ol0.az_brake_dis;

    tpm_ALBRKENMCP = TPM_SDSS_FRAME.status.b10.w0.mcp_alt_brk_en_cmd;
    tpm_ALBRKDISMCP = TPM_SDSS_FRAME.status.b10.w0.mcp_alt_brk_dis_cmd;
    tpm_AZBRKENMCP = TPM_SDSS_FRAME.status.b10.w0.mcp_az_brk_en_cmd;
    tpm_AZBRKDISMCP = TPM_SDSS_FRAME.status.b10.w0.mcp_az_brk_dis_cmd;

    tpm_AZSTATE = TPM_SDSS_FRAME.axis_state[0];
    tpm_ALSTATE = TPM_SDSS_FRAME.axis_state[1];
    tpm_ROSTATE = TPM_SDSS_FRAME.axis_state[2];

    return (0);
}

long tpmSub5HzProcess(struct subRecord *psub)
{
    return (0);
}

long tpmSub20HzProcess(struct subRecord *psub)
{
    long tmp;

    read_shmem_with_crc();
    if (tpm_CRCLOOPS >= CRCCHECK_MAXTRY) return (0);

    tpm_AZMTRC1 = extend12bit(TPM_SDSS_FRAME.status.i3.az_1_current);
    tpm_AZMTRC2 = extend12bit(TPM_SDSS_FRAME.status.i3.az_2_current);
    tpm_AZMTRV1 = TPM_SDSS_FRAME.status.i3.az_1_voltage;
    tpm_AZMTRV2 = TPM_SDSS_FRAME.status.i3.az_2_voltage;
    tpm_ALMTRC1 = extend12bit(TPM_SDSS_FRAME.status.i3.alt_1_current);
    tpm_ALMTRC2 = extend12bit(TPM_SDSS_FRAME.status.i3.alt_2_current);
    tpm_ALMTRV1 = TPM_SDSS_FRAME.status.i3.alt_1_voltage;
    tpm_ALMTRV2 = TPM_SDSS_FRAME.status.i3.alt_2_voltage;

    tpm_ROMTRC = extend12bit(TPM_SDSS_FRAME.status.i4.rot_1_current);
    tpm_ROMTRV = TPM_SDSS_FRAME.status.i4.rot_1_voltage;

    tpm_AZCPOS = (TPM_SDSS_FRAME.axis[0].position_hi << 16) + 
		  TPM_SDSS_FRAME.axis[0].position_lo;
    tpm_AZCVOLT = TPM_SDSS_FRAME.axis[0].voltage;
    tpm_AZENC1 = (TPM_SDSS_FRAME.axis[0].actual_position_hi << 16) +
		TPM_SDSS_FRAME.axis[0].actual_position_lo;
    tpm_AZENC2 = (TPM_SDSS_FRAME.axis[0].actual_position2_hi << 16) +
		TPM_SDSS_FRAME.axis[0].actual_position2_lo;
    tpm_AZERR = TPM_SDSS_FRAME.axis[0].error;

    tpm_ALCPOS = (TPM_SDSS_FRAME.axis[1].position_hi << 16) + 
		  TPM_SDSS_FRAME.axis[1].position_lo;
    tpm_ALCVOLT = TPM_SDSS_FRAME.axis[1].voltage;

tmp = tpm_ALENC1;
    tpm_ALENC1 = (TPM_SDSS_FRAME.axis[1].actual_position_hi << 16) + 
		TPM_SDSS_FRAME.axis[1].actual_position_lo;
tpm_DALENC1 = tpm_ALENC1 - tmp;

    tpm_ALENC2 = (TPM_SDSS_FRAME.axis[1].actual_position2_hi << 16) +
		TPM_SDSS_FRAME.axis[1].actual_position2_lo;
    tpm_ALERR = TPM_SDSS_FRAME.axis[1].error;

    tpm_ROCENC = (TPM_SDSS_FRAME.axis[2].actual_position_hi << 16) + 
		TPM_SDSS_FRAME.axis[2].actual_position_lo;
    tpm_ROCPOS = (TPM_SDSS_FRAME.axis[2].position_hi << 16) + 
		TPM_SDSS_FRAME.axis[2].position_lo;
    tpm_ROCVOLT = TPM_SDSS_FRAME.axis[2].voltage;
    tpm_ROERR = TPM_SDSS_FRAME.axis[2].error;

    tpm_AZMCPPOS = TPM_SDSS_FRAME.pvt[0].position;
    tpm_AZMCPVEL = TPM_SDSS_FRAME.pvt[0].velocity;
    tpm_AZMCPTIM = TPM_SDSS_FRAME.pvt[0].time;
    tpm_ALMCPPOS = TPM_SDSS_FRAME.pvt[1].position;
    tpm_ALMCPVEL = TPM_SDSS_FRAME.pvt[1].velocity;
    tpm_ALMCPTIM = TPM_SDSS_FRAME.pvt[1].time;
    tpm_ROMCPPOS = TPM_SDSS_FRAME.pvt[2].position;
    tpm_ROMCPVEL = TPM_SDSS_FRAME.pvt[2].velocity;
    tpm_ROMCPTIM = TPM_SDSS_FRAME.pvt[2].time;

    tpm_AZTCCPOS = TPM_SDSS_FRAME.tccmove[0].position;
    tpm_AZTCCVEL = TPM_SDSS_FRAME.tccmove[0].velocity;
    tpm_AZTCCTIM = TPM_SDSS_FRAME.tccmove[0].time;
    tpm_ALTCCPOS = TPM_SDSS_FRAME.tccmove[1].position;
    tpm_ALTCCVEL = TPM_SDSS_FRAME.tccmove[1].velocity;
    tpm_ALTCCTIM = TPM_SDSS_FRAME.tccmove[1].time;
    tpm_ROTCCPOS = TPM_SDSS_FRAME.tccmove[2].position;
    tpm_ROTCCVEL = TPM_SDSS_FRAME.tccmove[2].velocity;
    tpm_ROTCCTIM = TPM_SDSS_FRAME.tccmove[2].time;

    return (0);
}

long tpmSub200HzProcess(struct subRecord *psub)
{
/*
 * Altitude Axis Positions, Currents, Voltages, Error
*/ 
    tpm_ALENC1F = (TPM_SDSS_FRAME.axis[1].actual_position_hi << 16) + 
		TPM_SDSS_FRAME.axis[1].actual_position_lo;
    tpm_ALMTRC1F = TPM_SDSS_FRAME.status.i3.alt_1_current;
    tpm_ALERRF = TPM_SDSS_FRAME.axis[1].error;

/*
 * Azimuth Axis Positions, Currents, Voltages, Error
*/  
    tpm_AZENC1F = (TPM_SDSS_FRAME.axis[0].actual_position_hi << 16) +
		TPM_SDSS_FRAME.axis[0].actual_position_lo;
    tpm_AZMTRC1F = TPM_SDSS_FRAME.status.i3.az_1_current;
    tpm_AZERRF = TPM_SDSS_FRAME.axis[0].error;

/*
 * Rotator Positions, Currents, Voltages, Error
*/ 
    tpm_ROCENCF = (TPM_SDSS_FRAME.axis[2].actual_position_hi << 16) + 
		TPM_SDSS_FRAME.axis[2].actual_position_lo;
    tpm_ROMTRCF = TPM_SDSS_FRAME.status.i4.rot_1_current &= 0x0fff;
    tpm_ROERRF = TPM_SDSS_FRAME.axis[2].error;

    return (0);
}
