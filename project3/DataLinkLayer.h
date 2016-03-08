/*
Author: Yo Karita
*/
/*
Header for DataLinkLayer class.
*/
#pragma once

#include <vector>
#include "Frame.h" 
#include "Ack.h"
#include "Packet.h"

using namespace std;

class DataLinkLayer
{
public:
	DataLinkLayer();
	~DataLinkLayer();
	//Receive a packet from network layer.
	void recvPacket(Packet pkt);
	//Split the packet into frames and return the frames.
	vector<Frame> splitPacket();
	//Getter to get the packet.
	Packet getPkt();
private:
	//The packet received from network layer.
	Packet pkt;
	//The frames generated after splitting the packets.
	vector<Frame> frames;
};

