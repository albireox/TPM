/* tnetDev.h */

/*
modification history
-------------------
01a,29apr96,bdg  created
*/

#ifndef INCtnetDevh
#define INCtnetDevh


#define TNET_PTY_SIZE	4096		/* xfer buffer size */
#define TNET_TASK_PRI	3		/* connect task priority */
#define TNET_CONN_RETRY	15		/* retry to connect every 15 seconds */

extern STATUS	tnetDevCreate (char *, char *, int);
extern STATUS	tnetDevClose (char *, int);


#endif	/* !INCtnetDevh */
