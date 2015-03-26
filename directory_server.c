/*
This is directory server
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

#define UDPPORT1 "21107"    // the port users(i.e. file servers) will be connecting to in phase 1
#define UDPPORT2 "31107"    // the port users(i.e. clients) will be connecting to in phase 2
#define REGMSG "File_Server1 41107"    // registration message sending to the directory server

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Function used to find the file server's TCP port number. s is the number of file server (i.e.1 or 2 or 3)
char* findPortNum(int s){
    FILE *fp;
	static char str1[65],str2[65],str3[65],file_s[12] = "File_Server";
	//char *token1, *token2, *token3;
	char tcpport[8],server[2];
	char *message;
	
	if ((fp = fopen("directory.txt", "rb")) == NULL){
	    printf("cannot open file\n");
		exit(1);
	}
	fgets(str1,30,fp);               // the strings read from this file contain "\n"
	fgets(str2,30,fp);
	fgets(str3,30,fp);
	//printf("%s",str1);
	//printf("%s",str2);
	//printf("%s",str3);
	//fread(str, sizeof(str), 1, fp);
	//strcpy(&str[65],"\0");
	//printf("%s\n",str);
	
	sprintf(server,"%d",s);
	//printf("%s\n",server);
	strcpy(&file_s[11],server);
	//printf("%s\n",file_s);
	//token1=strtok(str,"\n");  
	
	if (strstr(str1,file_s) != NULL){
	    //printf("%s",str1);
		//message = str1;
		return str1;
	}
	if (strstr(str2,file_s) != NULL){
	    //printf("%s",str2);
		//message = str2;
		return str2;
	}
	if (strstr(str3,file_s) != NULL){
	    //printf("%s",str3);
		//message = str3;
		return str3;
	}
    
}

// Function used to determine which file server is the closest.
char *determine(int c[],int clientNum,int count){
    FILE *fp;
	int cost[2][3];          
	int i,j,min,server;
	int comp[3];
	static char *sentMsg;
	
	if ((fp = fopen("topology.txt", "rb")) == NULL){
	    printf("cannot open file\n");
		exit(1);
	}
	
	for (i = 0; i < 2; i++){                        // Read the costs from topology.txt and store them in a two dimension array
	    for (j = 0; j < 3; j++){
		    fscanf(fp,"%d",&cost[i][j]);
		}
		//fscanf(fp,"\n");
	}
	
	fclose(fp);
	
	//printf("%d %d %d\n", c[0],c[1],c[2]);
	if (count > 1){                               // If threre are more than one file server have the requested doc.
	    for (i = 0; i < 3; i++){
	        if (!c[i]){
			    cost[clientNum-1][i] = 32767;     // Set the cost to be the maximum number
		    }
	    }
		
		// Compare the cost and choose the one who is the closest to the client
		min = cost[clientNum-1][0];
		server = 1;
		for (j = 1;j < 3; j++){
		    if (min > cost[clientNum-1][j]){ 
			    min = cost[clientNum-1][j]; 
				server = j+1;
			}
		}
		//printf("Finally, file server %d can give help to client %d.\n", server,clientNum);
	}else{                                         // If there is only one server has the requested doc.
	    for (i = 0; i < 3; i++){
		    if (c[i]){
			server = i+1;
	        //printf("There is only one file server has the requested file. It is file server %d.\n",server); }
		}
	}
	
	sentMsg = findPortNum(server);
	return sentMsg;
	/*for(i=0;i<2;i++)
	{
		for(j=0;j<3;j++)
		{
			printf("%d ",str[i][j]);
		}
		printf("\n");
	}*/
	//fread(str, sizeof(str), 1, fp);
	//printf("%s\n", str);
	
	
}

