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
#include <signal.h>
#include "returnCodes.h"

using namespace std;

void queryHandling(int); /* function prototype */
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void intHandler(int waste=1) {
    error("\nThe user pressed Ctrl-C, so terminating...\n");
}
int main(int argc, char *argv[])
{
	/*
	 * Handling Ctrl-C (SIGINT) to ensure that code
	 * quits gracefully
	 */
	signal(SIGINT, intHandler);
	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int mode;
	if (argc < 3) {
		cout<<"ERROR, no port provided. Usage is "<<argv[0]<<" ./server <port-number> <server mode>\n";
		cout<<"Server mode can be one of the following:\n1: Iterative (one at a time, in while loop)\n2. Multi-threaded\n3. Using select() function\n";
		exit(1);
	}
	mode = atoi ( argv[2] );
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
	
	if ( mode == 1 ){ // Iterative approach
		while (true){
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) 
				error("ERROR on accept");
			queryHandling ( newsockfd );
			close(newsockfd);
		}
	}
	if ( mode == 2 ){ // Multi-process based
		// Eliminating Zombie problem
		signal(SIGCHLD,SIG_IGN);
		while (1) {
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
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
	}
	if ( mode == 3 ){ // Select()
			fd_set master;		// master file descriptor list
			fd_set read_fds;	// temp file descriptor list for select()
			int fdmax;     		// maximum file descriptor number
			char buffer[2560];	// buffer to exchange data between server and client
			FD_ZERO(&master);	// clear the entries from the master set
			FD_ZERO(&read_fds);	// Clear all entries from the temporary set read_fds
			/* add the listener to the master set */
			FD_SET ( sockfd , &master );
			/* keep track of the biggest file descriptor */
			fdmax = sockfd;
			while(1){
				read_fds = master;
				if ( select ( fdmax+1 , &read_fds , NULL , NULL , NULL) == -1)
					error("select() function failed \n");

				/* run through the existing connection looking for data to read */
				for ( int i = 0; i <= fdmax; i++ ){
					if ( FD_ISSET ( i , &read_fds ) ){
						if ( i == sockfd ){
							/* handle new connections */
							cout << "Ready to accept a new connection\n";
							clilen = sizeof cli_addr;
							/* accept a connection */
							newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
							if (newsockfd < 0)
								error("Select() Server could not accept new connection\n ");
							else {
								FD_SET (newsockfd , &master ); // add to master set
								if ( newsockfd > fdmax )
									fdmax = newsockfd;
							}
						}
						else {
							/* handle data from client */
							bzero(buffer,2560);

							/* receive data */
							int rdatalen = recv ( i, buffer, 2559, 0);
							if ( rdatalen <= 0 ) {
								/* got error or connection closed by client */
								if (rdatalen == 0) {
									/* connection closed */
									printf("selectserver: socket %d hung up\n", i);
								}
								else
									error("Receiving error:- Did not receive from client when expected");
								close(i); // bye!
								FD_CLR(i, &master); // remove from master set
							}
							else {
								/* some data received from client */
								stringstream ss;
								ss << buffer;
								string query = ss.str();
								string output = execQuery(query);

								char writeOut[2560];
								bzero(writeOut,2560);
								strcpy ( writeOut , output.c_str() );
								int n = write(i,writeOut,strlen(writeOut));
								/*
								 * Now write the output that comes from execQuery
								 */
								if (n < 0)
									error("ERROR writing to socket");
							}
						} /* END handling data from client */
					} /* END got new incoming connection */
				} /* END looping through file descriptors */
			} /* END main while loop */
	} /* END select case completely */
	if ( mode != 1 || mode != 2 || mode != 3 ){ // Wrong argument
		close(sockfd);
		cout<<"The argument for mode is wrong. \nServer mode can be one of the following:\n1: Iterative (one at a time, in while loop)\n2. Multi-threaded\n3. Using select() function\n" ;
		exit (1);
	}
	close(sockfd);
	return 0; 
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
	if (n < 0)
		error("ERROR writing to socket");
	/*
	 * Now write the output that comes from execQuery
	 */
	if ( string::npos != output.find(totalLines) ){
		unsigned pos = output.find(" ");
		int numLines = atoi(output.substr(0,pos).c_str());
		for ( int i=1; i<=numLines; i++ ){
			bzero(buffer,2560);
			n = read(sock,buffer,2559);
			stringstream convert;
			convert << buffer;
			string getLineQuery = convert.str();

			string output = execQuery(getLineQuery);
			bzero(writeOut,2560);
			strcpy ( writeOut , output.c_str() );
			n = write(sock,writeOut,strlen(writeOut));
			if (n < 0)
				error("ERROR writing to socket");
			}
			bzero(writeOut,2560);
	}
}
