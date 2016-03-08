/*
Author: Mengwen Li (mli2)
*/
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <bitset>
#include <signal.h>
#include <sys/time.h>
#include "Ack.h"
#include "Frame.h"
#include "Packet.h"
#include "DataLinkLayer.h"
#include <iomanip>

using namespace std;

//Defines the flag that controls the simulated network layer and data link layer.
bool nwl = false;
bool dll = false;
//Records the total frame count for error simulation.
long totalFrameCnt = 0;
//Record the total packet count for one photo.
int totalPktCnt = 0;
//Variable used for statistics.
int totalNumFramesSent = 0;
int totalNumFramesRetrans = 0;
int totalNumGoodAcks = 0;
int totalNumErrorAcks = 0;

ofstream outputfile;
//filename of the logfile
string logname;
//Defines a data link layer object.
DataLinkLayer dataLink;
/*Network layer function*/
//Network layer function to send packet to data link layer.
void nwl_send(Packet pkt);
/*Data link layer function*/
//Data link layer function to send Ack data frame to network layer.
int dll_send_nwl(Frame frame, bool* breakFlg, int seqNum, bool ieop);
//Data link layer function to send frame to physical layer.
void dll_send_py(Frame frame, int sock);
//Data link layer function to receive a data frame or an ack from server.
//Also simulates the physical layer receive funtion.
//The data frame received contains an ack in its payload.
int dll_recv_py(int sock, Frame* frame, Ack* ack);
/*Physical layer function*/
//Physical layer function to connect the server.
int py_connect(char*);
//Physical layer function to send frame to server side.
int py_layer_send(char* frame, size_t frameSize, int sock);
/*Helper functions*/
//Converts host name to ip address.
int htoi(char* host, char** ip);
//Converts an int to string.
string toString(int i);
//Converts time info to the string data
string timeToString(struct tm* pstart);
//Write the time and messages in the log file
void outputToLog(string logoutput);

