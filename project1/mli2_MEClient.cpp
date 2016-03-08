/*
Author: Mengwen Li (mli2)
*/
#include <fstream>
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
#include "helper.h"

#define BUFSIZE 200

using namespace std;
//Convert host name to ip address.
int htoi(char* host, char** ip);

int main(int argc, char* argv[])
{
	//Check commend line args.
	if(argc > 3)
	{
		cout << "Too many args!" << endl;
		exit(-1);
	}
	//Prepare for writing to file.
	ofstream outFile;
	outFile.open("MEClient.log");

	//Get server IP.
	char* servIP;
	htoi(argv[1], &servIP);
	//Define the server port.
	in_port_t servPort = 8000;
	//Create socekt.
	int sock;
	//Construct server address.
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
	//Convert IPv4 address to network format.
	int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
	if(rtnVal == 0)
	{
		cout << "Invalid address" << endl;
		exit(-1);
	}
	else if(rtnVal < 0)
	{
		cout << "inet_pton() failed" << endl;
		exit(-1);
	}
	servAddr.sin_port = htons(servPort);
	//Read in the lines user type in.
	string cmd;
	ssize_t numBytes;
	char buffer[BUFSIZE];
	bool authorLevel, loggedIn = false;
	vector<string> cmdToken;	
	for(;;)
	{
		getline(cin, cmd);	
		//Handle the case when an enter is entered.
		if(cmd.length() == 0)
		{
			continue;
		}
		cmdToken = split(cmd.c_str(), ' ');
		//Handle login.
		if(cmdToken.at(0) == "login")
		{
			//Write command to file.
			outFile << "Command issued: " << endl << cmd << endl;
			//Handle the case when user has logged in.
			if(loggedIn)
			{
				cout << "There's already a user logged in!" << endl;
				outFile << "Response: " << endl << "There's already a user logged in!" << endl;
				continue;
			}
			//Check the number of args of command.
			if(cmdToken.size() == 1 || cmdToken.size() > 2)
			{
				cout << "Invalid command format! login <username>" << endl;
				outFile << "Response: " << endl << "Invalid command format! login <username>" << endl;
				continue;
			}
			//Check the name os the user.
			if(cmdToken.at(1) == "FEMA" || cmdToken.at(1) == "Query")
			{
				loggedIn = true;
				if(cmdToken.at(1) == "FEMA")
				{
					authorLevel = true;
				}
				else
				{
					authorLevel = false;
				}
				//Create socket.
				sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if(sock < 0)
				{
					cout << "socket() failed" << endl;
					exit(-1);
				}
				//Initiates a 3-way handshake.
				int cntVal = connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
				if(cntVal < 0)
				{
					cout << "connect() failed" << endl;
					exit(-1);
				}
			    sendMsg(sock, cmd);
				//Receive message back from server.
			    numBytes = recv(sock, buffer, BUFSIZE-1, 0);
			    if(numBytes <= 0)
			    {
			        cout << "recv() fail" << endl;
			        exit(-1);
			    }
			    buffer[numBytes] = '\0';
			    cout << buffer << endl;
			    //Write the response to log file.
			    outFile << "Response: " << endl << buffer << endl;
			}
			else
			{
				cout << "User name incorrect!" << endl;
				outFile << "Response: " << endl << "User name incorrect" << endl;
				continue;
			}
		}
		//Handle quit.
		if(cmdToken.at(0) == "quit")
		{
			//Write the command to file.
			outFile << "Command issued: " << endl << cmd << endl;
			if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
			//Check the number of args.
			if(cmdToken.size() > 2)
			{
				cout << "Invalid command format! quit <EOF>" << endl;
				outFile << "Response: " << endl << "Invalid command format! quit <EOF>" << endl;
				continue;
			}
			//Send the command to server.
			sendMsg(sock, cmd);
			numBytes = recv(sock, buffer, BUFSIZE-1, 0);
			if(numBytes <= 0)
            {
                cout << "recv() fail" << endl;
                exit(-1);
            }
            buffer[numBytes] = '\0';
            cout << buffer << endl;
            //Write the response to file.
            outFile << "Response: " << endl << buffer << endl;
            //Mark the client as not logged in.
            loggedIn = false;
            //Handle the case when "EOF" is entered.
            if(cmdToken.size() == 2 && cmdToken.at(1) == "EOF")
            {
            	numBytes = recv(sock, buffer, BUFSIZE-1, 0);
				if(numBytes <= 0)
	            {
	                cout << "recv() fail" << endl;
	                exit(-1);
	            }
	            buffer[numBytes] = '\0';
	            cout << buffer << endl;
	            //Output the response to file and close the log file.
	            outFile << buffer << endl;
	            outFile.close();
	            //Send the confirm message, close socket and exit.
	            sendMsg(sock, "confirm");
	            close(sock);
            	return 0;
            }
            //close the socket.
            close(sock);
		}
		//Handle add.
		if(cmdToken.at(0) == "add")
		{
			//Write the command to log.
			outFile << "Command issued: " << endl << cmd << endl;
			//Check for log in.
			if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
			//Check whether the user is FEMA.
			if(!authorLevel)
			{
				cout << "User can't use add" << endl;
				outFile << "Response: " << endl << "User can't use add" << endl;
				continue;
			}
			//Check the number of arguments.
			if(cmdToken.size() != 6)
			{
				cout << "Invalid command format! add <id_number> <first_name> <last_name> <gender> <location>" << endl;
				outFile << "Response: " << endl << "Invalid command format! add <id_number> <first_name> <last_name> <gender> <location>" << endl;
				continue;
			}
			//Check the size of id.
			if(cmdToken.at(1).size() != 9)
			{
				cout << "Invalid id number digit!" << endl;
				outFile << "Response: " << endl << "Invalid id number digit!" << endl;
				continue;
			}
			//Check the length of names, gender and address.
			if(cmdToken.at(2).size() > 20 || cmdToken.at(3).size() > 25 || 
				cmdToken.at(5).size() > 30 || cmdToken.at(4).size() > 1)
			{
				cout << "Invalid input argument length!" << endl;
				outFile << "Response: " << endl << "Invalid input argument length!" << endl;
				continue;
			}
			//Check the letter of gender.
			if((cmdToken.at(4).compare("F") != 0) && (cmdToken.at(4).compare("M") != 0))
			{
				cout << "Invalid gender! Should be F or M" << endl;
				outFile << "Response: " << endl << "Invalid gender! Should be F or M" << endl;
				continue;
			}
			//Send command to server.
			sendMsg(sock, cmd);
			//receve the message sent back by server.
			numBytes = recv(sock, buffer, BUFSIZE-1, 0);
            if(numBytes <= 0)
            {
                cout << "recv() fail" << endl;
                exit(-1);
            }
            buffer[numBytes] = '\0';
            cout << buffer << endl;
            //Write the response to log.
            outFile << "Response: " << endl << buffer << endl;
		}
        //Handle update.
		if(cmdToken.at(0) == "update")
		{
			//Write command to log.
			outFile << "Command issued: " << endl << cmd << endl;
			//Handle log in, author level, arg size and the length of arguments the same as above.
			if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
			if(!authorLevel)
			{
				cout << "User can't use update" << endl;
				outFile << "Response: " << endl << "User can't use update" << endl;
				continue;
			}
			if(cmdToken.size() != 6)
			{
				cout << "Invalid command format! update <id_number> <first_name> <last_name> <gender> <location>" << endl;
				outFile << "Response: " << endl << "Invalid command format! update <id_number> <first_name> <last_name> <gender> <location>" << endl;
				continue;
			}
			if(cmdToken.at(1).size() != 9)
			{
				cout << "Invalid id number digit!" << endl;
				outFile << "Response: " << endl << "Invalid id number digit!" << endl;
				continue;
			}
			if(cmdToken.at(2).size() > 20 || cmdToken.at(3).size() > 25 || 
				cmdToken.at(5).size() > 30 || cmdToken.at(4).size() > 1)
			{
				cout << "Invalid input argument length!" << endl;
				outFile << "Response: " << endl << "Invalid input argument length!" << endl;
				continue;
			}
			if((cmdToken.at(4).compare("F") !=0) && (cmdToken.at(4).compare("M") != 0))
			{
				cout << "Invalid gender! Should be F or M" << endl;
				outFile << "Response: " << endl << "Invalid gender! Should be F or M" << endl;
				continue;
			}
			//Send the command to server.
			sendMsg(sock, cmd);
			//May need to modify this.
			char buffer[BUFSIZE];
			numBytes = recv(sock, buffer, BUFSIZE-1, 0);
			if(numBytes <= 0)
			{
				cout << "recv() fail" << endl;
				exit(-1);
			}
			buffer[numBytes] = '\0';
			string buf(buffer);
			if(buf != "success")
			{
				cout << buffer << endl;
				//Write the response to log.
				outFile << "Response: " << endl << buffer << endl;
			}
		}
        //Handle remove.
        if(cmdToken.at(0) == "remove")
        {
        	//Write command to log.
        	outFile << "Command issued: " << endl << cmd << endl;
        	//Handle log in, author level, arg size and arg length the same as above.
        	if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
            if(!authorLevel)
            {
                cout << "User cna't use remove" << endl;
                outFile << "Response: " << endl << "User can't use remove" << endl;
                continue;
            }
            if(cmdToken.size() != 2)
            {
                cout << "Invalid command format! remove <id_number>" << endl;
                outFile << "Response: " << endl << "Invalid command format! remove <id_number>" << endl;
                continue;
            }
            if(cmdToken.at(1).size() != 9)
			{
				cout << "Invalid id number digit!" << endl;
				outFile << "Response: " << endl << "Invalid id number digit!" << endl;
				continue;
			}
			//Send command to server.
            sendMsg(sock, cmd);
            //Receive from server.
            numBytes = recv(sock, buffer, BUFSIZE-1, 0);
            if(numBytes <= 0)
            {
                cout << "recv() fail" << endl;
                exit(-1);
            }
            buffer[numBytes] = '\0';
            cout << buffer << endl;
            //Write the response to log.
            outFile << "Response: " << endl << buffer << endl;
        }
        //Handle find.
        if(cmdToken.at(0) == "find")
        {
        	//Write command to log.
        	outFile << "Command issued: " << endl << cmd << endl;
        	//Handle login, length of args the same as above.
        	if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
            //Check argument size.
            if(cmdToken.size() != 3)
            {
                cout << "Invalid command format! find <first_name> <last_name>" << endl;
                outFile << "Response: " << endl << "Invalid command format! find <first_name> <last_name>" << endl;
                continue;
            }
            if(cmdToken.at(1).size() > 20 || cmdToken.at(2).size() > 25)
			{
				cout << "Invalid input argument length!" << endl;
				outFile << "Response: " << endl << "Invalid input argument length!" << endl;
				continue;
			}
            //Send message to server.
            sendMsg(sock, cmd);
            //Receive from server.
            numBytes = recv(sock, buffer, BUFSIZE-1, 0);
            if(numBytes <= 0)
            {
                cout << "recv() fail" << endl;
            }
            buffer[numBytes] = '\0';
            string bufStr(buffer);
            //Continuous receive message from server.
            outFile << "Response: " << endl;
            while(bufStr != "end")
            {
                cout << bufStr << endl;
                //Write the response to log.
                outFile << bufStr << endl;
                //Send the confirm message to server to make server send the next message.
                sendMsg(sock, "confirm");
                numBytes = recv(sock, buffer, BUFSIZE-1, 0);
                if(numBytes <= 0)
                {
                    cout << "recv() fail" << endl;
                }
                buffer[numBytes] = '\0';
                bufStr = string(buffer);
            }
        }
        //Handle list.
        if(cmdToken.at(0) == "list")
        {
        	//Write the command to log.
        	outFile << "Command issued: " << endl << cmd << endl;
        	//Check for log in.
        	if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
			//Check the number of arguments.
            if(cmdToken.size() > 3)
            {
            	cout << "Invalid command format!" << endl;
                cout << "list" << endl << "list <start>" << endl << "list <start> <finish>" << endl;
                outFile << "Invalid command format!" << endl;
                outFile << "list" << endl << "list <start>" << endl << "list <start> <finish>" << endl;
                continue;
            }
            //Send command to server.
            sendMsg(sock, cmd);
            //Receive from server.
            numBytes = recv(sock, buffer, BUFSIZE-1, 0);
            if(numBytes <= 0)
            {
                cout << "recv() fail" << endl;
            }
            buffer[numBytes] = '\0';
            string bufStr(buffer);
            //Check for no record found condition.
            // if(bufStr == "end")
            // {
            //     cout << "No record found!" << endl;
            //     outFile << "Response: " << endl << "No record found!" << endl;
            //     continue;
            // }
            outFile << "Response: " << endl;
            //Continuous receive message from server.
            while(bufStr != "end")
            {
                cout << bufStr << endl;
                //Write the response to log.
                outFile << bufStr << endl;
                sendMsg(sock, "confirm");
                numBytes = recv(sock, buffer, BUFSIZE-1, 0);
                if(numBytes <= 0)
                {
                    cout << "recv() fail" << endl;
                }
                buffer[numBytes] = '\0';
                bufStr = string(buffer);
            }
        }
        //Handle locate
        if(cmdToken.at(0) == "locate")
        {
        	//Write command to log.
        	outFile << "Command issued: " << endl << cmd << endl;
        	//Check for log in, arg size and arg length.
        	if(!loggedIn)
			{
				cout << "Need to login first!" << endl;
				outFile << "Response: " << endl << "Need to login first!" << endl;
				continue;
			}
        	if(cmdToken.size() != 2)
        	{
        		cout << "Invalid command format! locate <location>" << endl;
        		outFile << "Response: " << endl << "Invalid command format! locate <location>" << endl;
        		continue;
        	}
        	if(cmdToken.at(1).size() > 30)
			{
				cout << "Invalid input argument length!" << endl;
				outFile << "Response: " << endl << "Invalid input argument length!" << endl;
				continue;
			}
			//Send the command to server.
        	sendMsg(sock, cmd);
        	//Receive from server.
        	numBytes = recv(sock, buffer, BUFSIZE-1, 0);
        	if(numBytes <= 0)
        	{
        		cout << "recv() fail" << endl;
        	}
        	buffer[numBytes] = '\0';
        	string bufStr(buffer);
      //   	if(bufStr == "end")
    		// {
    		// 	cout << "No record found!" << endl;
    		// 	outFile << "Response: " << endl << "No record found!" << endl;
    		// 	continue;
    		// }
    		outFile << "Response: " << endl;
    		//Continuous receive message from server.
    		while(bufStr != "end")
    		{
    			cout << bufStr << endl;
    			//Write the response to log.
    			outFile << bufStr << endl;
    			sendMsg(sock, "confirm");
    			numBytes = recv(sock, buffer, BUFSIZE-1, 0);
    			if(numBytes <= 0)
    			{
    				cout << "recv() fail" << endl;
    			}
    			buffer[numBytes] = '\0';
    			bufStr = string(buffer);
    		}
        }
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
