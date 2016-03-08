//Modified from code in D&C.
//author: Mengwen Li (mli2)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#define BUFSIZE 200

static const int MAXPENDING = 5;

using namespace std;

//handleTCPClient handles the message received from client.
int handleTCPClient(int);

int main(int argc, char* argv[])
{
	//Check for command line arguments.
	if(argc > 1)
	{
		cout << "Don't have parameters" << endl;
		exit(-1);
	}
	//Defines the server port.
	in_port_t servPort = 5000;
	
	//Create server socket.
	int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(servSock < 0)
	{
		cout << "socket() failed" << endl;
		exit(-1);
	}
	
	//Construct local address structure.
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(servPort);
	
	//Assigns local Internet protocol address to servSock.
	int bindRtn = bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if(bindRtn < 0)
	{
		cout << "bind() failed" << endl;
		exit(-1);
	}
	
	//Converts the unconnected socket servSock to passive socket and specifies the max pending.
	int listenRtn = listen(servSock, MAXPENDING);
	if(listenRtn < 0)
	{
		cout << "listen() failed" << endl;
		exit(-1);
	}
	
	//Handle the incoming connection.
	while(1)
	{
		struct sockaddr_in clntAddr;
		memset(&clntAddr, 0, sizeof(clntAddr));
		socklen_t clntAddrLen = sizeof(clntAddr);
		//Dequeues the next connection from servSock queue.
		int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
		if(clntSock < 0)
		{
			cout << "accept() failed" << endl;
		}
		//Get the name of the client.
		/*
		char clntName[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL)
		{
			cout << "Handling client " << clntName << "/" << ntohs(clntAddr.sin_port) << endl;
			cout << "clntAddr.sin_port is " << clntAddr.sin_port << endl;
		}
		else
		{
			cout << "Can't get client address" << endl;
			exit(-1);
		}
		*/
		//Handle the client message and check whether the received message is DEL ETX.
		int ret = handleTCPClient(clntSock);
		//Close both the clntSock and the servSock if DEL ETX is received and exit.
		if(ret == 2)
		{
			close(clntSock);
			close(servSock);
			exit(0);
		}
	}
	exit(0);
}

//handleTCPClient handles the message received from client.
int handleTCPClient(int clntSocket)
{
	//Record the count of messages received.
	int cnt = 0;
	//Indicates DLE ETX has been received.
	int endFlag = 0;

	char buffer[BUFSIZE];
	ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
	if(numBytesRcvd < 0)
	{
		cout << "recv() failed" << endl;
		exit(-1);
	}
	while(numBytesRcvd > 0)
	{
		cnt ++;
		
		//Send the received message back to client.
		ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
		if(numBytesSent < 0 || numBytesSent != numBytesRcvd)
		{
			cout << "send() failed" << endl;
			exit(-1);
		}
		
		//Receive again.
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
		if(numBytesRcvd < 0)
		{
			cout << "recv() failed" << endl;
			exit(-1);
		}
		
		//If DEL ETX has been received, set endFlag.
		if(numBytesRcvd == 2 && buffer[0] == 16 && buffer[1] == 3 && !endFlag)
		{
			cout << "Enhanced Echo Server sent " << cnt << " strings to the Enhanced Echo Client" << endl;
			endFlag = 1;
		}
	}
	
	//If endFlag is set, return 2.
	if(endFlag)
	{
		return 2;
	}
	return 0;
}