int main(int argc, char** argv)
{
	//Get the started time and create a logfile.
	struct timeval tv_s, tv_e;
	gettimeofday(&tv_s, NULL);
	time_t now = tv_s.tv_sec;
	struct tm *pstart = localtime(&now);
	string olog;
	logname = string("client_") + *argv[2] + ".log";
	outputfile.open(logname.c_str());
	outputfile  << timeToString(pstart) << ":" <<setfill('0') << setw(3) << tv_s.tv_usec/1000 << "	" << "Process  started" << endl;
	outputfile.close();

	//
	//Define a input file stream for reading in photos.
	ifstream photo;
	//Check for command line arguments.
	if(argc != 4)
	{
		cout << "Command line argument not correct!" << endl;
		exit(-1);
	}
	//Get the client id number and the client photo count.
	int id = atoi(argv[2]);
	int numPhotos = atoi(argv[3]);
	//Defines the socket descriptor.
	int sockDscp;
	//Calls the physical layer connect function to establish connection with server.
	sockDscp = py_connect(argv[1]);
	//Transport all the given photos.
	for(int np=0; np<numPhotos; np++)
	{
		//Open the photo.
		string name = "photo" + toString(id) + toString(np + 1) + ".jpg";
		photo.open(name.c_str());
		//log: opend the photo
		olog = "Opened " + name;
		outputToLog(olog.c_str());
		//Set the network layer flag to true and data link layer flag to false
		//since network layer will start reading in packets first.
		nwl = true;
		dll = false;
		while(true)
		{
			//Break flag to break out the while loop when one photo is finished transmitting. 
			bool breakFlg = false;
			//Defines the variables used by select function.
			fd_set readfds;
			struct timeval tv;
			//Initialize the fd_set and the timer interval.
			FD_ZERO(&readfds);
			FD_SET(sockDscp, &readfds);
			tv.tv_sec = 0;
			tv.tv_usec = 250000;	
			//Start the network layer if network layer flag is true.
			if(nwl)
			{
				//Read 256 bytes data from the photo.
				char buffer[256];
				char eop = 0;
				memset(buffer, 0, 256);
				photo.read(buffer, 256);
				//Check whether the read has 256 bytes of data.
				//If not, get the number of bytes that are read in.
				int rtn;
				if(photo)
				{
					rtn = 256;
				}
				else
				{
					rtn = photo.gcount();
				} 
				//Construct a packet from the data reading in.
				vector<char> chunkBuf;
				for(int i=0; i<rtn; i++)
				{
					chunkBuf.push_back(buffer[i]);
				}
				Packet pkt(chunkBuf);
				//If the data read in is not 256 bytes, set this packet as the end of photo.
				if(rtn != 256)
				{
					pkt.setEndofPhoto();
				}				
				//Send the packet to data link layer.
				olog = "Packet is sent to the datalink layer ";
				outputToLog(olog.c_str());
				totalPktCnt ++;
				nwl_send(pkt);
			}
			//Start the data link layer if data link layer flag is true.
			else if(dll)
			{	
				//Get the frames from the packet.			
				vector<Frame> frames = dataLink.splitPacket();
				//Transfer all the frames to server.
				for(int i=0; i<frames.size(); i++)
				{
					//Defines the frame that is going to be sent.
					Frame send_frame = frames.at(i);
					//Record the sequence number to check the ack frame latter.
					int seqNum = send_frame.getSeq();
					//Set end of photo indicator.
					bool ieop = frames.at(i).isEndofPhoto();
					//Generating an error frame every 6 frames.
					if(totalFrameCnt % 6 == 0)
					{	
						send_frame = send_frame.generateErrorFrame();
					}
					//Send this frame through physical layer.
					dll_send_py(send_frame, sockDscp);
					totalNumFramesSent ++;
					olog = "Frame sent for frame " + toString(frames.at(i).getSeq()) + " of packet " + toString(totalPktCnt);
					outputToLog(olog.c_str());
					//Defines the received frame or receive ack.
					Frame recvFrame;
					Ack recvAck;			
					while(1)
					{
						//Using select funtion to set up the timer.
						int rtnVal = select(sockDscp + 1, &readfds, NULL, NULL, &tv);					
						if(rtnVal < 0)
						{
							cout << "select error" << endl;
							exit(-1);
						}
						//If the socket receives data.
						else if(rtnVal > 0 && FD_ISSET(sockDscp, &readfds))
						{
							//Call the data link layer receive funtion to receive data from physical layer.
							//Checks the return value of this funtion to determin wheter the receive data
							//represents a data frame or an ack.
							int drpRtn = dll_recv_py(sockDscp, &recvFrame, &recvAck);
							//If the received data is a data frame.
							if(drpRtn)
							{
								//Check whether the received data frame is correct by sending it to network layer.
								if(recvFrame.isCorrect())
								{
									int rtn = dll_send_nwl(recvFrame, &breakFlg, seqNum, ieop);
									//If the ack packet contained in the received data frame is correct,
									//reset the timer and send the next frame. 
									//If not, keeps waiting for timeout to retransmit.
									if(rtn == 1)
									{
										olog = "Ack frame received successfully for frame " + toString(frames.at(i).getSeq()) + " of packet " + toString(totalPktCnt);
										totalNumGoodAcks ++;
										outputToLog(olog.c_str());
										FD_ZERO(&readfds);
										FD_SET(sockDscp, &readfds);
										tv.tv_sec = 0;
										tv.tv_usec = 250000;
										break;
									}
									else
									{
										olog = "Ack frame received in error for frame " + toString(frames.at(i).getSeq()) + " of packet " + toString(totalPktCnt);
										outputToLog(olog.c_str());
										totalNumErrorAcks ++;
									}
								}								
							}
							//If the received data is an ack.
							else
							{
								//If the received ack is correct, reset the timer and send the next frame.
								//If not, keeps waiting for timeout to retransmit.
								if(recvAck.isCorrect(seqNum))
								{
									olog = "Ack packet received successfully for frame " + toString(frames.at(i).getSeq()) + " of packet " + toString(totalPktCnt);
									totalNumGoodAcks ++;
									outputToLog(olog.c_str());
									FD_ZERO(&readfds);
									FD_SET(sockDscp, &readfds);
									tv.tv_sec = 0;
									tv.tv_usec = 250000;
									break;
								}
								else
								{
									olog = "Ack packet received in error for frame " + toString(frames.at(i).getSeq()) + " of packet " + toString(totalPktCnt);
									outputToLog(olog.c_str());
									totalNumErrorAcks ++;
								}
							}							
						}
						//If the timer expires.
						else
						{
							olog = "Timer was expired";
							outputToLog(olog.c_str());
							//Retransmit the correct frame by physical layer.
							dll_send_py(frames.at(i), sockDscp);
							totalNumFramesRetrans ++;
							totalNumFramesSent ++;
							olog = "Frame resent for frame " + toString(frames.at(i).getSeq()) + " of packet " + toString(totalPktCnt);
							outputToLog(olog.c_str());
							//Reset the timer and waiting for server side ack.
							tv.tv_sec = 0;
							tv.tv_usec = 250000;
							FD_ZERO(&readfds);
							FD_SET(sockDscp, &readfds);
							continue;
						}
					}
					//Increase the total frame count after sending a frame.
					//The the retransitted frames are not counted when simulating the error.
					totalFrameCnt ++;
				}
				//If the break flag is set, break out the while loop and send the next photo.
				if(breakFlg)
				{
					break;
				}
			}
		}
		//Close the photo after sending it and reset the total packet count.
		photo.close();
		totalPktCnt = 0;
		olog = "Closed " + name;
		outputToLog(olog.c_str());
        outputfile.open(logname.c_str(),  ios::app);
		//Output the statistics.
        outputfile << "The total number of frames sent (including retransmitted frames) is " << toString(totalNumFramesSent) << endl;
        outputfile << "The total number of frames retransmitted is " << toString(totalNumFramesRetrans) << endl;
        outputfile << "The total number of good ACKs (including good Ack and good data frame including ack) received is " << toString(totalNumGoodAcks) << endl;
        outputfile << "The total number of error ACKs (including error Ack and data frame including error ack) received is " << toString(totalNumErrorAcks) << endl;
        totalNumErrorAcks = 0;
        totalNumGoodAcks = 0;
        totalNumFramesRetrans = 0;
        totalNumFramesSent = 0;
        outputfile.close();
    }
	//Close the socket after sending all the photos.
	close(sockDscp);
	gettimeofday(&tv_e, NULL);
	now = tv_e.tv_sec;
	struct tm *pend = localtime(&now);
	outputfile.open(logname.c_str(),  ios::app);
	int milisec = ((tv_e.tv_sec - tv_s.tv_sec) * 1000 + (tv_e.tv_usec - tv_s.tv_usec) / 1000);
	string executiontime =toString(milisec/1000/60)+"m"+toString(milisec/1000%60) +"s"+ toString(milisec % 1000) + "ms";
	outputfile  << timeToString(pend) << ":" <<setfill('0') << setw(3) << tv_e.tv_usec/1000 << "	" << "Process ended"<< endl;	
	outputfile <<"The total exectution time is "<< executiontime << endl;
	outputfile.close();
}
//Data link layer function to send Ack data frame to network layer.
//Also simulates the check in network layer.
int dll_send_nwl(Frame frame, bool* breakFlg, int seqNum, bool ieop)
{
	//Get the ack from the data frame.
	Ack ack;
	ack = ack.reconstruct(frame.getDataField());
	//If the ack is correct.
	if(ack.reconstruct(frame.getDataField()).isCorrect(seqNum))
	{
		//If this frame is the end of photo, set the break flag.
		if(ieop)
		{
			*breakFlg = true;
		}
		nwl = true;
		dll = false;
		return 1;
	}
	//If the ack is not correct, return -1.
	else
	{
		return -1;
	}
}
//Network layer function to send packet to data link layer.
void nwl_send(Packet pkt)
{
	//Set data link layer flag to true.
	dll = true;
	//Call the recvPacket() method in DataLinkLayer class to receive packet from network layer.
	dataLink.recvPacket(pkt);
	//Set network layer flag to false.
	nwl = false;
}
//Data link layer function to receive a data frame or an ack from server.
//Also simulates the physical layer receive funtion.
//The data frame received contains an ack in its payload.
int dll_recv_py(int sock, Frame* frame, Ack* ack)
{
	//Set up the receive buffer.
	int size = 150;
	ssize_t numBytes = 0;
	char buffer[size];
	//Use recv() funtion to receive from server.
	numBytes = recv(sock, buffer, size, 0);
	vector<char> objBuf;
	for(int i=0; i<numBytes; i++)
	{
		objBuf.push_back(buffer[i]);
	}
	//Check the frame type field to determie whether it's a data frame or an ack.
	//If it's an ack, return 0, if it's a data frame, return 1.
	if(objBuf.at(2) == 'a')
	{
		Ack tempAck;
		*ack = tempAck.reconstruct(objBuf);
		return 0;
	}
	else
	{
		Frame tempFrame;
		*frame = tempFrame.reconstruct(objBuf);
		return 1;
	}
}
//Data link layer function to send frame to physical layer.
void dll_send_py(Frame frame, int sock)
{
	//Get the size of the frame.
	size_t frameSize = frame.getFrameSize();
	//Convert the data frame object to a char array.
	char frameData[frameSize];
	vector<char> frameByteRep = frame.getByteRep();
	for(int i=0; i<frameSize; i++)
	{
		frameData[i] = frameByteRep.at(i);
	}
	//Call the physical layer send funtion to send the frame.
	ssize_t numBytes = py_layer_send(frameData, frameSize, sock);
}
//Physical layer function to send frame to server side.
int py_layer_send(char* frame, size_t frameSize, int sock)
{
	ssize_t numBytes = send(sock, frame, frameSize, 0);
	if(numBytes != frameSize)
	{
		cout << "send() error!" << endl;
		exit(-1);
	}
	return numBytes;
}

//Converts host name to IPv4 address.
int htoi(char* host, char** ip)
{
	struct hostent *ht;
	ht = gethostbyname(host);
	if(ht == NULL)
	{
		cout << "gethostbyname() fail" << endl;
		exit(-1);
	}
	*ip = inet_ntoa(*((struct in_addr *)ht->h_addr_list[0]));
	return 0;
}
//Physical layer function to connect the server.
int py_connect(char* serverName)
{
	//Establish connection here.
	char* servIP;
	htoi(serverName, &servIP);
	//Define the server port.
	in_port_t servPort = 5000;
	//Create socket using IPv4 and TCP protocol.
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		cout << "socket() fail" << endl;
		exit(-1);
	}
	//Set up server address struct.
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
	if(rtnVal == 0)
	{
		cout << "inet_pton() failed" << "invalid address string" << endl;
		exit(-1);
	}
	else if(rtnVal < 0)
	{
		cout << "inet_pton() failed" << endl;
		exit(-1);
	}
	servAddr.sin_port = htons(servPort);
	int cntVal = connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if(cntVal < 0)
	{
		cout << "connect() failed" << endl;
		exit(-1);
	}
	return sock;
}
//Converts an int to string.
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
