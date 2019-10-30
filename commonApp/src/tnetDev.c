/* tnetDev.c */
#include <stdio.h>

/*
modification history
-------------------
01a,29apr96,bdg  created
*/

/*
DESCRIPTION

This program acts as a VxWorks client for a remote terminal server.  The
remote host may be identified by either name or IP address.  The port must
be greater than the reserved IP port numbers (currently 1024).  A TCP socket
is connected to the remote host and a task spawned to monitor the socket.
A pseudo-terminal device is created to buffer input and output through
the socket to the user.  A typical applications might look like:
	hostAdd ("sim2ioc", "140.252.15.83");
	tnetDevCreate ("/pty/tmp.", "sim2ioc", 3301);
	fd = open ("/pty/tmp.M", O_RDWR, 0);
	write (fd, "i\n", 2);
	n = read (fd, buf, 1024);

NOTES
There is no graceful way to stop the connection task.
A reboot hook needs to be added to at least shut the socket down.

*/

#ifdef vxWorks
#include "vxWorks.h"
#include "hostLib.h"
#include "inetLib.h"
#include "sockLib.h"
#include "taskLib.h"
#include "sysLib.h"
#include "ptyDrv.h"
#include "errnoLib.h"
#include "iosLib.h"
#include "ioLib.h"
#include "string.h"
#include "selectLib.h"
#include "logLib.h"
#define  getErrno errnoGet ()
#else
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define logMsg(a,b,c,d,e,f,g) fprintf(stderr,(a),(b),(c),(d),(e),(f),(g))
#define STATUS int
#define BOOL   int
#define OK 0
#define ERROR -1
#define TRUE 1
#define FALSE 0
#define FOREVER while (TRUE)
#define getErrno errno
int prtMsg = TRUE;
#endif

int tnetDebug = 0;

#include "tnetDev.h"
 
#define TNET_TASK_NAME  "tTnet"
#define TNET_TASK_OPT   (VX_SUPERVISOR_MODE | VX_UNBREAKABLE | VX_STDIO)
 
static void     tnetTask (int, int, int);
static int      tnetOpen (int, int, int);


/*******************************************************************************
* tnetDevCreate - create a tnet device
*
* This routine creates a device which connects to a remote terminal server.
* The device name, remote host name or IP address, and server port number
* are input arguments.  The connection tries forever and stays connected
* forever.
*
* RETURNS
*   OK or ERROR if the device could not be created.
*/
 
