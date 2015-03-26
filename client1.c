/*
This is client 1
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
#define UDPPORT "31107"      // the UDP port used to send request
#define C_UDPPORT "32107"    // the UDP port of Client 1
#define REQMSG "Client1 doc1"    // registration message sending to the directory server
#define MAXBUFLEN 100

int contactServer(char *addr[],char tcpport[],int serverNum){
    int sockfd, numbytes;  
	char buf[MAXBUFLEN];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	unsigned short port;
	struct sockaddr_in my_addr;
	int my_addr_len;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(addr[0], tcpport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}
	
	getsock_check=getsockname(sockfd,(struct sockaddr *)&my_addr,&my_addr_len);
	if (getsock_chech == -1){
	    perror("getsockname");
		exit(1);
	}
	port = ntohs(my_addr->sin_port);
    printf("Phase 3:Client 1 has dynamic TCP port number %d and IP address %s.\n",port,inet_ntoa(my_addr->sin_addr));
	
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	
	freeaddrinfo(servinfo); // all done with this structure
	
	if (send(sockfd, REQMSG, 12, 0) != -1)
		//perror("send");
        printf("Phase 3:The file request from Client 1 has been sent to the File Server %d.\n",serverNum);
		
	if ((numbytes = recv(sockfd, buf, MAXBUFLEN-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	printf("Phase 3:Client 1 received %s from File Server %d\n",buf,serverNum);
	printf("Phase 3:End of Phase 3 for Client 1.\n");
	
	close(sockfd);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
    int rv;
    int numbytes,recvnumbytes;
	char buf[MAXBUFLEN],num[1],tcpport[6];

    if (argc != 2) {                          // Check whether the command line is correct
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

	printf("Phase 2:Client 1 has UDP port number %d and IP address \n", atoi(C_UDPPORT));      // On-screen message upon startup of phase 2
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], UDPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {                                          // If there is no socket that satisfy requirements in hints created.
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

	freeaddrinfo(servinfo);
	
    if ((numbytes = sendto(sockfd, &REQMSG, strlen(REQMSG), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
	
	// On-screen message after the file request is sent to the directory server.
	printf("Phase 2:The File request from Client 1 has been sent to the Directory Server.\n");
	
	/*addr_len = sizeof their_addr;
    if ((recvnumbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,                  // read from the directory server and store in "buf"
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {                    // The directory server can send the information of file server
        perror("recvfrom");                                                     // that has the requested doc. But there are some problems with 
        exit(1);                                                                // recvfrom function. So I use a fixed message here. 
		                                                                        // Actually, this client can handle the received message dynamically.
    }
	printf("%s\n",buf);*/
	char buf1[] = "File_Server1 41107";
	strncpy(tcpport,&buf1[13],5);
	strcpy(&tcpport[5],"\0");
	num [0] = buf1[11];
	int serverNum = atoi (num);
    
    close(sockfd);
	
	printf("Phase 2:The File requested by Client 1 is present in File Server %d and the File Server's TCP port number is %s.\n",serverNum,tcpport);
	printf("Phase 2:End of Phase 2 for Client 1.\n");      // On-screen message at the end of phase 2
	contactServer(&argv[1],tcpport,serverNum);
	
    return 0;
}
