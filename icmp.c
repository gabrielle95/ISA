/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: icmp.c */
/* Description: Handles ICMP protocol functions to send and receive ICMP echo packets */
#include "icmp.h"

/* Calculates checksum for ICMP */
uint16_t ip_checksum(const void *buf, size_t hdr_len){
	unsigned long sum = 0;
    const uint16_t *ip1;

    ip1 = buf;
    while (hdr_len > 1)
    {
        sum += *ip1++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        hdr_len -= 2;
    }
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return(~sum);
}

/* Prepares an ICMP packet */
uint8_t *prepare_icmp_packet(struct icmphdr *icmph, int datalen, int seq_id, int echo_id, int Family){

	uint8_t *data = NULL;
	uint8_t *packet = NULL;

	if(datalen > 0){
		data = allocate_ustrmem(datalen);
	}

	packet = allocate_ustrmem(IP_MAXPACKET);

	if((unsigned)datalen < sizeof(double) && datalen != 0){
		generate_rnd_data(data, datalen, seq_id);
	}
	if((unsigned)datalen > sizeof(double)){
		generate_rnd_data(data, datalen, seq_id);
	}

	if((unsigned)datalen >= sizeof(double)){
		double timestamp = get_time();
		memcpy(data, &timestamp, sizeof(double));
	}

    Family == AF_INET ? (icmph->type = ICMP_ECHO) : (icmph->type = ICMPV6_ECHO);
    icmph->code = 0;
    icmph->checksum = 0;
    icmph->un.echo.sequence = seq_id;
    icmph->un.echo.id = echo_id;

    /* COPY ICMP HEADER TO PACKET */
    memcpy(packet, icmph, ICMP_HDRLEN);

    /* COPY PAYLOAD DATA AFTER HEADER TO PACKET */
    if(data != NULL)
    	memcpy(packet + ICMP_HDRLEN, data, datalen);

    /* CALCULATE CHECKSUM OF ALL AND COPY TO PACKET */
    icmph->checksum = ip_checksum(packet, ICMP_HDRLEN+datalen);
    memcpy(packet, icmph, ICMP_HDRLEN);

    if(data != NULL)
    	free(data);
    return packet;
}

/* Function to generate random data for the packet */
void generate_rnd_data(uint8_t *target, int datalen, int seed){
	time_t t;

	srand((unsigned) time(&t) + (unsigned)seed);

	for(int i = 0; i < datalen - 1; i++){
		target[i] = rand();
	}
	target[datalen-1] = '\0';
}

/* Function to send the ICMP packet */
int
send_icmp(	int Socket,
			int datalen,
			struct sockaddr* target,
			size_t targetlen,
			uint8_t *packet)
{

	int send_size;
	send_size = sendto(	Socket,
						packet,
						ICMP_HDRLEN + datalen,
						0,
						target,
						targetlen
					  );

	if(send_size < 0){
		throw_error(ENET, (strerror(errno)));
	}
	return send_size;
}

