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

using namespace std;

int main(int argc, char* argv[])
{
	//Check for command line arguments.
	if(argc != 3)
	{
		//Ask this.
		if(argc == 2)
		{
			cout << "Input string length out of range" << endl;
		}
		cout << "Parameters: <Server addr> <Echo Word>" << endl;
		exit(-1);
	}

	//Get the server ip and echo string.
	//Use port number 5000.
	char* servIP = argv[1];
	char* echoString = argv[2];
	in_port_t servPort = 5000;
	
	//Check for input string length.
	size_t echoStringLen = strlen(echoString);
	if(echoStringLen > 32 || echoStringLen < 1)
	{
		cout << "Input string length out of range" << endl;
		exit(-1);
	}
	
	//Create socket using IPv4 and TCP protocol.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		cout << "socket() failed" << endl;
		exit(-1);
	}
	
	//Set up server address.
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
	if(rtnVal == 0)
	{
		cout << "inet_pton() failed" << " invalid address string" << endl;
		exit(-1);
	}
	else if(rtnVal < 0)
	{
		cout << "inet_pton() failed" << endl;
		exit(-1);
	}
	servAddr.sin_port = htons(servPort);
	
	//Establish the connection to the echo server.
	int cntVal = connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if(cntVal < 0)
	{
		cout << "connect() failed" << endl;
		exit(-1);
	}
	
	//Send string to server through socket.
	ssize_t numBytes = send(sock, echoString, echoStringLen, 0);
	if(numBytes != echoStringLen)
	{
		cout << "send() error" << endl;
		exit(-1);
	}
	
	//Receive the string back from server.
	unsigned int totalBytesRcvd = 0;
	cout << "Client received: ";
	
	//Try to repeatedly receive bytes from server until all bytes are received.
	while(totalBytesRcvd < echoStringLen)
	{
		char buffer[BUFSIZE];
		numBytes = recv(sock, buffer, BUFSIZE-1, 0);
		if(numBytes <= 0)
		{
			cout << "recv() failed" << endl;
			exit(-1);
		}
		totalBytesRcvd += numBytes;
		buffer[numBytes] = '\0';
		cout << buffer;
	}
	cout << endl;
	
	//Close socket.
	close(sock);
	exit(0);
}