// Function used to find the file server(s) which can provide file.c
char *findFileServer(char req[], int clientNum){
    FILE *fp;
	char str[100];
	char *token1, *token2, *token3;
	int c[3]={0,0,0};                         // Used to record which file server has the requested doc.
	static char *sentMsg;
	int i = 0;
	char buf1[] = "doc1";
	char buf2[] = "doc2";
	
	// Open resource.txt to find file servers that have the requested files.
	if ((fp = fopen("resource.txt", "rb")) == NULL){
	    printf("cannot open file\n");
		exit(1);
	}
	
	//fgets(str, 100, fp);
	// Read data from resource.txt
	fread(str, sizeof(str), 1, fp);
	//printf("%s\n", str);
	
	// Truncate the whole file into 3 parts and store them in an array.
	// Look at the first line in resource.txt(i.e. check whether File Server1 has the requested file)
	token1=strtok(str,"\n");  
    if (token1)  
    {  
        //printf("%s\n",token1);
        if (strstr(token1,req) != NULL){
		    //printf("File Server 1 has the requirement file and the required file is %s.\n", req);
	        i = i+1;
			c[0] = 1;
		}
	} 
	
	// Look at the second line in resource.txt(i.e. check whether File Server2 has the requested file)
    token2=strtok(NULL,"\n");  
    if (token2)  
    {  
        //printf("%s\n",token2);
        if (strstr(token2,req) != NULL){
		    //printf("File Server 2 has the requirement file and the required file is %s.\n", req);
		    i = i+1;
		    c[1] = 1;
		}
    }  
    
	// Look at the last line in resource.txt(i.e. check whether File Server3 has the requested file)
	token3=strtok(NULL,"\n");  
    if (token3)  
    {  
        //printf("%s\n",token3);  
        if (strstr(token3,req) != NULL){
		    //printf("File Server 3 has the requirement file and the required file is %s.\n", req);
			i = i+1;
			c[2] = 1;
		}
    }  
	
	// If a requested file of client1 is present in multiple file server
	//if (i > 1){
	//printf("%d %d %d\n",c[0],c[1],c[2]);
	sentMsg = determine(c,clientNum,i);
	return sentMsg;
	//}
	
	fclose(fp);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes, sentnumbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
	struct hostent *he;
	struct in_addr **addr_list;
	int i = 0;
	char num[1],req2[10];
	char req1[10];
	char *sentMsg;
    
	// On-screen message upon startup of phase 1.
	printf("Phase 1:The Directory Server has UDP port number %d and IP address 68.181.201.3\n", atoi(UDPPORT1));
	
	FILE *fp;
	if ((fp = fopen("directory.txt", "wb")) == NULL){
	    printf("cannot open file\n");
		exit(1);
	}
	
	while(i < 5){
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	
	if (i < 3){
	    if ((rv = getaddrinfo(NULL, UDPPORT1, &hints, &servinfo)) != 0) {       // file servers use UDPPORT1 and clients use UDPPORT2.
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));             // This is for the first 3 file servers.
            return 1;
        }
	}
	else{
	    if ((rv = getaddrinfo(NULL, UDPPORT2, &hints, &servinfo)) != 0) {       // file servers use UDPPORT1 and clients use UDPPORT2.
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));             // This is for the last 2 clients
            return 1;
		}
	}
	
	// loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }
	
	if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
	
	freeaddrinfo(servinfo);
	
	
	addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,                  // read from the file server or client and store in "buf"
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
	
	if (numbytes != 0){
	   i = i + 1;
	}
	
	//printf("The received message is: %s.\n", buf);
	
	if (i < 4){
	    num[0] = buf[11];                        // file servers use buf[11] and clients use buf[6]
	    int serverNum = atoi (num);
	    printf("Phase 1:The Directory Server has received request from File Server %d.\n", serverNum);        // On-screen message for file server.
		
		// Write the message from file servers into directory.txt.
		if (i != 1){
	    fseek(fp, 0, SEEK_END);
	    fprintf(fp, "\n%s", buf);
	    }
	    else{
		    ftruncate(fileno(fp),0);
	        rewind(fp);
			fprintf(fp, "%s", buf);
	    }
	    
		
		if (i == 3){	
	        fclose(fp);
			printf("Phase 1:The Directory.txt file has been created. End of Phase 1 for the Directory Server.\n");           // On-screen message at the end of phase 1.
	    } 
		
	}
	else{
	    num[0] = buf[6];                        // file servers use buf[11] and clients use buf[6]
		int clientNum = atoi (num);
		printf("Phase 2:The Directory Server has received request from Client %d.\n", clientNum);              // On-screen message for client.
		
		if(clientNum == 1){                        // Store the request from client 1 and client 2.
		    strncpy(req1,&buf[8],4);
            strcpy(&req1[4],"\0");			
			//printf("%s\n", req1);
			sentMsg = findFileServer(req1,clientNum);
			//printf("the msg is %s",sentMsg);
			
			if ((sentnumbytes = sendto(sockfd, sentMsg, strlen(sentMsg), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
			printf("Phase 2:File server details has been sent to Client %d.\n",clientNum);
        }
        else{
		    strncpy(req2,&buf[8],4);
            strcpy(&req2[4],"\0");			
			//printf("%s\n",req2);
			sentMsg = findFileServer(req2,clientNum);
			
			if ((sentnumbytes = sendto(sockfd, sentMsg, strlen(sentMsg), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
			printf("Phase 2:File server details has been sent to Client %d.\n",clientNum);
        }			
	    
		if(i == 6){
		    printf("Phase 2:End of Phase 2 for the Directory Server.\n");             // On-screen message at the end of phase 2.
		}
		
	}
	
	
	close(sockfd);
		
	}
	return 0;
}


