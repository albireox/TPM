#include <vxWorks.h>
#include <stdio.h>
#include <etherLib.h>
#include <ifLib.h>
#include <inetLib.h>
#include <sockLib.h>
#include <socket.h>
#include <string.h>
#include <taskLib.h>
#include "tsDefs.h"
#include "tsSubrHead.h"
#include <types.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <dbDefs.h>
#include <subRecord.h>
#include <dbCommon.h>
#include <recSup.h>

#define MAX_ALARMS 10

struct request {
	char msg[1024];
};

long packeteer_seqno = 0;
int sFd = 0;

struct {
    int nalarms;
    char keyword[MAX_ALARMS][40];
    char value[MAX_ALARMS][40];
} packeteerAlarms;


int
packeteer_init()
{
    struct sockaddr_in serverAddr;
    int sockAddrSize;
    char inetAddr[INET_ADDR_LEN];
    extern void packeteer_listen();

    packeteer_seqno = 0;
    packeteerAlarms.nalarms = 0;

    sockAddrSize = sizeof(struct sockaddr_in);
    bzero((char *)&serverAddr, sockAddrSize);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(30311);

    
    if ((sFd = socket(AF_INET, SOCK_DGRAM, 0)) == ERROR) {
	printf("Failed to open the socket!\n");
	return ERROR;
    }

    if (bind(sFd, (struct sockaddr *)&serverAddr, sockAddrSize) == ERROR) {
	printf("bind failed\n");
	return ERROR;
    }
/*
    taskSpawn("pListen", 199, VX_FP_TASK, 8000,
         (FUNCPTR)packeteer_listen, sFd, 0,0,0,0,0,0,0,0,0);
*/
                                                                         
    return OK;
}

void
packeteer_send(char *clientName, char *msg)
{
    int mlen;
    struct request myRequest;
    struct sockaddr_in clientAddr;
    int sockAddrSize;

    sockAddrSize = sizeof(struct sockaddr_in);
    bzero((char *)&clientAddr, sockAddrSize);
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(30311);

    if (((clientAddr.sin_addr.s_addr = inet_addr(clientName)) == ERROR) &&
	((clientAddr.sin_addr.s_addr = hostGetByName(clientName)) == ERROR)) {
	   printf("unknown client name: %s\n", clientName);
	   return;
    }

    strcpy(myRequest.msg, msg);

    if (sendto(sFd, (caddr_t) &myRequest, sizeof(myRequest), 0,
	(struct sockaddr *) &clientAddr, sockAddrSize) == ERROR) {
	printf("sendto failed\n");
	return;
    } else {
	printf("send %s\n", myRequest.msg);
    }
}

void
packeteer_add_alarm(char *keyword, char *value)
{
    if (packeteerAlarms.nalarms < MAX_ALARMS) {
	strcpy(packeteerAlarms.keyword[packeteerAlarms.nalarms], keyword);
	strcpy(packeteerAlarms.value[packeteerAlarms.nalarms], value);
	packeteerAlarms.nalarms++;
    } else {
	printf("Maximum of %d alarms exceeded!\n", MAX_ALARMS);
    }	
}

void
packeteer_send_alarm(char *clientName) 
{
    TS_STAMP now;
    char msg[1024];
    char errmsg[128];
    int i;


    if (packeteerAlarms.nalarms > 0) {
        tsLocalTime(&now);

        sprintf(msg, "timeStamp %d serialNum %d arrayname tpmError",
	    now.secPastEpoch + TS_EPOCH_SEC_PAST_1970,
	    packeteer_seqno++);

	for (i = 0; i < packeteerAlarms.nalarms; i++) {
	    sprintf(errmsg," \042%s\042 \042%s\042",
		packeteerAlarms.keyword[i],
		packeteerAlarms.value[i]);
		strcat(msg, errmsg);
	}
	packeteerAlarms.nalarms = 0;

        packeteer_send(clientName, msg);
    }
}

void
packeteer_listen() 
{
    struct sockaddr_in clientAddr;
    struct request clientRequest;
    int sockAddrSize;
    char inetAddr[INET_ADDR_LEN];
    char gnp[80];

    sockAddrSize = sizeof(struct sockaddr_in);

    while (1) {
        if (recvfrom(sFd, (char *) &clientRequest, sizeof(clientRequest),
			0, (struct sockaddr *)&clientAddr, &sockAddrSize) 
		== ERROR) {
	     printf("recvfrom failed!\n");
    	     return 0;
        }
        inet_ntoa_b(clientAddr.sin_addr,inetAddr);

        printf("read: %s %s\n", inetAddr,clientRequest.msg);

        if (!strncmp(clientRequest.msg, "PNG:", 4)) {
	    sprintf(gnp,"GNP:sdsstpm:peregrin:%d:30311",taskIdSelf());
	    packeteer_send(inetAddr,gnp);
	} 
    }
}


long tpmPacketInit(struct subRecord *psub)
{
    packeteer_init();
    return (0);
}

void
packeteer_test(char *client_name)
{
    packeteer_add_alarm("TPM","In test mode");
    packeteer_add_alarm("Altitude axis","Tracking error");
    packeteer_add_alarm("M1", "Oscillating");
    packeteer_add_alarm("M2", "Force limit!");
    packeteer_send_alarm(client_name);
}
	
long tpmPacketProcess(struct subRecord *psub)
{
    packeteer_test("sdsshost");
    psub->val = packeteer_seqno;

    return 0;
}
