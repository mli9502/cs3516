/*
Author: Mengwen Li (mli2)
*/
/*
Ack class. Defines the structure of an Ack packet.
Also defines methods for Ack.
*/
#include "Ack.h"
#include "stdlib.h"
#include <vector>
#include <sstream>
#include <bitset>

using namespace std;
//Default constructor for Ack.
Ack::Ack(){}
//Construct an Ack with sequence number, frame type and error detection.
Ack::Ack(short seq, char ft, short ed)
{
	this->seq = seq;
	this->ft = ft;
	this->ed = ed;
}
//Destructor.
Ack::~Ack(){}
//Get the byte representation for Ack packet.
//The byte representation will be sent through socket.
vector<char> Ack::getByteRep()
{
	vector<char> byteRep;
	byteRep.push_back(seq >> 8);
	byteRep.push_back(char(seq));
	byteRep.push_back(ft);
	byteRep.push_back(ed >> 8);
	byteRep.push_back(ed);
	return byteRep;
}
//Reconstruct the Ack from the byte representation.
Ack Ack::reconstruct(vector<char> ackData)
{
	short seq = ackData.at(0) << 8 | (ackData.at(1) & 0x00ff);
	char ft = ackData.at(2);
	short ed = ackData.at(3) << 8 | (ackData.at(4) & 0x00ff);
	return Ack(seq, ft, ed);
}
//Check whether the Ack packet is correct or not according to the given sequence number.
bool Ack::isCorrect(int seqNum)
{
	if(this->seq == seqNum && this->seq == this->ed)
	{
		return true;
	}
	return false;
}
//toString method to print out the information of an Ack frame.
string Ack::toString()
{
	stringstream ss;
	ss << "seq: " << seq << " ft: " << ft << " ed: " << ed;
	return ss.str();
}
//Generate an error version of Ack frame by fliping the first bit in error detection field.
Ack Ack::generateErrorAck()
{
	//Get the first bit of ed field.
	short temp = (this->ed >> 15) & 0x0001;
	short nEd;
	//Flip the first bit depending on whether it is 0 or 1.
	if (temp == 1)
	{
		nEd = this->ed & 0x7fff;
	}
	else
	{
		nEd = this->ed | 0x8000;
	}
	// bitset<16> ed(this->ed);
	// bitset<16> ned(nEd);
	// cout << ed << endl;
	// cout << ned << endl;
	return Ack(this->seq, this->ft, nEd);
}
