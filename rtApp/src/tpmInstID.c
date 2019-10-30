#include <vxWorks.h>
#include <types.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <string.h>
#include <dbDefs.h>
#include <genSubRecord.h>
#include <genSubUtil.h>
#include <dbCommon.h>
#include <recSup.h>

int InstIDDebug = 0;

long tpmInstIDInit(struct genSubRecord *psub)
{
    return OK;
}

long tpmInstIDProcess(struct genSubRecord *psub)
{
    long status;
    int i;
    int idx;	
    long abi1_16;
    char *instid[3];
/*
 *	Negation and bit order reversal of 4 bit nibble.
 */
    int bitmap[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 
			6, 10, 2, 12, 4, 8, 0};
/*
 *	ID strings
*/
    const char idstring[40][16] = {
	"DISCONN", "CART 1", "CART 2", "CART 3", 
	"CART 4", "CART 5", "CART 6", "CART 7",
	"CART 8", "CART 9", "ENG CAM", "11-UNUSED", 
	"12-UNUSED", "13-UNUSED", "IMAGER", "NO DEVICE"};


    abi1_16 = genSubLongInput(psub, &psub->a, &status);
    if (InstIDDebug) printf("abi1_16 = %04x\n", abi1_16);

    for (i=0; i < 3; i++) {
        instid[i] = genSubStringOutput(psub, &psub->vala + i, &status);
	idx = (abi1_16 >> (i*4)) & 0x000f; 
	if (InstIDDebug) printf("%d: raw=%01x\n", i, idx);
	idx = bitmap[idx]; 
	if (InstIDDebug) printf("%d: idx=%01x\n", i, idx);
	strcpy(instid[i], idstring[idx]);
    }

    return 0;
}
