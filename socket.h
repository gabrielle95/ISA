/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: socket.h - header file for socket.c */
#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>

#include "argparse.h"
#include "memops.h"


int create_socket_4(bool udp);
int create_socket_6(bool udp);
void set_sockopt(int Socket, int Family);
void connect_socket(int Socket, struct sockaddr* target, size_t targetlen);
int socket_timeout(int Socket, struct timeval *timeout);
int l_create_socket();
void l_set_sockopt(int Socket);
void l_bind_socket(int Socket, int Port);

#endif