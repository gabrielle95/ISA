/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: argparse.h - header file for argparse.c */
#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "memops.h"
#include "testovac.h"

//APPLICATION ERROR CODES
#define EARGS -1
#define EARGVAL -2
#define EFORK -3
#define ESOCK -4
#define ENET -5
#define EHELP -6


int parse_args(int argc, char *argv[]);
long long get_param_value(char *argv[], int i);
int strtofl(const char* str, float *dest);
void throw_error(int code, char *msg);
void display_help();

#endif