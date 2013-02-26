/***********************************************************************
 * Author: Amogh Tolay, Networks assignment 2, Sem 6
 * This is a basic calendar server implementation.
 * The server runs in 3 modes:
 * 1. Iterative (only one client supported)
 * 2. Multi-threaded (multiple clients at once supported)
 * 3. Selection of client based on select()
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include "fileReadWrite.h"

using namespace std;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[2560];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		cout<<"ERROR, no port provided. Usage is "<<argv[0]<<" ./server <port-number> <server mode>\n";
		cout<<"Server mode can be one of the following:\n1: Iterative (one at a time)\n2. Multi-threaded\n3. Using select()\n";
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	/*
	 * Code to ensure binding port is free next time
	 */
	int reuseYES=1;
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseYES,sizeof(int)) == -1) {
		perror("setsockopt: Could not clear the port that was being used earlier.");
		exit(1);
	}
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while (true){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");
		bzero(buffer,2560);
		n = read(newsockfd,buffer,2559);
		if (n < 0)
			error("ERROR reading from socket");
		/*
		 * At this point, buffer contains the entire query. Now, call the
		 * execQuery function on this buffer
		 */
		stringstream convert;
		convert << buffer;
		string query = convert.str();

		string output = execQuery(query);

		char writeOut[2560];
		bzero(writeOut,2560);
		strcpy ( writeOut , output.c_str() );
		n = write(newsockfd,writeOut,strlen(writeOut));
		/*
		 * Now write the output that comes from execQuery
		 */

		if (n < 0)
			error("ERROR writing to socket");
		close(newsockfd);
	}
	close(sockfd);
	return 0; 
}
