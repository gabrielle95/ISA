/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: memops.c */
/* Description: Contains functions handling memory operations */
#include "memops.h"

/* LIST OPERATIONS */

NodeList *nodelist_push(NodeList * List, char *data, int Family){
	NodeList *temp = malloc(sizeof(NodeList));

	temp->ip_str = data;
	temp->ai_family = Family;
	temp->next = NULL;

	if(List == NULL)
		List = temp;
	else if(List->next == NULL)
		List->next = temp;
	else{
		NodeList *Head = List;
		while(Head->next != NULL)
			Head = Head->next;
		Head->next = temp;
	}
	return List;
}

void nodelist_free(NodeList * List){
	NodeList *tmp;

	while(List != NULL){
		tmp = List;
		List = List->next;
		free(tmp->ip_str);
		free(tmp);
	}
}

/* MALLOCS */

/* Following borrowed from P.D. Buchan (pdbuchan@yahoo.com)
 * under GNU General Public License. */

// Allocate memory for an array of chars.
char *allocate_strmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *allocate_ustrmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int *allocate_intmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (int *) malloc (len * sizeof (int));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (int));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
    exit (EXIT_FAILURE);
  }
}