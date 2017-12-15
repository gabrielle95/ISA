/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: argparse.c */
/* Description: Handles argument parsing */
#include "argparse.h"

int parse_args(int argc, char *argv[]){

	/* Set all to default values */
	av.sparam = SPARAM_DEF;
	av.iparam = IPARAM_DEF;
	av.wparam = WPARAM_DEF;
	av.tparam = TPARAM_DEF;

	for(int i = 0; i < argc; i++){
		if(strcmp(argv[i], "-h") == 0){
			display_help();
			exit(0);
		}

		if(strcmp(argv[i], "./testovac") == 0)
			continue;
		else if(strcmp(argv[i], "-u") == 0)
			af.f_udp = 1;
		else if(strcmp(argv[i], "-t") == 0){
			af.f_tparam = 1;

			if(strtofl(argv[i+1], &av.tparam) != 1){
				return EARGVAL;
			}
			i++;
		}
		else if(strcmp(argv[i], "-w") == 0){
			af.f_wparam = 1;

			if(strtofl(argv[i+1], &av.wparam) != 1){
				return EARGVAL;
			}
			i++;
		}
		else if(strcmp(argv[i], "-i") == 0){
			af.f_iparam = 1;
			if(strtofl(argv[i+1], &av.iparam) != 1){
				return EARGVAL;
			}
			i++;
		}
		else if(strcmp(argv[i], "-p") == 0){
			af.f_port = 1;
			long long temp = get_param_value(argv,i);
			if(temp >= 0 && temp < 65536){
				av.port = (uint16_t)temp;
				i++;
			}
			else
				return EARGVAL;

		}
		else if(strcmp(argv[i], "-l") == 0){
			af.f_listenp = 1;
			long long temp = get_param_value(argv,i);
			if(temp >= 0 && temp < 65536){
				av.listenp = (uint16_t)temp;
				i++;
			}
			else
				return EARGVAL;
		}
		else if(strcmp(argv[i], "-s") == 0){
			af.f_sparam = 1;
			av.sparam = get_param_value(argv, i);
			if(av.sparam > IP_MAXPACKET - IP_HDRLEN - ICMP_HDRLEN)
				av.sparam = IP_MAXPACKET - IP_HDRLEN - ICMP_HDRLEN;
				/*make this frame lenght */
			i++;
		}
		else if(strcmp(argv[i], "-r") == 0){
			af.f_rparam = 1;
			//i++;
			if(strtofl(argv[i+1], &av.rparam) != 1){
				return EARGVAL;
			}
			i++;
		}
		else if(strcmp(argv[i], "-v") == 0)
			af.f_verbose = 1;
		else{ //NODE PARSING

			if(af.f_udp && !af.f_port){
				fprintf(stderr, "E: Port (-p) must be supplied when using UDP protocol (-u).");
				return EARGS;
			}
			if(av.tparam == -1 || av.iparam == -1 || av.wparam == -1 || av.rparam == -1 || av.sparam == -1)
				return EARGVAL;

			struct addrinfo *servinfo = NULL;
			servinfo = hostname_to_ip(argv[i]);
			if (servinfo == NULL){
				fprintf(stderr, "Testovac error: Can't resolve %s.\n", argv[i]);
				continue;
			}
			else{
				struct sockaddr_in *h4;
    			struct sockaddr_in6 *h6;
				if(servinfo->ai_family == AF_INET){ //IPv4
        			h4 = (struct sockaddr_in *) servinfo->ai_addr;
        			char *ip4 = allocate_strmem(INET_ADDRSTRLEN+1);
            		for (int i = 0; i < INET_ADDRSTRLEN +1; i++)
    					ip4[i] = '\0';
        			inet_ntop(servinfo->ai_family, &(h4->sin_addr), ip4, INET_ADDRSTRLEN);
        			av.nodelist = nodelist_push(av.nodelist, ip4, AF_INET);
    			}
    			else if(servinfo->ai_family == AF_INET6){ //IPv6
        			h6 = (struct sockaddr_in6 *) servinfo->ai_addr;
        			char *ip6 = allocate_strmem(INET6_ADDRSTRLEN+1);

        			for (int i = 0; i < INET6_ADDRSTRLEN +1; i++)
    					ip6[i] = '\0';
        			inet_ntop(servinfo->ai_family, &(h6->sin6_addr), ip6, INET6_ADDRSTRLEN);
        			av.nodelist = nodelist_push(av.nodelist, ip6, AF_INET6);
    			}
			}
		}
		if(av.tparam == -1 || av.iparam == -1 || av.wparam == -1 || av.rparam == -1 || av.sparam == -1)
			return EARGS;

		if(af.f_udp && af.f_sparam){
			if(av.sparam <= 0){
				fprintf(stderr, "ERROR: Invalid UDP data size. (Must be greater than 0)\n");
				return EARGS;
			}
		}
	}
	if(!af.f_sparam && af.f_udp)
		av.sparam = SUDP_DEF;
	return 0;
}

/* Get the numeric value of the parameter */
long long get_param_value(char *argv[], int i){
	if(argv[i+1] != NULL){
		char *tstr = argv[i+1];
		for(int a = 0; a < (int)strlen(tstr); a++){
			if(!isdigit(tstr[a])){
				return -1;
			}
		}
		return strtol(argv[i+1], NULL, 10);
	}
	return -1;
}

/* String to float */
int strtofl(const char* str, float *dest){
	if(str == NULL) {return 2;}
	char temp;
	return sscanf(str,"%f %c", dest, &temp) == 1;
}

/* ERROR */
void throw_error(int code, char *msg){
    fflush(stdout);
    fprintf(stderr, "TESTOVAC error: %s\n", msg);
    if(av.nodelist != NULL){
    	nodelist_free(av.nodelist);
    }
    exit(code);
}

/* HELP */
void display_help(){
    fprintf(stdout,
    		"\n"
    		"Program: testovac (ISA - Měření ztrátovosti a RTT)\n"
    		"Author: Gabriela Pacakova, xpacak01\n"
            "Date: Sep 25, 2017\n"
            "Usage: ./testovac [options] [target list...]\n"
            "Note: Requires root access if you're not using UDP (-u)\n"
            "Options:\n"
            "[-h] - displays this help\n"
            "[-u] - UDP protocol will be used for testing\n"
            "[-t <interval>] - in seconds, interval after which the packet loss statistic is displayed, default 300 s\n"
            "[-i <interval>] - in milliseconds, wait \"interval\" milliseconds between sending each packet, default 100 ms\n"
            "[-p <port>] - used with UDP, sets which port to use to send UDP packets to\n"
            "[-l <port>] - program will listen on this port and reply on incoming UDP messages\n"
            "[-s <size>] - size of packet data, ICMP default 56 bytes, UDP default 64 bytes\n"
            "[-r <value>] - in milliseconds, RTT treshold. Packets over treshold are reported if -r is set\n"
            "[-w <timeout>] - in seconds, sets  the timeout for the first packet, after that it's 2xRTT \n"
            "[-v] - verbose mode, displays received packets (similar to ping)\n"
            "\n"
            "Target list:\n"
            "<target> - IPv4 / IPv6 / hostname of the target\n"
            "\t - multiple targets must be separated by whitespace\n"
            "\n"
            );
}