/*
Author: Mengwen Li (mli2) 
*/
/*
Header for Frame class.
*/
#pragma once
#include <vector>
#include <iostream>

using namespace std;

class Frame
{
public:
	Frame();
	/*
	Constructor for a data frame.
	A data frame contains: 2 bytes sequence number,
							1 byte frame type,
							1 byte end of packet,
							a char vector storing the data,
							1 byte end of photo indicator,
							2 types error detection.
	For the frame type, 'f' represents data frame and 'a' represents ack frame.
	*/
	Frame(short seq, char ft, char eop, vector<char> dataField, char endofPhoto, short ed);
	~Frame();
	//Get the size of the data frame.
	int getFrameSize();
	//Get the data field for the data frame.
	vector<char> getDataField();
	//Get the byte representation for the data frame in order to send through socket.
	vector<char> getByteRep();
	//Reconstruct the data frame from the char vector received from socket.
	Frame reconstruct(vector<char> frameData);
	//Set the error detection field for data frame.
	void setErrorDetec();
	//Check whether the data frame is correct or not.
	bool isCorrect();
	//toString method for printing out the data frame.
	string toString();
	//Equals mathod to check wheter the frame is duplicate.
	bool equals(Frame f);
	//Check whether the frame is the end of packet.
	bool isEndofPacket();
	//Getter for the sequence number.
	short getSeq();
	//Check whether the data frame belongs to a packet that is the end of photo.
	bool isEndofPhoto();
	//Generate an error frame by flipping the first bit in error detection field.
	Frame generateErrorFrame();
private:
	//Sequence number. 2 bytes.
	short seq;
	//Frame type, 1 byte.
	char ft;
	//End of packet. 1 byte.
	char eop;
	//Data field, less than 130 bytes.
	vector<char> dataField;
	//End of photo indicator. 1 byte.
	char endofPhoto;
	//Error detection. 2 bytes.
	short ed;
};

