/* ISA 2017/2018 */
/* Project Měření ztrátovosti a RTT */
/* Author: xpacak01, Gabriela Pacáková */
/* File: testovac.c */
/* Description: Handles forking and the program flow */
#include "testovac.h"

ArgFlags af;
ArgVals av;
volatile sig_atomic_t done = 0;

int main(int argc, char* argv[]){

	/*Parsing args*/
	av.nodelist = NULL;
	int ret = parse_args(argc,argv);

	if (ret < 0){
		throw_error(EARGS, "Invalid argument values.");
	}

	NodeList * head = av.nodelist;

	int Socket = 0, seq_id, echo_id, nreceived, nsent, nlost, nchild, reply;
	int ntreshold = 0;
	struct icmphdr *icmph = NULL;
	uint8_t *echo_request = NULL;

	double wtime = av.wparam;
	if (af.f_wparam)
		wtime = av.wparam * 1000;

	double tmin = 9999999, tmax = 0, tavg = 0, tmdev = 0;
	double tsum = 0, tsum2 = 0;
	char *revDNS = allocate_strmem(100);
	double rtt, tsent, tsent2;

	double hourly_stats_time = get_time(); //1 hour = 3600000 ms
	double packetloss_info_time = get_time();
	double interval;
	double ptreshold, percentage;

	pid_t PID = getpid();
	echo_id = PID;
	nchild = 1;

	struct sockaddr_in target4;
	struct sockaddr_in6 target6;

	/* To catch SIGTERM */
	struct sigaction action;
	sigset_t block_mask;
    //memset(&action, 0, sizeof(struct sigaction));
    sigfillset(&block_mask);
    action.sa_handler = terminate;
    //sigemptyset(&action.sa_mask);
    action.sa_mask = block_mask;
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

	/***** CORE *****/
	/* If we have any nodes in the list, we're going to test them */
	/* We fork the process for every node, so we can test them paralelly */
	while(head != NULL){
		PID = fork();
		echo_id += nchild;
		nchild++;

		/* CHILD process begins to  recieve and send replies on a given node */
		/* Creates all the needed structures, file descriptors and packets */
		if(PID == 0){

			interval = av.iparam;
			nreceived = 0;
			nlost = 0;
			nsent = 0;
			seq_id = 1;

			if(head->ai_family == AF_INET){ /* IPV4 */
				if(av.sparam > 1456) //MTU 1500-28-16
					av.sparam = 1456;
				Socket =  create_socket_4(af.f_udp);
				set_sockopt(Socket, AF_INET);
				target4.sin_family = AF_INET;
				target4.sin_port = htons(av.port);
				inet_pton(AF_INET, head->ip_str, &target4.sin_addr.s_addr);
				connect_socket(Socket, (struct sockaddr *)&target4, sizeof(target4));
			}
			else{ /* IPV6 */
			if(av.sparam > 1436)
					av.sparam = 1436;  //MTU 1500-48-16
				Socket =  create_socket_6(af.f_udp);
				set_sockopt(Socket, AF_INET6);
				target6.sin6_family = AF_INET6;
				target6.sin6_port = htons(av.port);
				inet_pton(AF_INET6, head->ip_str, &target6.sin6_addr.s6_addr);
				connect_socket(Socket, (struct sockaddr *)&target6, sizeof(target6));
			}

			/* Reverse DNS lookup to print out the hostname */
			revDNS = reverse_lookup(head->ip_str);

			icmph = (struct icmphdr *)allocate_strmem(sizeof(struct icmphdr));
			double tparam_ms = av.tparam * 1000;

    		/*Should break out of on SIGTERM */
			while(!done){
				/* ICMP/UDP LOOP*/
				/* Continues this loop until -t time is up */
				while(get_time() - packetloss_info_time < tparam_ms && !done){

					if(head->ai_family == AF_INET)
					{
						if(af.f_udp){ /*IPv4 UDP*/
							echo_request = prepare_udp_packet(av.sparam, seq_id);
							send_udp(Socket, av.sparam, (struct sockaddr *)&target4, sizeof(target4), echo_request);
						}
						else{ /*IPv4 ICMP*/
							echo_request = prepare_icmp_packet(icmph, av.sparam, seq_id, echo_id, head->ai_family);
							send_icmp(Socket, av.sparam, (struct sockaddr *)&target4, sizeof(target4), echo_request);
						}
					}
					else
					{
						if(af.f_udp){ /*IPv6 UDP*/
							echo_request = prepare_udp_packet(av.sparam, seq_id);
							send_udp(Socket, av.sparam, (struct sockaddr *)&target6, sizeof(target6), echo_request);
						}
						else{ /*IPv6 ICMP*/
							echo_request = prepare_icmp_packet(icmph, av.sparam, seq_id, echo_id, head->ai_family);
							send_icmp(Socket, av.sparam, (struct sockaddr *)&target6, sizeof(target6), echo_request);
						}
					}

					/* If the packet data size is less than sizeof double and we can't insert timestamp*/
					/* We save the time sent in a variable */
					if((unsigned)av.sparam < sizeof(double)){
						tsent2 = get_time();
					}
					nsent += 1;

				wait_for_reply:
					reply = wait_for_reply(Socket, wtime, echo_id, seq_id, &tsent, af.f_udp, echo_request, av.sparam);

					/* If the packet is to be ignored from the recv queue, */
					/* process goes back to recv queue, leaving the packet to be picked up */
					/* by another child process*/
					/* This only happens if 2 child processes are pinging the same host */
					/* Completely under control */
					if(reply == IGNORE){
						goto wait_for_reply;
					}
					/* The packet we received is the one we're waiting for */
					/* Calculates RTT, next wait time (2*RTT),... */
					else if(reply == SUCCESS){
						nreceived += 1;
						rtt = get_time() - tsent;
						if((unsigned)av.sparam < sizeof(double)){
							rtt = get_time() - tsent2;
						}

						if(af.f_rparam && rtt > av.rparam)
							ntreshold += 1;

						wtime = 2 * rtt;

						if(rtt < tmin)
							tmin = rtt;

						if(rtt > tmax)
							tmax = rtt;

						tsum += rtt;
						tsum2 += rtt * rtt;

						/* If verbose output is enabled */
						if(af.f_verbose){
							fflush(stdout);
							print_tdstring();
							fprintf(stdout, "%lld bytes from %s (%s) time=%.2f ms\n", av.sparam, revDNS, head->ip_str, rtt);
						}
					}
					/* Calculates the interval to send the next packet */
					/* See manual.pdf for more info */
					av.iparam >= wtime ? (interval = av.iparam - wtime) : (interval = 0);
					//fprintf(stdout, "wtime:%f\n", wtime);
					//fprintf(stdout, "interval:%f\n", interval);
					struct timespec tim, tim2;

					if(interval >= 1000){
						tim.tv_sec = interval / 1000;
						tim.tv_nsec = ((long int)interval % 1000) * 1000000;
						//tim.tv_nsec = (fmod(interval, 1000.0)) * 1000000;
					}
					else{
						tim.tv_sec = 0;
						tim.tv_nsec = interval * 1000000;
					}
					//fprintf(stdout, "sec:%ld\n", tim.tv_sec);
					//fprintf(stdout, "nsec:%ld\n", tim.tv_nsec);
					nanosleep(&tim, &tim2);

					free(echo_request);
					seq_id += 1;
				}
				/* END OF ICMP/UDP WHILE */

				/* Calculate statistics */
				tavg = tsum / nreceived;

				nlost = nsent - nreceived;
				percentage = ((double)nlost / (double)nsent) * 100;

				tsum /= nreceived;
				tsum2/= nreceived;
				tmdev = sqrt(tsum2 - tsum * tsum);

				/* Print statistics */
				/* Also set the timer for next statistics printout */
				print_packetloss(revDNS, percentage, nlost, false);
				if(af.f_rparam){
					ptreshold = ((double)ntreshold / (double)nsent) * 100;
					print_packetloss(revDNS, ptreshold, ntreshold, true);
				}
				packetloss_info_time = get_time();

				if(get_time() - hourly_stats_time >= /*60000*/3600000){
					hourly_stats_time = get_time();
					print_hourly_stats(revDNS, percentage, tmin, tmax, tavg, tmdev);
				}
			}
			/*print_packetloss(revDNS, percentage, nlost, false);
			if(af.f_rparam){
				ptreshold = ((double)ntreshold / (double)nsent) * 100;
				print_packetloss(revDNS, ptreshold, ntreshold, true);
			}*/
			print_hourly_stats(revDNS, percentage, tmin, tmax, tavg, tmdev);

			free(icmph);
			kill(getppid(), SIGUSR1);
			exit(0);
		}
		/* Parent  just loops over nodes in the list*/
		else if(PID > 0){
			head = head->next;
		}
		/* In case there is a forking error */
		else{
			throw_error(EFORK, (strerror(errno)));
		}

	}

	/* Parent listens on the desired port, if -l is in args */
	if(PID > 0 && af.f_listenp){

		int rcv;
		int lSocket;
		struct sockaddr_storage client;
		char rcvbuf[IP_MAXPACKET];
		int lchildnum = 0;

		lSocket = l_create_socket();
		l_set_sockopt(lSocket);
		l_bind_socket(lSocket, av.listenp);

		/* done should catch SIGINT */
		while(!done){
			rcv = l_udp_listen( lSocket,
								(struct sockaddr *)&client, sizeof(client),
								rcvbuf,
								sizeof(rcvbuf));

			if(rcv == 0)
				continue;

			/* concurrent UDP */
			pid_t LPID = fork();
			lchildnum++;

			/* Parent keeps on listening */
			if(LPID > 0){
				continue;
			}
			/* Children reply on the incoming packets from their host */
			else if(LPID == 0){
				l_udp_reply(lSocket, (struct sockaddr *)&client, sizeof(client), rcvbuf, rcv);
				exit(0);
			}
			/* Forking error */
			else{
				throw_error(EFORK, (strerror(errno)));
			}
		}
		/* Listening parent should wait for all the children to finish */
		for(int i = 0; i <= lchildnum; i++){
			wait(NULL);
		}
		close(lSocket);
	}

	/* Waiting for node children to finish */
	for(int i = 0; i <= nchild; i++){
		wait(NULL);
	}

	/* Free the list of nodes */
	if(av.nodelist != NULL){
		nodelist_free(av.nodelist);
	}
	if(echo_request != NULL){
		free(echo_request);
	}
	close(Socket);
	return 0;
}

