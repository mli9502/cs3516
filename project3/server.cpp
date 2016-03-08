/*
Author: Yo Karita 
*/
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "Ack.h"
#include "Frame.h"
#include "Packet.h"
#include <iomanip>

using namespace std;
//Defins the max pending connections.
static const int MAXPENDING  = 5;
//Defines the output file stream to write the new photo.
ofstream photo;
//Defines the total client count and the total ack packets sent.
int clntCnt = 0;
int ackCnt = 0;
ofstream outputfile;
//Since we decide to open a new photo file right after finish writing the first one,
//we will have one extra file with 0 byte in it after receiving all the files.
//To deal with this, we just delete the extra file.
string extraFile;
//filename of the logfile
string logname;

bool breakFlg = false;
//Data link layer function to receive a data frame from client.
//Also simulates the physical layer receive function.
Frame dll_recv_py(int sock);
//Data link layer function to send the received packet to network layer 
//and also received an ack back.
Ack dll_send_nwl(Packet packet, int* photoCnt, int clientCnt, int seqNum);
//Network layer function to receive a packet from data link layer and send back an ack.
Ack nwl_recv_dll(Packet packet, int* photoCnt, int clientCnt, int seqNum);
//Physical layer function to send a data frame that has an ack in payload to client side.
ssize_t py_send_frame(Frame frame, int sock);
//Physical layer function to send an ack frame to client side.
ssize_t py_send_ack(Ack ack, int sock);
//Function to handle a client.
int handleTCPClient(int clntSock, int clientCnt);
//Helper function to convert an it to string.
string toString(int i);
//Converts time info to the string data
string timeToString(struct tm* pstart);
//Write the time and messages in the log file
void outputToLog(string logoutput);

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		cout << "Command line arg invalid." << endl;
		exit(-1);
	}
	//Define the server port.
	in_port_t servPort = 5000;
	//Create server sock.
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
	//Wait for client connection.
	int listenRtn = listen(servSock, MAXPENDING);
	if(listenRtn < 0)
	{
		cout << "listen() failed" << endl;
		exit(-1);
	}
	while(1)
	{
		struct sockaddr_in clntAddr;
		memset(&clntAddr, 0, sizeof(clntAddr));
		socklen_t clntAddrLen = sizeof(clntAddr);
		//Get the socket of the connected client.
		int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
		if(clntSock < 0)
		{
			cout << "accept() failed" << endl;
		}
		else
		{
			//Fork a new process to handle the connected client.
			pid_t pid = fork();
			//If it is the child process, call handleTCPClient funtion to handle the incoming packets.
			if(pid == 0)
			{
				int rtn = handleTCPClient(clntSock, clntCnt);
				photo.close();
                remove(extraFile.c_str());
				close(clntSock);
			}
			//If it is the father process, increase the client count.
			else if(pid > 0)
			{
				clntCnt ++;
			}
			else
			{
				cout << "Fork error!" << endl;
				exit(-1);
			}
		}
	}
}
//Function to handle a client.
int handleTCPClient(int clntSock, int clientCnt)
{
	//Creates a log file and records the started time
	struct timeval tv_s, tv_e;
	gettimeofday(&tv_s, NULL);
	time_t now = tv_s.tv_sec;
	struct tm *pstart = localtime(&now);
	string olog;
	logname = string("server_") + toString(clientCnt+1) + ".log";
	outputfile.open(logname.c_str());
	outputfile  << timeToString(pstart) << ":" <<setfill('0') << setw(3) << tv_s.tv_usec/1000 << "	" << "Connected with Client"<< toString(clientCnt+1) << endl;
	outputfile.close();
	//Record the photo count.
	int photoCnt = 0;
	vector<Frame> frames;
	//Open the first new photo.
	string name = "photonew" + toString(clientCnt + 1) + toString(photoCnt) + ".jpg";
	photo.open(name.c_str());
	while(1)
	{
		//Receive a data frame from client.
		Frame frame = dll_recv_py(clntSock);
		if(breakFlg)
		{
			break;
		}
        olog = "client id: " + toString(clntCnt + 1) + " Received a frame from the client";
        outputToLog(olog.c_str());
		//Check whether the received frame is correct or not.
		//If the frame is not correct, record the event, drop it and do nothing.
		//If the frame is duplicate, record event and send back an ack.
		if(frame.isCorrect())
		{
			//If this is the first frame, push to frames directly.
			if(frames.empty())
			{
				frames.push_back(frame);
			}
			else
			{
				//If it is an duplicate frame, drop the frame and send back an ack.
				if(frames.back().equals(frame))
				{
					olog = "client id: " + toString(clntCnt + 1) + " Received a duplicated frame.";
					outputToLog(olog.c_str());
					//If the duplicate frame is the end of packet, send back a frame with ack in payload.
					if(frames.back().isEndofPacket())
					{
						Ack ack(frames.back().getSeq(), 'a', frames.back().getSeq());
						vector<char> ackByteRep = ack.getByteRep();
						Frame ackFrame(0, 'f', 1, ackByteRep, 0, 0);
						ackFrame.setErrorDetec();
						py_send_frame(ackFrame, clntSock);
						olog = "client id: " + toString(clntCnt + 1) + " ACK frame was sent to the client.";
						outputToLog(olog.c_str());
						frames.clear();
					}
					//If not, just send back an ack.
					else
					{
						Ack dup_ack(frames.back().getSeq(), 'a', frames.back().getSeq());
						py_send_ack(dup_ack, clntSock);
						olog = "client id: " + toString(clntCnt + 1) + " ACK packet was sent the client.";
						outputToLog(olog.c_str());
					}		
					continue;
				}
				//If the frame is not duplicate, push it back to frames.
				if(!frames.back().equals(frame))
				{
					frames.push_back(frame);
				}
			}
			//If the last frame of the received frames is the end of packet, send back a data frame with ack in payload.
			if(frames.back().isEndofPacket())
			{
				//Set up the clear flag to detemine whether the frame history needs to be cleared.
				//If a wrong ack is generated, set the clear flag to false so the duplicate frame will be detected.
				bool clearFlg = true;
				Packet packet;
				//Reconstruct the packet from the frame history.
				packet.reconstruct(frames);
				//Get the sequence number of the frame.
				short seqNum = frames.back().getSeq();
				//Send packet to network layer and receive ack back.
				Ack ack = dll_send_nwl(packet, &photoCnt, clientCnt, seqNum);
				olog = "client id: " + toString(clntCnt + 1) + " A packet was sent the network layer.";
				outputToLog(olog.c_str());
				//Generate error ack if ackCnt % 11 is 0.
				if(ackCnt % 11 == 0)
				{
					ack = ack.generateErrorAck();
					clearFlg = false;
				}
				//Get the byte representation of the ack in order to put the ack in the data frame payload.
				vector<char> ackByteRep = ack.getByteRep();
				Frame ackFrame(0, 'f', 1, ackByteRep, 0, 0);
				//Set the error detection of the ack frame.
				ackFrame.setErrorDetec();
				//Send this frame through physical layer to client.
				py_send_frame(ackFrame, clntSock);
				//Increase the ack count and clear the history accordingly.
				ackCnt ++;
				if(clearFlg)
				{
					frames.clear();
				}
			}
			//If the last frame is not the end of packet, send an ack back.
			else
			{
				Ack ack(frames.back().getSeq(), 'a', frames.back().getSeq());
				//Generate error ack if ackCnt % 11 is 0.
				if(ackCnt % 11 == 0)
				{
					ack = ack.generateErrorAck();
				}
				//Send this ack through physical layer to client.
				py_send_ack(ack, clntSock);
				olog = "client id: " + toString(clntCnt + 1) + " ACK packet was sent to the client.";
				outputToLog(olog.c_str());
				//Increase the ack count.
				ackCnt ++;
			}

		}
		else
		{
			olog = "client id: " + toString(clntCnt + 1) + " Received a frame in error.";
			outputToLog(olog.c_str());
		}
	}
	olog = "client id: " + toString(clntCnt + 1) + " Connection closed";
	outputToLog(olog.c_str());
}
//Data link layer function to receive a data frame from client.
//Also simulates the physical layer receive function.
Frame dll_recv_py(int sock)
{
	ssize_t numBytes = 0;
	char buffer[150];
	numBytes = recv(sock, buffer, 150, 0);
	if(numBytes <= 0)
	{
		//cout << "recv() failed" << endl;
		//exit(-1);
		breakFlg = true;
		return Frame();
	}
	//Reconstruct and return the received frame from the char array received.
	vector<char> frameBuf;
	for(int i=0; i<numBytes; i++)
	{
		frameBuf.push_back(buffer[i]);
	}
	Frame frame; 
	return frame.reconstruct(frameBuf);
}
//Data link layer function to send the received packet to network layer 
//and also received an ack back.
Ack dll_send_nwl(Packet packet, int* photoCnt, int clientCnt, int seqNum)
{
	//Calls the network layer receive funtion to get an ack back.
	Ack ack = nwl_recv_dll(packet, photoCnt, clientCnt, seqNum);
	return ack;
}
//Network layer function to receive a packet from data link layer and send back an ack.
Ack nwl_recv_dll(Packet packet, int* photoCnt, int clientCnt, int seqNum)
{
	//Get the data field from the packet.
	vector<char> data = packet.getPayLoad();
	//Write the data to photo.
	for(int i=0; i<data.size(); i++)
	{
		photo << data.at(i);
	}
	//If this packet is the end of photo, increase the photo cnt and open a new photo for write.
	if(packet.isEndofPhoto())
	{
		(*photoCnt) ++;
		photo.close();
		string name = "photonew" + toString(clientCnt + 1) + toString(*photoCnt) + ".jpg";
		extraFile = name;
		photo.open(name.c_str());
	}
	//Returns an ack frame.
	Ack ack(seqNum, 'a', seqNum);
	return ack;
}
//Physical layer function to send a data frame that has an ack in payload to client side.
ssize_t py_send_frame(Frame frame, int sock)
{
	//Get the size and the byte representation of the frame.
	size_t frameSize = frame.getFrameSize();
	char frameData[frameSize];
	vector<char> frameByteRep = frame.getByteRep();
	for(int i=0; i<frameSize; i++)
	{
		frameData[i] = frameByteRep.at(i);
	}
	//Send the frame through socket.
	ssize_t numBytes = send(sock, frameData, frameSize, 0);
	if(numBytes != frameSize)
	{
		cout << "send() error!" << endl;
		exit(-1);
	}
	return numBytes;
}
//Physical layer function to send an ack frame to client side.
ssize_t py_send_ack(Ack ack, int sock)
{
	//Get the size and the byte representation fo the ack.
	size_t ackSize = 5;
	char ackData[ackSize];
	vector<char> ackByteRep = ack.getByteRep();
	for(int i=0; i<ackSize; i++)
	{
		ackData[i] = ackByteRep.at(i);
	}
	//Send the ack through socket.
	ssize_t numBytes = send(sock, ackData, ackSize, 0);
	if(numBytes != ackSize)
	{
		cout << "send() error!" << endl;
		exit(-1);
	}
	return numBytes;
}
//Helper function to convert an it to string.
string toString(int i)
{
    string result;
    std::ostringstream conv;
    conv << i;
    result = conv.str();
    return result;
}
//Converts time info to the string data
string timeToString(struct tm* pstart){
	stringstream sstream;
	sstream << setfill('0') << setw(2) <<  pstart->tm_hour << ":" << setfill('0') << setw(2) << pstart->tm_min << ":" << setfill('0') << setw(2) << pstart->tm_sec;
	return sstream.str();
}
//Write the time and messages in the log file
void outputToLog(string logoutput){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t now = tv.tv_sec;
	struct tm *pnow = localtime(&now);
	outputfile.open(logname.c_str(), ios::app);
	outputfile << timeToString(pnow) << ":" <<setfill('0') << setw(3) << tv.tv_usec/1000 << "	" << logoutput << endl;
	outputfile.close();
	return;
}

