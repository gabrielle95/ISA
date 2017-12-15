/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: udp.h - header file for udp.c */
#ifndef UDP_H
#define UDP_H

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include <linux/in6.h>
#include <netinet/icmp6.h>

#include "socket.h"
#include "icmp.h"
#include "memops.h"
#include "argparse.h"
#include "utils.h"
#include "testovac.h"

uint8_t *prepare_udp_packet(int datalen, int seed);
int send_udp(int Socket,
			 int datalen,
		 	 struct sockaddr* target,
		 	 size_t targetlen,
		 	 uint8_t *packet);

int l_udp_listen(int Socket,
				 struct sockaddr *client_addr,
				 size_t cl_addrlen,
				 char *buffer,
				 size_t bufflen);

int l_udp_reply(int Socket,
				struct sockaddr *client_addr,
				size_t cl_addrlen,
				char *buffer,
				size_t bufflen);
#endif