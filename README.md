# ISA - Network Applications and Network Administration assignment
Parallel ping tool using BSD sockets. Created as a university assignment. Not recommended to use as an inspiration (for anything).

# Platform and language
Linux 32/64 bit. Implemented using C language.

# Compilation
Use "make all" with the attached Makefile. GCC required.
For more information, please read manual.pdf.
Use "man -l testovac.1" to read the manual page.

# Description
The basic purpose of "testovac" is to test a number of chosen hosts in the network by sending echo packets and waiting for the replies from the hosts. It also allows to listen on the desired UDP port for UDP communication, and send UDP packets to the host instead of ICMP packets. Every -t seconds it provides packet loss statistics (default 300s), and every hour it prints statistics about the average and mdev RTT's of each host. Use ctrl-c to exit the application in the shell. It will print out all the statistics just before termination. For related bugs, please see manual.pdf or the man page.

# Examples of execution

./testovac -i 1000 -r 20 -v www.google.com www.facebook.com www.yahoo.com

ONLY WORKS AS ROOT. Using raw sockets. Will test Google, Facebook and Yahoo, pinging them every 1000ms (1s). Verbose flag -v assures the printout of received echo reply packets. Packets with RTT greater than 20ms will be reported in the statistics report every 300s or until termination of the application.

./testovac -u -p 8000 -v www.google.com localhost

UDP works without root priviledges. Will send UDP packets to Google and to yourself filled with random data. Port 8000 is chosen in this exapmle.

./testovac -l 8000

Will listen on all interfaces at localhost on port 8000.

For more options see ./testovac -h, man page or manual.pdf.
