/*
Author: Mengwen Li (mli2) 
*/
/*
Frame class. Defines the structure of an data frame packet.
Also defines methods for data frame.
*/
#include "Frame.h"
#include <stdlib.h>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <sstream>

using namespace std;
/*
Constructor for a data frame.
A data frame contains: 2 bytes sequence number,
						1 byte frame type,
						1 byte end of packet,
						a char vector storing the data,
						1 byte end of photo indicator,
						2 types error detection.
*/
Frame::Frame(short seq, char ft, char eop, vector<char> dataField, char endofPhoto, short ed)
{
	this->seq = seq;
	this->ft = ft;
	this->eop = eop;
	this->dataField = dataField;
	this->ed = ed;
	this->endofPhoto = endofPhoto;
}

Frame::Frame(){}

Frame::~Frame(){}
//toString method for printing out the data frame.
string Frame::toString()
{
	// bitset<8> b1(dataField.front());
	// bitset<8> b2(dataField.back());
	stringstream ss;
	ss << "seq: " << seq << " ft: " << ft << " eop: " << eop << " ed: " << ed
		<< " size: " << this->dataField.size() << endl;
	return ss.str();
}
//Get the size of the data frame.
int Frame::getFrameSize()
{
	int size = 7 + this->dataField.size();
	return size;
}
//Get the data field for the data frame.
vector<char> Frame::getDataField()
{
	return this->dataField;
}
//Get the byte representation for the data frame in order to send through socket.
vector<char> Frame::getByteRep()
{
	int size = this->dataField.size() + 7;	
	vector<char> byteRep;
	byteRep.push_back(seq >> 8);
	byteRep.push_back(char(seq));
	byteRep.push_back(ft);
	byteRep.push_back(eop);
	for (int i = 0; i < this->dataField.size(); i++)
	{
		byteRep.push_back(this->dataField.at(i));
	}
	byteRep.push_back(endofPhoto);
	byteRep.push_back(ed >> 8);
	byteRep.push_back(ed);
	return byteRep;
}
//Reconstruct the data frame from the char vector received from socket.
Frame Frame::reconstruct(vector<char> data)
{
	int i;
	short seq = data.at(0) << 8 | (data.at(1) & 0x00ff);
	char ft = data.at(2);
	char eop = data.at(3);
	vector<char> dataField;
	for (i = 0; i < data.size() - 7; i++)
	{
		dataField.push_back(data.at(i + 4));
	}
	char endofPhoto = data.at(data.size() - 3);
	short ed = data.at(data.size() - 2) << 8 | (data.at(data.size() - 1) & 0x00ff);
	return Frame(seq, ft, eop, dataField, endofPhoto, ed);
}
//Set the error detection field for data frame.
void Frame::setErrorDetec()
{
	//Get the size of the frame without ed field.
	int size = this->dataField.size() + 5;
	vector<short> temp;
	short sum = 0;
	//Get the byte representation of the frame.
	vector<char> byteRep = this->getByteRep();
	//Regroup the byte representation to 16-bits group.
	if (size % 2 == 0)
	{
		for (int i = 0; i < size; i += 2)
		{
			temp.push_back(byteRep.at(i) << 8 | byteRep.at(i + 1) & 0x00ff);
		}
	}
	else
	{
		int i;
		for (i = 0; i < size - 1; i += 2)
		{
			temp.push_back(byteRep.at(i) << 8 | (byteRep.at(i + 1) & 0x00ff));
		}
		temp.push_back(byteRep[i] << 8);
	}
	//Add the 16 bit values together.
	for (int i = 0; i < temp.size(); i++)
	{
		sum += temp.at(i);
	}
	//Get the inverse of the sum and assigns this to error detection field.
	this->ed = ~sum;
}
//Check whether the data frame is correct or not.
bool Frame::isCorrect()
{
	//Get the size of the frame without ed field.
	int size = this->dataField.size() + 5;
	vector<short> temp;
	short sum = 0;
	//Get the byte representation of the frame.
	vector<char> byteRep = this->getByteRep();
	//Regroup the byte representation to 16-bits group.
	if (size % 2 == 0)
	{
		for (int i = 0; i < size; i += 2)
		{
			temp.push_back(byteRep.at(i) << 8 | byteRep.at(i + 1) & 0x00ff);
		}
	}
	else
	{
		int i;
		for (i = 0; i < size - 1; i += 2)
		{
			temp.push_back(byteRep.at(i) << 8 | (byteRep.at(i + 1) & 0x00ff));
		}
		temp.push_back(byteRep[i] << 8);
	}
	//Add the 16 bit values together.
	for (int i = 0; i < temp.size(); i++)
	{
		sum += temp.at(i);
	}
	//Check whether the inverse of this value equals to the error detection field.
	if (~sum == this->ed)
	{
		return true;
	}
	return false;
}
//Equals mathod to check wheter the frame is duplicate.
bool Frame::equals(Frame f)
{
	if (f.seq == this->seq &&
		f.ft == this->ft &&
		f.eop == this->eop &&
		f.dataField == this->dataField &&
		f.endofPhoto == this->endofPhoto &&
		f.ed == this->ed)
	{
		return true;
	}
	return false;
}
//Check whether the frame is the end of packet.
bool Frame::isEndofPacket()
{
	return this->eop == 1;
}
//Getter for the sequence number.
short Frame::getSeq()
{
	return this->seq;
}
//Check whether the data frame belongs to a packet that is the end of photo.
bool Frame::isEndofPhoto()
{
	return this->endofPhoto == 1;
}
//Generate an error frame by flipping the first bit in error detection field.
Frame Frame::generateErrorFrame()
{
	//Check whether the first bit is 0 or 1.
	short temp = (this->ed >> 15) & 0x0001;
	short nEd;
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
	//Return a data frame with new error detection field.
	return Frame(this->seq, this->ft, this->eop, this->dataField, this->endofPhoto, nEd);
}
