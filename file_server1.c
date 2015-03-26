/*
This is file server1
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define TCPPORT "41107"    // the TCP port users will be connecting to in phase 3
#define UDPPORT "21107"    // the UDP port used to register
#define S_UDPPORT "22107"  // the UDP port of File Server
#define REGMSG "File_Server1 41107"    // registration message sending to the directory server

#define MAXBUFLEN 100         // used when File Server 1 acts as the server
#define BACKLOG 10	 // how many pending connections queue will hold

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int contactClient(){
    int sockfd, new_fd,numbytes,i;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	//struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv,clientNum;
	char buf[MAXBUFLEN],num[1],req1[4],req2[4];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, TCPPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	printf("Phase 3:File Server 1 has TCP port %d and IP address 68.181.201.3\n",atoi(TCPPORT));
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}
	
	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	i = 0;
	while(i < 2) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		
		if ((numbytes = recv(new_fd, buf, MAXBUFLEN-1, 0)) == -1) {
	        perror("recv");
	        exit(1);
	    }
		if (numbytes != 0){
		    i = i+1;
		}
		
		strncpy(req1,&buf[8],4);
		strcpy(&req1[4],"\0");
		num[0] = buf[6];
		int clientNum = atoi (num);
		printf("Phase 3:File server 1 received the request from the Client %d for the file %s.\n", clientNum,req1);
		
		if (send(new_fd, req1, 4, 0) == -1)
			perror("send");
		printf("Phase 3:File Server 1 has sent %s to client %d.\n",req1,clientNum);
		close(new_fd);
	}
	close(sockfd); // child doesn't need the listener
}

int main(int argc, char *argv[])                                      /* need to input the IP addr and port number in the command line!!!!!!!! */
{
    int udp_sockfd,tcp_sockfd;
    struct addrinfo udp_hints, *udp_servinfo, *udp_p;
    int rv;
    int numbytes;
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if (argc != 2) {                                                  /* determine whether there are three input. eg. telnet nunki.usc.edu */
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }
	
    memset(&udp_hints, 0, sizeof udp_hints);                          /* clean the contents in udp_hints */
    udp_hints.ai_family = AF_UNSPEC;                                  /* the requirements stored in udp_hints. */
    udp_hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(argv[1], UDPPORT, &udp_hints, &udp_servinfo)) != 0) {           /* information about the created socket */
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	
	// loop through all the results and make a socket
    for(udp_p = udp_servinfo; udp_p != NULL; udp_p = udp_p->ai_next) {
        if ((udp_sockfd = socket(udp_p->ai_family, udp_p->ai_socktype,
                udp_p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }
	
	if (udp_p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
    
	printf("Phase 1:File Server 1 has UDP port nnumber %d and IP address ", atoi(S_UDPPORT));            // On-screen message of Server1. What's the IP address?????????????????????
	if ((he = gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        return 2;
    }
	addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
        printf("%s.\n", inet_ntoa(*addr_list[i]));
    }
	
    if ((numbytes = sendto(udp_sockfd, &REGMSG, strlen(REGMSG), 0,
             udp_p->ai_addr, udp_p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

	printf("Phase 1:The Registration request from File Server 1 has been sent to the Directory Server.\n");      //On-screen message after sending registration request.
	
    freeaddrinfo(udp_servinfo);
	
	printf("Phase 1:End of Phase 1 for File Server 1.\n");      // On-screen message at the end of phase 1
	
    close(udp_sockfd);
	
	contactClient();

    return 0;
}
