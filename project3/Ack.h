/*
Author: Yo Karita
*/
/*
Header for Ack class.
*/

#pragma once
#include <vector>
#include <iostream>

using namespace std;

class Ack
{
public:
	Ack();
	Ack(short seq, char ft, short ed);
	~Ack();
	//Get the byte representation for Ack packet. The byte representation will be sent through socket.
	vector<char> getByteRep();
	//Reconstruct the Ack from the byte representation.
	Ack reconstruct(vector<char> ackData);
	//Check whether the Ack packet is correct or not according to the given sequence number.
	bool isCorrect(int seqNum);
	//toString method to print out the information of an Ack frame.
	string toString();
	//Generate an error version of Ack frame by fliping the first bit in error detection field.
	Ack generateErrorAck();
private:
	//Sequence number, 2 bytes.
	short seq;
	//Frame type, 1 byte.
	char ft;
	//Error detection, 2 bytes.
	short ed;
};