#ifdef vxWorks
int tnetDevCreate (
    char *serialName,			/* serial device to create */
    char *hostName,			/* host to contact */
    int port)				/* port on host to connect */
{
 
    int fd;
    int ipAdrs;
    int tid;
    char slaveName[128];
					/* simple argument checking */
    if (serialName == NULL || hostName == NULL || port < IPPORT_RESERVED)
    {
	logMsg ("tnetDevCreate: ERROR--invalid arguments\n",
	    NULL, NULL, NULL, NULL, NULL, NULL);
	return ERROR;
    }
					/* resolve IP address */
    if ((ipAdrs = inet_addr (hostName)) == ERROR)
	if ((ipAdrs = hostGetByName (hostName)) == ERROR)
	{
	    errnoSet (S_hostLib_UNKNOWN_HOST);
	    logMsg ("tnetDevCreate: ERROR--invalid host name %s\n",
		(int) hostName, NULL, NULL, NULL, NULL, NULL);
	    return ERROR;
	}
					/* create pseudo tty pair */
    if (ptyDrv () == ERROR ||
	ptyDevCreate (serialName, TNET_PTY_SIZE, TNET_PTY_SIZE) == ERROR)
    {
	logMsg ("tnetDevCreate: ERROR--cannot create pty %s (0x%08x)s\n",
	    (int) serialName, getErrno, NULL, NULL, NULL, NULL);
	return ERROR;
    }
					/* build slave file name */
    strncpy (slaveName, serialName, sizeof (slaveName));
    strncat (slaveName, "S", 1);
					/* open slave file name */
    if ((fd = open (slaveName, O_RDWR, 0)) == ERROR )
    {
	logMsg ("tnetDevCreate: ERROR--cannot open pty %s (0x%08x)s\n",
	    (int) slaveName, getErrno, NULL, NULL, NULL, NULL);
	return ERROR;
    }
					/* spawn communication task */
    if ((tid = taskSpawn (TNET_TASK_NAME, TNET_TASK_PRI, TNET_TASK_OPT,
	10000, (FUNCPTR) tnetTask, fd, ipAdrs, port, 
	NULL, NULL, NULL, NULL, NULL, NULL, NULL)) == ERROR)
    {
	logMsg ("tnetDevCreate: ERROR--unable to spawn %s (0x%x)\n",
	    (int) TNET_TASK_NAME, getErrno, NULL, NULL, NULL, NULL);
	return ERROR;
    }

    return tid;
}
#else
int main( int argc, char * argv[] )
{
    char *serialName;			/* serial device to create */
    char *hostName;			/* host to contact */
    int port=0;				/* port on host to connect */
    int fd;
    int ipAdrs;
    int i;
    struct hostent * hostentry;
    struct in_addr in;

    if (argc < 4 )
    {
        fprintf( stderr, "Usage: %s, <Serial Device>, <Host>, <port>\n", argv[0] );
        return ERROR;
    }

    serialName = argv[1];
    hostName = argv[2];
    port = atoi( argv[3] );
					/* simple argument checking */
    if (serialName == NULL || hostName == NULL || port < IPPORT_RESERVED|| strncmp(serialName, "/dev/pty", 8)!=0 )
    {
	fprintf( stderr,"tnetDevCreate: ERROR--invalid arguments\n" );
	return ERROR;
    }
					/* resolve IP address */
    if ((ipAdrs = inet_addr (hostName)) == ERROR)
    {
        if ((hostentry = gethostbyname (hostName)) == NULL)
	{
	    fprintf ( stderr, "tnetDevCreate: ERROR--invalid host name %s\n", hostName );
	    return ERROR;
        }
        (void) memcpy(&in.s_addr, *(hostentry->h_addr_list), sizeof (in.s_addr));
        ipAdrs = inet_addr ( inet_ntoa(in) );
    }

    /* open controller device name */
    if ((fd = open (serialName, O_RDWR | O_NDELAY | O_NONBLOCK)) == ERROR )
    {
	fprintf( stderr, "tnetDevCreate: ERROR--cannot open pty %s (0x%08x)s\n", (int) serialName, getErrno );
	return ERROR;
    }

    /* Set controller device in remote mode */
    i = 1;
    ioctl( fd, TIOCREMOTE, &i );

    /* start communication task */
    tnetTask( fd, ipAdrs, port );
}
#endif

/*******************************************************************************
* tnetTask
*/

static void tnetTask (
    int fd,				/* pty file descriptor */
    int ipAdrs,				/* remote host address */
    int port)				/* remote host port */
{
    int sd;
    int n;
    char *buf;
    fd_set readFds;
    int i;
					/* open network connection */
    if ((sd = tnetOpen (-1, ipAdrs, port)) == ERROR)
    {
	logMsg ("tnetDev: ERROR--cannot open socket (0x%08x)\n", 
	    getErrno, NULL, NULL, NULL, NULL, NULL);
	return;
    }
    buf = (char *) malloc (TNET_PTY_SIZE);
				/* wait for data on either descriptor */
    FOREVER
    {
	FD_ZERO (&readFds);
	FD_SET (fd, &readFds);
	FD_SET (sd, &readFds);
	if (select (FD_SETSIZE, &readFds, NULL, NULL, NULL) == ERROR)
	{
	    logMsg ("tnetDev: ERROR--select 0x%x\n",
		getErrno, NULL, NULL, NULL, NULL, NULL);
	    break;
	}
	
				/* read pty, write it to socket */
	if (FD_ISSET (fd, &readFds))
	{
	    n = read (fd, buf, TNET_PTY_SIZE);
            if (tnetDebug == 1 && n != ERROR) {
	        printf("tnetDev: fd (%d) ", n);
                for (i=0; i < n; i++) printf("%c", buf[i]);
                printf("\n");
            }
	    if (n == ERROR)
#ifdef vxWorks
		logMsg ("tnetDev: read fd error 0x%x\n",
		    getErrno, NULL, NULL, NULL, NULL, NULL);
#else
            {
                static int count;

                sleep( 1 );
                count++;
                if (count > 20)
                {
                    fprintf( stderr, "tnetDev task shutting down after 20 seconds of inactivity" );
                    exit( EXIT_FAILURE );
                }
            }
#endif
	    else if (write (sd, buf, n) == ERROR)
	    {
		if ((sd = tnetOpen (sd, ipAdrs, port)) == ERROR)
		    logMsg ("tnetDev: ERROR--cannot open socket (0x%08x)\n",
			getErrno, NULL, NULL, NULL, NULL, NULL);
	    }

	}
					/* read socket, write it to pty */
	if (FD_ISSET(sd, &readFds))
	{
	    n = read (sd, buf, TNET_PTY_SIZE);
            if (tnetDebug == 1 && n != ERROR) {
	        printf("tnetDev: sd (%d) ", n);
                for (i=0; i < n; i++) printf("%c", buf[i]);
                printf("\n");
            }
	    if (n == ERROR || n == 0)
	    {
                logMsg ("tnetDev: ERROR--read from sd returned %d\n",
			n, NULL, NULL, NULL, NULL, NULL);
		        
		if ((sd = tnetOpen (sd, ipAdrs, port)) == ERROR)
		    logMsg ("tnetDev: ERROR--cannot open socket (0x%08x)\n",
		        getErrno, NULL, NULL, NULL, NULL, NULL);
		else
		    logMsg ("tnetDev: successfully reopened socket\n",
		        NULL, NULL, NULL, NULL, NULL, NULL);
	    }
	    else if (write (fd, buf, n) == ERROR)
            {
		logMsg ("tnetDev: write fd error 0x%x\n",
		    getErrno, NULL, NULL, NULL, NULL, NULL);
            }

/*             if (n != ERROR) */
/*             { */
/*                 int i; */
/*                 for (i=0;i<n;i++) printf("%c\n",buf[i]); */
/*             }                 */
	}
    }
					/* doubtful we'll ever get here */
    free (buf);

    close (fd);
    close (sd);
}


