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

//handleTCPClient function handles the incoming messages.
void handleTCPClient(int);

int main(int argc, char* argv[])
{
	//Check for command line args.
	if(argc > 1)
	{
		cout << "Should not have parameters" << endl;
		exit(-1);
	}

	//Set the port number.
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
	
	//Bind to the local address.
	int bindRtn = bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if(bindRtn < 0)
	{
		cout << "bind() failed" << endl;
		exit(-1);
	}
	
	//Convert servSock into passive socket and specify the max connections.
	int listenRtn = listen(servSock, MAXPENDING);
	if(listenRtn < 0)
	{
		cout << "listen() failed" << endl;
		exit(-1);
	}
	while(1)
	{
		struct sockaddr_in clntAddr;
		socklen_t clntAddrLen = sizeof(clntAddr);
		
		//Wait for incoming connections and return a new socket descriptor.
		int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
		if(clntSock < 0)
		{
			cout << "accept() failed" << endl;
		}
		//Get the name of the client.
//		char clntName[INET_ADDRSTRLEN];
//		if(inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL)
//		{
//			cout << "Handling client " << clntName << "/" << ntohs(clntAddr.sin_port) << endl;
//			cout << "clntAddr.sin_port is " << clntAddr.sin_port << endl;
//		}
//		else
//		{
//			cout << "Can't get client address" << endl;
//			exit(-1);
//		}
		
		//Handle the client message.
		handleTCPClient(clntSock);
		
		//Close the server socket and exit.
		close(servSock);
		exit(0);
	}
}

void handleTCPClient(int clntSocket)
{
	char buffer[BUFSIZE];
	//Receive message from client.
	ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
	if(numBytesRcvd < 0)
	{
		cout << "recv() failed" << endl;
		exit(-1);
	}
	
	//Send received string and receive again until end of stream.
	while(numBytesRcvd > 0)
	{
		//Send back the string.
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
	}
	
	//Close the client socket.
	close(clntSocket);
}