/* Function that waits and handles the ICMP reply */
int
wait_for_reply( int Socket,
				double wtime,
				int echo_id,
				int seq_id,
				double *timestamp,
				bool f_udp,
				uint8_t *sent_packet,
				int datalen)
{
	struct timeval timeout;
	int sock = 0;

	/* Conversion of wait time into timeval struct */
	if(wtime >= 1000){
		timeout.tv_sec = wtime / 1000;
		timeout.tv_usec = ((int)wtime % 1000) * 1000;
	}
	else{
		timeout.tv_sec = 0;
		timeout.tv_usec = wtime * 1000;
	}

	sock = socket_timeout(Socket, &timeout);
	if(sock == 0){
		return FAILURE; //timed out
	}

	struct sockaddr_storage reply_addr; //can be from IPV4 or IPV6
	int recv_size;
	char buffer[IP_MAXPACKET]; //recv_buffer

	//receive packet
	recv_size = receive_icmp_reply( Socket,
									(struct sockaddr *)&reply_addr,
									sizeof(reply_addr),
									buffer,
									sizeof(buffer),
									0);

	if(recv_size == 0){
		return FAILURE;
	}

	/* If it's a UDP, compares data and returns */
	if(f_udp){
		if(memcmp(&buffer, sent_packet, datalen) !=0){
			return IGNORE;
		}
		if((unsigned)datalen > sizeof(double)){
			*timestamp = retrieve_timestamp(reply_addr.ss_family, buffer, true);
		}
		return SUCCESS;
	}

	/* Compares sequence and echo IDs */
	int rep_echo_id, rep_seq_id;
	int ids;
	ids = retrieve_ids_icmp(&rep_echo_id, &rep_seq_id, reply_addr.ss_family, buffer);

	if(rep_echo_id != echo_id){
		return IGNORE; //ignore packet, not my echoid
	}

	/*my packet, but smaller sequence number than expected.
	  Arrived after timeout, I need to remove it from the msg queue */
	if(rep_seq_id != seq_id){
		if(rep_seq_id < seq_id){
			return IGNORE;
		}
	}

	if(ids == IGNORE){
		return IGNORE;
	}

	if(ids == FAILURE){
		return FAILURE; //didnt get type ECHO REPLY
	}

	/* All checks passed, retrieve timestamp */
	*timestamp = retrieve_timestamp(reply_addr.ss_family, buffer, false);

	return SUCCESS;
}

/* Receive the ICMP packet */
int
receive_icmp_reply( int Socket,
				    struct sockaddr *reply_addr,
				    size_t reply_addr_len,
				    char *buffer,
				    size_t bufflen,
				    int flags)
{
	int recv_size;
	unsigned char msg_ancillary[40];
	struct iovec iov = { buffer, bufflen};
	struct msghdr msghd = {reply_addr, reply_addr_len, &iov, 1, &msg_ancillary, sizeof(msg_ancillary), 0};

	recv_size = recvmsg(Socket, &msghd, flags);
	if(recv_size <= 0){
		return FAILURE;
	}

	return recv_size;
}

/* Retrieve the ICMP echo id and sequence id from the reply */
/* Also checks ICMP reply type */
int
retrieve_ids_icmp( 	int *echo_id,
					int *seq_id,
					int Family,
					char *buffer)
{
	struct icmphdr *icmp_rep = (struct icmphdr *)allocate_ustrmem(sizeof(struct icmphdr));

	if(Family == AF_INET){
		memcpy(icmp_rep, buffer + IP_HDRLEN, ICMP_HDRLEN);
	}
	else if(Family == AF_INET6){
		memcpy(icmp_rep, buffer, ICMP_HDRLEN);
	}

	*echo_id = icmp_rep->un.echo.id;
	*seq_id = icmp_rep->un.echo.sequence;

	/*If I'm pinging myself (localhost, etc), I need to throw away my own ECHO requests*/
	if(icmp_rep->type == ICMP_ECHO || icmp_rep->type == ICMPV6_ECHO){
		free(icmp_rep);
		return IGNORE;
	}

	/*Different icmp message then ECHO REPLY*/
	if(icmp_rep->type != ICMP_ECHOREPLY && icmp_rep->type != ICMPV6_ECHOREPLY){
		free(icmp_rep);
		return FAILURE;
	}

	free(icmp_rep);
	return SUCCESS;
}

/* Retrieve the timestamp from the reply */
double retrieve_timestamp(int Family, char *buffer, bool udp){
	double timestamp = 0;

	if(udp){
		memcpy(&timestamp, buffer, sizeof(double));
		return timestamp;
	}

	if(Family == AF_INET)
		memcpy(&timestamp, buffer + IP_HDRLEN + ICMP_HDRLEN, sizeof(double));
	else
		memcpy(&timestamp, buffer + ICMP_HDRLEN, sizeof(double));

	return timestamp;
}