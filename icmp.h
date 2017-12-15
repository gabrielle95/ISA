/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: icmp.h - header file for icmp.c */
#ifndef ICMP_H
#define ICMP_H

#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include <linux/in6.h>
#include <netinet/icmp6.h>

#include "socket.h"
#include "memops.h"
#include "argparse.h"
#include "utils.h"
#include "testovac.h"

#define FAILURE 0
#define SUCCESS 1
#define IGNORE 2

uint16_t ip_checksum(const void *buf, size_t hdr_len);
uint8_t *prepare_icmp_packet(struct icmphdr *icmph, int datalen, int seq_id, int echo_id, int Family);
void generate_rnd_data(uint8_t *target, int datalen, int seed);
int send_icmp(int Socket, int datalen, struct sockaddr* target, size_t targetlen, uint8_t *packet);

int wait_for_reply( int Socket,
					double wtime,
					int echo_id,
					int seq_id,
					double *timestamp,
					bool f_udp,
					uint8_t *sent_packet,
					int datalen);

int receive_icmp_reply( int Socket,
				    	struct sockaddr *reply_addr,
				    	size_t reply_addr_len,
				    	char *buffer,
				    	size_t bufflen,
				    	int flags);

int retrieve_ids_icmp(int *echo_id,
					 	int *seq_id,
					 	int Family,
					 	char *buffer);

double retrieve_timestamp(int Family, char *buffer, bool udp);
#endif