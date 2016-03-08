/*
Author: Mengwen Li (mli2)
*/
/*
DataLinkLayer class for modeling the datalink layer.
*/
#include "DataLinkLayer.h"
#include <iostream>

DataLinkLayer::DataLinkLayer(){}
DataLinkLayer::~DataLinkLayer(){}
//Receive a packet from network layer.
void DataLinkLayer::recvPacket(Packet pkt)
{
	this->pkt = pkt;
	return;
}
//Split the packet into frames and return the frames.
vector<Frame> DataLinkLayer::splitPacket()
{
	vector<Frame> frames;
	//Get the payload of the packet.
	vector<char> buf = this->pkt.getPayLoad();
	//If this packet is end of photo, it can be less than 130 bytes.
	if (this->pkt.isEndofPhoto())
	{
		//If it is less than 130 bytes, generate 1 frame.
		if (buf.size() <= 130)
		{
			Frame frame_1(0, 'f', 1, buf, 1, 1);
			frames.push_back(frame_1);
		}
		//If it is more than 130 bytes, generate 2 frames.
		else
		{
			vector<char> sub_0(buf.begin(), buf.begin() + 130);
			vector<char> sub_1(buf.begin() + 130, buf.end());
			Frame frame_1(0, 'f', 0, sub_0, 1, 1);
			Frame frame_2(1, 'f', 1, sub_1, 1, 1);
			frames.push_back(frame_1);
			frames.push_back(frame_2);
		}
	}
	//If it's not the end of photo, it will have 256 bytes, so generate 2 frames.
	else
	{
		vector<char> sub_0(buf.begin(), buf.begin() + 130);
		vector<char> sub_1(buf.begin() + 130, buf.end());
		Frame frame_1(0, 'f', 0, sub_0, 0, 1);
		Frame frame_2(1, 'f', 1, sub_1, 0, 1);
		frames.push_back(frame_1);
		frames.push_back(frame_2);
	}
	//Set the error detection field for those frames.
	for (int i = 0; i < frames.size(); i++)
	{
		frames.at(i).setErrorDetec();
	}
	return frames;
}
//Getter to get the packet.
Packet DataLinkLayer::getPkt()
{
	return this->pkt;
}
