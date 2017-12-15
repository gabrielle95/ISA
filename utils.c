/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: utils.c*/
/* Descrition: Contains utility functions like DNS conversion and time */
#include "utils.h"

/**************HOSTNAME TO IP ***********/
/* Converts hostname to IP adress if possible*/
/* Detects IP version */
struct addrinfo *hostname_to_ip(char *hostname)
{
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;

    if(af.f_udp){ //UDP -u USED
    	hints.ai_socktype = SOCK_DGRAM;
    }
    else{ //RAW ICMP PROTOCOL
    	hints.ai_socktype = SOCK_RAW;
	}

	if ((getaddrinfo(hostname , NULL , &hints , &servinfo)) != 0){
        	return NULL;
    }

	return servinfo;
}

/*********** REVERSE LOOKUP*************/
/* DNS lookup - IP back to hostname, if possible */
char *reverse_lookup(char *ip){ //should work for both v4 and v6
    struct addrinfo *result;
    struct addrinfo *res;
    int error;
    /* resolve the domain name into a list of addresses */
    error = getaddrinfo(ip, NULL, NULL, &result);
    if (error != 0){
        freeaddrinfo(result);
        return NULL;
    }
    /* loop over all returned results and do reverse lookup */
    for (res = result; res != NULL; res = res->ai_next)
    {
        char *hostname = malloc(150 * sizeof(char));
        memset(hostname,0,150);

        error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
        if (error != 0)
            continue;

        if (strcmp(hostname, "\0") != 0){
            freeaddrinfo(result);
            return hostname;
        }
        free(hostname);
    }
    freeaddrinfo(result);
    return NULL;

}

/**********TIME*********************/
/* Gets the time in ms with the most possible precision */
double get_time() {
    struct timeval tv;
    double d;

    gettimeofday (&tv, NULL);
    d = ((double) tv.tv_usec) / 1000. + (unsigned long) tv.tv_sec * 1000;

    return d;
}

/* Prints the date and time string in the required format */
void print_tdstring(){
	time_t timer;
	char buffer[26];
	struct tm* tm_info;
	int centisec;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	centisec = lrint(tv.tv_usec/10000.0);
	if(centisec >=10000){
		centisec -= 10000;
		tv.tv_sec++;
	}

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	fprintf(stdout, "%s.%d ", buffer, centisec);
}