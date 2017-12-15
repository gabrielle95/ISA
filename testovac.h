/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: testovac.h - header file for testovac.c */

#ifndef TESTOVAC_H
#define TESTOVAC_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "memops.h"
#include "argparse.h"
#include "socket.h"
#include "icmp.h"
#include "udp.h"

#define TPARAM_DEF 300//(300 s)
#define IPARAM_DEF 100 // (100 ms)
#define WPARAM_DEF 2000//(2 s)
#define SPARAM_DEF 56 //Bytes
#define SUDP_DEF 64

#define ICMP_HDRLEN 8
#define IP_HDRLEN 20
#define IP6_HDRLEN 40
#define FR_SIZE_MAX 1480

#define ICMPV6_ECHO 128
#define ICMPV6_ECHOREPLY 129

/* Strucures for argument flags and values */
/* -h is checked before setting these flags */
typedef struct{
	bool f_udp; //-udp prot
    bool f_tparam; // -t 300s
    bool f_iparam; // -i 100 ms
    bool f_wparam; //-w (2*rtt)
    bool f_sparam; //-s
    bool f_port; //-p
    bool f_listenp; //-l
    bool f_rparam; //-r, RTT value, reported if RTT exceeds this limit
    bool f_verbose; //-v
} ArgFlags;

typedef struct{
	float tparam; //sec
	float iparam; //ms
	float wparam; //sec
	long long sparam; //bytes
	uint16_t port;
	uint16_t listenp;
	float rparam; //ms
	NodeList *nodelist;
} ArgVals;

extern ArgFlags af;
extern ArgVals av;

void print_packetloss(char *hostname, double percentage, int nlost, bool rparam);
void print_hourly_stats(char *hostname,
						double percentage,
						double tmin,
						double tmax,
						double tavg,
						double tmdev);
void terminate();

#endif