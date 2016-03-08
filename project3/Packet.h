/*
Author: Mengwen Li (mli2) 
*/
/*
Header for Packet class.
*/

#pragma once
#include <vector>
#include "Frame.h"

using namespace std;

class Packet
{
public:
	Packet();
    //Constructor for Packet.
	Packet(vector<char> buffer);
	~Packet();
    //Get the pay load for the packet.
	vector<char> getPayLoad();
    //Check whether the packet is the end of photo or not.
	bool isEndofPhoto();
    //Reconstruct the packet from a vector of frames.
	void reconstruct(vector<Frame> frames);
    //Set the end of photo indicator for packet.
	void setEndofPhoto();

private:
    //The payload of packet, the data is read in from the photo file.
	vector<char> payLoad;
    //The end of photo indicator.
	int endofPhoto;
};

