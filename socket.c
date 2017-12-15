/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: socket.c */
/* Description: Contains socket related functions */
#include "socket.h"

/* Create an IPv4 socket */
int create_socket_4(bool udp){

	int Socket;
	struct protoent *protocol;

	if(udp){
		Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(Socket < 0){
			throw_error(ESOCK, (strerror(errno)));
		}
	}
	else{
		if((protocol = getprotobyname("icmp")) == NULL)
			throw_error(ESOCK, "Unknown protocol \"icmp\"");
		/* ICMP */
		/*try to create raw socket */
		Socket = socket(AF_INET, SOCK_RAW, protocol->p_proto);
		//fprintf(stdout, "Protocol: %d\n", protocol->p_proto);
		/* if user isnt root*/
		if(Socket < 0){
			/* try to create datagram socket, no root required */
			Socket = socket(AF_INET, SOCK_DGRAM, protocol->p_proto);
			if(Socket < 0){
				throw_error(ESOCK, (strerror(errno)));
			}
		}
	}

	/* non blocking socket - using fcntl */

	int f;

	if((f = fcntl(Socket, F_GETFL, 0)) < 0){
		close(Socket);
		throw_error(ESOCK, (strerror(errno)));
	}

	if(fcntl(Socket, F_SETFL, f | O_NONBLOCK) < 0){
		close(Socket);
		throw_error(ESOCK, (strerror(errno)));
	}

	return Socket;
}

/* Create IPv6 socket */
int create_socket_6(bool udp){
	int Socket;
	struct protoent *protocol;

	if(udp){
		Socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		if(Socket < 0){
			throw_error(ESOCK, (strerror(errno)));
		}
	}
	else{
		/*ICMPv6*/
		if((protocol = getprotobyname("ipv6-icmp")) == NULL)
			throw_error(ESOCK, "Unknown protocol \"icmpv6\"");
		/*try to create raw socket */
		Socket = socket(AF_INET6, SOCK_RAW, protocol->p_proto);
		//fprintf(stdout, "Protocol: %d\n", protocol->p_proto);
		/* if user isnt root*/
		if(Socket < 0){
			/* try to create datagram socket, no root required */
			Socket = socket(AF_INET6, SOCK_DGRAM, protocol->p_proto);
			if(Socket < 0){
				throw_error(ESOCK, (strerror(errno)));
			}
		}
	}

	/* non blocking socket - using fcntl */

	int f;

	if((f = fcntl(Socket, F_GETFL, 0)) < 0){
		close(Socket);
		throw_error(ESOCK, (strerror(errno)));
	}

	if(fcntl(Socket, F_SETFL, f | O_NONBLOCK) < 0){
		close(Socket);
		throw_error(ESOCK, (strerror(errno)));
	}

	return Socket;
}

/* Set required socket options (TTL/HOP LIMIT to 255) */
void set_sockopt(int Socket, int Family){
	unsigned int ttl = 255;

	if(Family == AF_INET){
		if (setsockopt(Socket, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl))){
			throw_error(ESOCK, (strerror(errno)));
		}
	}
	else if(Family == AF_INET6){
		if (setsockopt(Socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (const char*)&ttl, sizeof(ttl))){
			throw_error(ESOCK, (strerror(errno)));
		}
	}
}

/* Connects the socket to the host */
/* Socket (UDP/RAW) will receive packets only from host it's connected to */
void connect_socket(int Socket, struct sockaddr* target, size_t targetlen){
	if(connect(Socket, target, targetlen) != 0){
		throw_error(ESOCK, (strerror(errno)));
	}
}

/* Handles socket timeout using select*/
int socket_timeout(int Socket, struct timeval *timeout){
	fd_set readset, writeset;

select:
	FD_ZERO(&readset);
	FD_ZERO(&writeset);
	FD_SET(Socket, &readset);

	int fdfound;

	fdfound = select(Socket+1, &readset, &writeset, NULL, timeout);
	if(fdfound < 0){
		/* In case of a system signal interrupt */
		if(errno == EINTR){
			goto select;
		}
		else{
			//perror("select: ");
		}
	}

	if(fdfound > 0){
		if(FD_ISSET(Socket, &readset)){
			return Socket;
		}
	}
	return 0;
}

/* UDP LISTEN socket functions */

int l_create_socket(){
	return create_socket_6(true);
}

void l_set_sockopt(int Socket){
	unsigned int ttl = 255;
	unsigned int one = 1;

	if (setsockopt(Socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (const char*)&ttl, sizeof(ttl))){
		throw_error(ESOCK, (strerror(errno)));
	}

	if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one))){
		throw_error(ESOCK, (strerror(errno)));
	}
}

void l_bind_socket(int Socket, int Port){
	struct sockaddr_in6 sbind;
	memset(&sbind, 0, sizeof(sbind));

	sbind.sin6_addr = in6addr_any;
	sbind.sin6_family = AF_INET6;
	sbind.sin6_port = htons(Port);

	if(bind(Socket, (struct sockaddr *)&sbind, sizeof(sbind)) < 0)
		throw_error(ESOCK, (strerror(errno)));
}