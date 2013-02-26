/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include "fileReadWrite.h"
#include <signal.h>

using namespace std;

void queryHandling(int); /* function prototype */
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
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
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		  sizeof(serv_addr)) < 0) 
		  error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	// Eliminating Zombie problem
	signal(SIGCHLD,SIG_IGN);
	
	while (1) {
		newsockfd = accept(sockfd, 
		(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");
		pid = fork();
		if (pid < 0)
			error("ERROR on fork");
		if (pid == 0)  {
			close(sockfd);
			queryHandling (newsockfd);
			exit(0);
		}
		else close(newsockfd);
	} /* end of while */
	close(sockfd);
	return 0; /* we never get here */
}

/******** QUERYHANDLING() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 **********************************************/
void queryHandling (int sock)
{
	int n;
	char buffer[2560];

	bzero(buffer,2560);
	n = read(sock,buffer,2559);
	if ( n < 0 )
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
	n = write(sock,writeOut,strlen(writeOut));
	/*
	 * Now write the output that comes from execQuery
	 */
	if (n < 0)
		error("ERROR writing to socket");
}
