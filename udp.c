/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: udp.c */
/* Description: Handles UDP protocol related functions */
#include "udp.h"

/* Function to prepare UDP packet to send */
uint8_t *prepare_udp_packet(int datalen, int seed){
	uint8_t *data = NULL;
	uint8_t *packet = NULL;
	//struct icmphdr *icmph;

	if(datalen > 0){
		data = allocate_ustrmem(datalen);
	}

	packet = allocate_ustrmem(IP_MAXPACKET);

	if((unsigned)datalen > sizeof(double)){
		generate_rnd_data(data, datalen, seed);
	}

	if((unsigned)datalen >= sizeof(double)){
		double timestamp = get_time();
		memcpy(data, &timestamp, sizeof(double));
	}

	memcpy(packet, data, datalen);

	if(data != NULL)
		free(data);
	return packet;
}

/* Function to send the udp packet */
int
send_udp(int Socket,
		 int datalen,
		 struct sockaddr* target,
		 size_t targetlen,
		 uint8_t *packet)
{
	int send_size;
	send_size = sendto( Socket,
						packet,
						datalen,
						0,
						target,
						targetlen);

	if(send_size < 0)
		throw_error(ENET, (strerror(errno)));
	return send_size;
}

/*********UDP LISTEN*********/
/* Function to listen on the desired port */
int l_udp_listen(	int Socket,
					struct sockaddr *client_addr,
					size_t cl_addrlen,
					char *buffer,
					size_t bufflen)
{
	int recv_size;
	recv_size = receive_icmp_reply(Socket, client_addr, cl_addrlen, buffer, bufflen, 0);

	return recv_size;
}

/* Function to send a UDP reply */
int l_udp_reply(int Socket,
				struct sockaddr *client_addr,
				size_t cl_addrlen,
				char *buffer,
				size_t bufflen)
{
	int send_size;
	send_size = sendto(Socket, buffer, bufflen, 0, client_addr, cl_addrlen);

	if(send_size < 0){
		throw_error(ENET, (strerror(errno)));
	}
	return send_size;
}