/*******************************************************************************
* tnetOpen
*/

static int tnetOpen (
    int sd,
    int ipAdrs,				/* ip address of remote host */
    int port)				/* port of remote host */
{
    struct sockaddr_in sin;
    int one = 1;
    BOOL prtMsg = TRUE;

    if (sd > 0)				/* clean up */
	close (sd);

    sd = ERROR;
    while (sd == ERROR)
    {
					/* configure socket */
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == ERROR)
	    break;

	setsockopt (sd, SOL_SOCKET, SO_KEEPALIVE, (char *) &one, sizeof (one));
					/* configure inet */
	memset ((char *) &sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl (ipAdrs);
	sin.sin_port = htons (port);
	if (connect (sd, (struct sockaddr *) &sin, sizeof (sin)) == ERROR)
	{
	    if (prtMsg)			/* print message first time only */
	    {
		logMsg ("tnetDev: ALERT--cannot connect to 0x%x %d (0x%x)\n",
		    ipAdrs, port, getErrno, NULL, NULL, NULL);
		prtMsg = FALSE;
	    }
	    close (sd);
	    sd = ERROR;
#ifdef vxWorks
	    (void) taskDelay (sysClkRateGet () * TNET_CONN_RETRY);
#else
            sleep( TNET_CONN_RETRY );
#endif
	}
    }
					/* print message if we tried hard */
    if (!prtMsg)
	logMsg ("tnetDev: INFO--connected to 0x%x %d (0x%x)\n",
	    ipAdrs, port, getErrno, NULL, NULL, NULL);

    return sd;
}


/*******************************************************************************
* tnetDevClose - clean up remote connections after a reboot
*
* This is a diagnostic routine which will tell a remote server to close
* a connection.  It is used to clean up after a hard vxWorks reboot
* when we don't want another reconnect.  Otherwise, the remote server
* will wait forever to reestablish the link with this host.
*/

STATUS tnetDevClose (
    char *hostName,
    int port)
{
    int ipAdrs;
    int sd;
					/* resolve IP address */
    if ((ipAdrs = inet_addr (hostName)) == ERROR)
    {
#ifdef vxWorks
	if ((ipAdrs = hostGetByName (hostName)) == ERROR)
	{
	    errnoSet (S_hostLib_UNKNOWN_HOST);
	    logMsg ("tnetDevCreate: ERROR--invalid host name %s\n",
		(int) hostName, NULL, NULL, NULL, NULL, NULL);
	    return ERROR;
	}
#else
        struct hostent * hostentry;

        if ((hostentry = gethostbyname (hostName)) == NULL)
	{
	    fprintf ( stderr, "tnetDevCreate: ERROR--invalid host name %s\n", hostName );
	    return ERROR;
        }
        ipAdrs = inet_addr ( *hostentry->h_addr_list );
#endif
    }

    if ((sd = tnetOpen (-1, ipAdrs, port)) == ERROR)
	return ERROR;
    close (sd);

    return OK;
}