/* Prints packet loss and RTT treshold statistics */
void print_packetloss(char *hostname, double percentage, int nlost, bool rparam){
	if(rparam){
		print_tdstring();
		fprintf(stdout, "%s: %.3f%% (%d) packets exceeded RTT treshold %.1f ms\n", hostname, percentage, nlost, av.rparam);
	}
	else{
		print_tdstring();
		fprintf(stdout, "%s: %.3f%% packet loss, %d packet lost\n", hostname, percentage, nlost);
	}
}

/*Prints hourly statistics about packet RTTs */
void print_hourly_stats(char *hostname,
						double percentage,
						double tmin,
						double tmax,
						double tavg,
						double tmdev)
{
	print_tdstring();
	if(percentage == 100.){
		fprintf(stdout, "%s: status down\n", hostname);
	}
	else{
		if(isnan(tmdev)){
			fprintf(stdout,
					"%s: %.3f%% packet loss, rtt min/max/avg/mdev %.3f/%.3f/%.3f/--- ms\n",
					hostname, percentage, tmin, tmax, tavg);
		}
		else{
			fprintf(stdout,
					"%s: %.3f%% packet loss, rtt min/max/avg/mdev %.3f/%.3f/%.3f/%.3f ms\n",
					hostname, percentage, tmin, tmax, tavg, tmdev);
		}

	}
}

/* When SIGINT is caught */
void terminate(void){
	//printf("Caught signal %d\n",s);
	done = 1;
}
