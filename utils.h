/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: utils.h - header file for utils.c */
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "memops.h"
#include "testovac.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

struct addrinfo *hostname_to_ip(char *hostname);
char *reverse_lookup(char *ip);
double get_time();
void print_tdstring();

#endif