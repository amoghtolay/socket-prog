#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <sstream>

using namespace std;
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[2560];
	if (argc < 4) {
		cout<<"Usage: "<<argv[0]<<" <hostname> <port> <command>\n";
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR: no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons(portno);
	if ( connect (sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	/*
	printf("Please enter the message: ");
	bzero(buffer,2560);
	fgets(buffer,2559,stdin);
	*/
	// Adding my code here to read from the main arguments itself:
	string query = "";
	for (int i=3; i<argc; i++){
		stringstream ss;
		ss<<argv[i];
		if ( i == 3 )
			query = ss.str();
		else
			query = query + " " + ss.str();
	}
	strcpy ( buffer , query.c_str() );
	n = write(sockfd,buffer,strlen(buffer));
	if (n < 0) 
		error("ERROR writing to socket");
	bzero(buffer,2560);
	n = read(sockfd,buffer,2559);
	if (n < 0) 
		error("ERROR reading from socket");
	printf("%s\n",buffer);
	
	close(sockfd);
	return 0;
}
