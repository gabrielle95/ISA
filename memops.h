/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: memops.h - header file for memops.c */
#ifndef MEMOPS_H
#define MEMOPS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

struct node{
	int ai_family;
	char *ip_str;
	struct node *next;
};

typedef struct node NodeList;

NodeList *nodelist_push(NodeList * List, char *data, int Family);
void nodelist_free(NodeList * List);

/* Following borrowed from P.D. Buchan (pdbuchan@yahoo.com)
 * under GNU General Public License. */
char *allocate_strmem (int len);
uint8_t *allocate_ustrmem (int len);
int *allocate_intmem (int len);

#endif