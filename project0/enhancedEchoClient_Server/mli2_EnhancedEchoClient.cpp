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
#include <netdb.h>
#include <iostream>
#include <vector>

#define BUFSIZE 200

using namespace std;
//Convert host name to ip address.
int htoi(char* host, char** ip);

int main(int argc, char* argv[])
{
	vector<char*> inputs;	
	//Handles the situation when the input args are less than 2.
	if(argc < 2)
	{
		cout << "Parameters: <Server name> <Echo Word>...<Echo Word>" << endl;
		exit(-1);
	}
	//Handles the situation when there are more than 5 input strings.
	else if(argc > 7)
	{
		cout << "No more than six input strings" << endl;
		exit(-1);
	}
	
	//Record the input strings and throw out the out of range strings.
	for(int i=2; i<argc; i++)
	{
		if(strlen(argv[i]) > 12 || strlen(argv[i]) < 1)
		{
			cout << "Input string \"" << argv[i] << "\" out of range" << endl;
			continue;
		}
		inputs.push_back(argv[i]);
	}
	
	//If all the strings are out of range, exit.
	if(inputs.empty())
	{
		cout << "No valid input string" << endl;
		exit(-1);
	}
	
	//Get the server ip from host name.
	char* servIP;
	htoi(argv[1], &servIP);
	//Define the server port.
	in_port_t servPort = 5000;
	
	//Create socket using IPv4 and TCP protocol.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		cout << "socket() failed" << endl;
		exit(-1);
	}
	
	//Set up server address struct.
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	
	//Convert the IPv4 address to network format.
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
	
	//Convert the port number from host byte order to network byte order.
	servAddr.sin_port = htons(servPort);	
	//Establish the connection to the echo server.
	int cntVal = connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if(cntVal < 0)
	{
		cout << "connect() failed" << endl;
		exit(-1);
	}
	
	//Send all the valid input strings.
	for(int i=0; i<inputs.size(); i++)
	{
		char* echoString = inputs.at(i);
		size_t echoStringLen = strlen(echoString);
		//Send the input string.
		ssize_t numBytes = send(sock, echoString, echoStringLen, 0);
		if(numBytes != echoStringLen)
		{
			cout << "send() error" << endl;
			exit(-1);
		}
		//Receive the string back from server.
		unsigned int totalBytesRcvd = 0;
		cout << "EnhancedClient received: ";
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
	}
	
	//Send DLE ETX
	char lastString[2] = {16, 3};
	ssize_t numBytes = send(sock, lastString, 2, 0);
	unsigned int totalBytesRcvd = 0;
	if(numBytes != 2)
	{
		cout << "send() error" << endl;
		exit(-1);
	}
	
	//Receive DLE ETX
	while(totalBytesRcvd < 2)
	{
		char buffer[BUFSIZE];
		numBytes = recv(sock, buffer, BUFSIZE-1, 0);
		if(numBytes <= 0)
		{
			cout << "recv() failed" << endl;
			exit(-1);
		}
		totalBytesRcvd += numBytes;
	}
	cout << "EnhancedClient: done" << endl;
	
	//Close socket.
	close(sock);
	exit(0);
}

//Converts host name to IPv4 address.
int htoi(char* host, char** ip)
{
	//cout << "host is " << host << endl;
	struct hostent *ht;
	ht = gethostbyname(host);
	if(ht == NULL)
	{
		cout << "gethostbyname() fail" << endl;
		exit(-1);
	}
	*ip = inet_ntoa(*((struct in_addr *)ht->h_addr_list[0]));
	//cout << *ip << endl;
	return 0;
}
