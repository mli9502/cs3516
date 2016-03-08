/*
Author: Mengwen Li (mli2)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <errno.h>
#include <fstream>
#include <sys/time.h>
#include <time.h>
//10K Bytes and 1 extra byte for "\0"
#define BUFSIZE 10241

using namespace std;

//Convert host name to ip address.
int htoi(char* host, char** ip);
//Split string with given character.
vector<string> split(const char *input, char c);
//Check whether the input contains a certain command and return the index of it.
int containsCmd(char**, const char*, int);
//Count the number of given flags in a given input.
int flgCnt(char**, char*, int);
//Check whether a flag is valid.
bool validFlg(char*);
//Check whether a command is valid.
bool validCmd(char**, int);

int main(int argc, char* argv[])
{
	int rtn;
	//Define the flags corresponding to the flags from command line.
	bool fileFlg = true;
	bool pingFlg = false;
	bool pktFlg = false;
	bool infoFlg = false;
	//Create output file, "web.txt" as default.
	ofstream outFile;
	string fn_str = "web.txt";
	char* fileName = (char*)(fn_str.c_str());
	//Define start and end time used for -pkt and -ping.
	struct timeval startTime, endTime;
	//Create vector to store the packet byte and time, used for -pkt.
	vector<struct timeval> pkgTime;
	vector<int> pkgBytes;
	//Check the number of arguments in command line, if less then 2, it can't be valid.
	if(argc < 2)
	{
		cout << "Command not valid!" << endl;
		exit(-1);
	}
	//Check for valid command line flags.
	if(!validCmd(argv, argc))
	{
		cout << "Command not valid!" << endl;
		exit(-1);
	}
	//Set the flag variable according to command line args.
	if(containsCmd(argv, "-ping", argc) != -1)
	{
		pingFlg = true;
	}
	if((rtn = containsCmd(argv, "-f", argc)) != -1)
	{
		fileName = argv[rtn + 1];
	}
	if(containsCmd(argv, "-nf", argc) != -1)
	{
		fileFlg = false;
	}
	if(containsCmd(argv, "-pkt", argc) != -1)
	{
		fileFlg = false;
		pktFlg = true;
	}
	if(containsCmd(argv, "-info", argc) != -1)
	{
		infoFlg = true;
	}
	//Open the file for output if fileFlg is true.
	if(fileFlg)
	{
		outFile.open(fileName);
	}
	string server, path;
	//Set the default path to "/"
	path = "/";
	int port;
	//Split the URL with "/" into components.
	vector<string> compo = split(argv[1], '/');
	vector<string> serv_Port;
	//If the URL does not start with "http://", the first argument is the server address.
	if(compo.at(0) != "http:")
	{
		serv_Port = split(compo.at(0).c_str(), ':');
		for(int i=1; i<compo.size(); i++)
		{
			path += compo.at(i) + "/";
		}
	}
	//If the URL contains "http://", the second 
	else
	{
		//Check for input "http://" as URL.
		if (compo.size() < 2)
		{
			cout << "Input URL is not correct" << endl;
			exit(-1);
		}
		serv_Port = split(compo.at(1).c_str(), ':');
		for(int i=2; i<compo.size(); i++)
		{
			path += compo.at(i) + "/";
		}
	}
	//Set the server.
	server = serv_Port.at(0);
	//Set the port number if specified, else, set to default.
	if(serv_Port.size() > 1)
	{
		port = atoi(serv_Port.at(1).c_str());
	}
	else
	{
		port = 80;
	}

	char* servIP;
	htoi((char*)server.c_str(), &servIP);
	//Define the server port.
	in_port_t servPort = port;
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
	if(pingFlg || pktFlg)
	{
		gettimeofday(&startTime, NULL);
	}
	int cntVal = connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if(cntVal < 0)
	{
		cout << "connect() failed" << endl;
		exit(-1);
	}
	//If -ping is in command line, output the ping value in ms.
	if (pingFlg)
	{
		gettimeofday(&endTime, NULL);
		double start = (double)(startTime.tv_sec) * 1000.0 + (double)(startTime.tv_usec) / 1000.0;
		double end = (double)(endTime.tv_sec) * 1000.0 + (double)(endTime.tv_usec) / 1000.0;
		cout << "The RTT time is " << end - start << " milliseconds" << endl;
	}
	//Construct request message.
	stringstream ss;
	string cr_str, rt_str;
	char cr = '\r';
	ss << cr;
	ss >> cr_str;
	char rt = '\n';
	ss << rt;
	ss >> rt_str;
	string msg = "GET " + path + " HTTP/1.0" + cr + rt + "Host:" + 
					server + cr + rt + cr + rt;
	char* echoString = (char*)msg.c_str();
	size_t echoStringLen = strlen(echoString);
	//Send the input string.
	int numBytes = send(sock, echoString, echoStringLen, 0);
	if(numBytes != echoStringLen)
	{
		cout << "send() error" << endl;
		exit(-1);
	}
	//Receive the string back from server.
	unsigned long totalBytesRcvd = 0;
	//Try to repeatedly receive bytes from server until all bytes are received.
	while(1)
	{
		char buffer[BUFSIZE];
		numBytes = recv(sock, buffer, BUFSIZE-1, 0);
		if(numBytes < 0)
		{
			cout << "recv() failed" << endl;
			exit(-1);
		}
		if(numBytes == 0)
		{
			break;
		}
		//If "-pkt" is in command line, record the time and number of bytes received.
		if(pktFlg)
		{
			timeval tempTime;
			gettimeofday(&tempTime, NULL);
			pkgTime.push_back(tempTime);
			pkgBytes.push_back(numBytes);
		}	
		totalBytesRcvd += numBytes;
		buffer[numBytes] = '\0';
		//If output to file, write to file, else, write to console.
		if(fileFlg)
		{
			outFile << buffer;
		}
		else
		{
			cout << buffer;
		}
	}
	cout << endl;
	//Close file.
	if(fileFlg)
	{
		outFile.close();
	}
	//If "-info" is in command line, output the rtt time.
	if(infoFlg)
	{
		struct tcp_info sockInfo;
		socklen_t sockLen = sizeof(sockInfo);
		rtn = getsockopt(sock, SOL_TCP, TCP_INFO, (void*)&sockInfo, &sockLen);
		if(rtn != 0)
		{
			cout << "getsockopt() error!" << endl;
			exit(-1);
		}
		cout << "The rtt value get from getsockopt() is " << sockInfo.tcpi_rtt / 1000.0 << " milliseconds" << endl;
		cout << "The rtt variance get from getsockopt() is " << sockInfo.tcpi_rttvar / 1000.0 << " milliseconds" << endl;
	}
	//Close the socket.
	close(sock);
	//int max = 0;
	//If "-pkt" is in command line, output time and byte count for each block received.
	if(pktFlg)
	{
		for(int i=0; i<pkgBytes.size(); i++)
		{
			cout << "Read number " << i + 1 << " : " << endl;
			cout << "Number of bytes received: " << pkgBytes.at(i) << endl;
			//if(max < pkgBytes.at(i))
			//{
			//	max = pkgBytes.at(i);
			//}
			//Print out the time of reception.
			double start = (double)(startTime.tv_sec) * 1000.0 + (double)(startTime.tv_usec) / 1000.0;
			double end = (double)(pkgTime.at(i).tv_sec) * 1000.0 + (double)(pkgTime.at(i).tv_usec) / 1000.0;
			cout << "Time: " << end - start << endl;
		}
		//cout << totalBytesRcvd << endl;
		// cout << "!!!!!!!!!!!!!!!!!!! MAX !!!!!!!!!!!!!!!!!!!" << endl;
		// cout << max << endl;
		// cout << "package bytes !!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		// for(int i=0; i<pkgBytes.size(); i++)
		// {
		// 	cout << pkgBytes.at(i) << endl;
		// }
		// cout << "time !!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		// for(int i=0; i<pkgBytes.size(); i++)
		// {
		// 	double start = (double)(startTime.tv_sec) * 1000.0 + (double)(startTime.tv_usec) / 1000.0;
		//  	double end = (double)(pkgTime.at(i).tv_sec) * 1000.0 + (double)(pkgTime.at(i).tv_usec) / 1000.0;
		//  	cout << end - start << endl;
		// }
	}
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

//Function used to tokenlize a string with given character.
vector<string> split(const char *input, char c)
{
	vector<string> result;
	const char* temp;
	while (*input != 0)
	{
		//Loop through the string and seperate it based on the given char.
		while (*input == c && *input != 0)
		{
			input++;
			if (*input == 0)
			{
				return result;
			}
		}
		temp = input;
		while (*input != c && *input != 0)
		{
			input++;
		}
		result.push_back(string(temp, input));
	}
	return result;
}
//Count the number of flags. 
int flgCnt(char** argv, char* input, int argc)
{
	int cnt = 0;
	for(int i=0; i<argc; i++)
	{
		if(strcmp(argv[i], input) == 0)
		{
			cnt ++;
		}
	}
	return cnt;	
}
//Checkt whether the command line arguments are valid or not.
bool validCmd(char** argv, int cnt)
{
	int rtn;
	int invalidCnt = 0;
	for(int i=2; i<cnt; i++)
	{
		if(!(validFlg(argv[i])))
		{
			invalidCnt ++;
		}
	}
	if(invalidCnt > 1)
	{
		cout << "Command line argument invalid!" << endl;
		exit(-1);
	}
	//If -f is not specified and there is an invalid flag.
	else if(invalidCnt == 1 && (rtn = containsCmd(argv, "-f", cnt)) == -1)
	{
		cout << "Found invalid flag" << endl;
		exit(-1);
	}
	//The locations of the Flgs should be greater than 1.
	if(validFlg(argv[1]))
	{
		cout << "Need to have the network address first before Flgs!" << endl;
		exit(-1);
	}
	//Check for special cases.
	//-f and -nf should not be existed at the same time.
	if((containsCmd(argv, "-f", cnt) != -1) && 
		(containsCmd(argv, "-nf", cnt) != -1))
	{
		cout << "-f can not be used together with -nf!" << endl;
		exit(-1);
	}
	//-pkt can't exist with -f and -nf.
	if(containsCmd(argv, "-pkt", cnt) != -1)
	{
		if(containsCmd(argv, "-f", cnt) != -1 ||
			containsCmd(argv, "-nf", cnt) != -1)
		{
			cout << "-pkt should not be used together with -f or -nf" << endl;
			exit(-1);
		}
	}
	//If -f is specified, the next arg should be file name.
	if((rtn = containsCmd(argv, "-f", cnt)) != -1)
	{
		if((!(cnt >= 2 + rtn)) || 
			validFlg(argv[1 + rtn]))
		{
			cout << "-f should be followed by a file name" << endl;
			exit(-1);
		}
	}
	//Check whether there are duplicate commands.
	for(int i=0; i<cnt; i++)
	{
		if(flgCnt(argv, argv[i], cnt) > 1)
		{
			cout << "Found duplicate commands!" << endl;
			exit(-1);
		}
	}
	return true;
}
//Check whether the input flag is valid or not.
bool validFlg(char* input)
{
	if(!(strcmp(input, "-f") &&
		strcmp(input, "-nf") &&
		strcmp(input, "-pkt") &&
		strcmp(input, "-ping") &&
		strcmp(input, "-info")))
	{
		return true;
	}
	return false;
}
//Check whether command line contains a specific command.
int containsCmd(char** argv, const char* cmd, int size)
{
	for(int i=0; i<size; i++)
	{
		if(strcmp(argv[i], cmd) == 0)
		{
			return i;
		}
	}
	return -1;
}
