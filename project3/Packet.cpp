/*
Author: Mengwen Li (mli2) 
*/
/*
Packet class. Defines the structure of an Packet.
Also defines methods for Packet.
*/

#include "Packet.h"

Packet::Packet(){}
//Constructor for Packet.
Packet::Packet(vector<char> buffer)
{
	this->payLoad = buffer;
	this->endofPhoto = 0;
}

Packet::~Packet(){}
//Get the pay load for the packet.
vector<char> Packet::getPayLoad()
{
	return this->payLoad;
}
//Check whether the packet is the end of photo or not.
bool Packet::isEndofPhoto()
{
	if(this->endofPhoto == 1)
	{
		return true;
	}
	return false;
}
//Reconstruct the packet from a vector of frames.
void Packet::reconstruct(vector<Frame> frames)
{
	this->payLoad.clear();
	for (int i = 0; i < frames.size(); i++)
	{
		for (int j = 0; j < frames.at(i).getDataField().size(); j++)
		{
			this->payLoad.push_back(frames.at(i).getDataField().at(j));
		}
	}
	if (frames.back().isEndofPhoto())
	{
		this->endofPhoto = 1;
	}
	else
	{
		this->endofPhoto = 0;
	}
}
//Set the end of photo indicator for packet.
void Packet::setEndofPhoto()
{
	this->endofPhoto = 1;
}
