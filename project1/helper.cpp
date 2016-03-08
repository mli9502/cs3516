/*
Author: Mengwen Li (mli2)
*/

#include "helper.h"

using namespace std;
//Function used to compare two persons based on their last name.
bool compare(Person p1, Person p2)
{
	int result = p2.getLastName().compare(p1.getLastName());
	if (result > 0)
	{
		return true;
	}
	return false;
}
//Function used to tokenlize a string with given character.
vector<string> split(const char *input, char c)
{
    vector<string> result;
    const char* temp;
    while (*input != 0)
    {
        while (*input == c && *input != 0)
        {
            input++;
            if(*input == 0)
            {
            	return result;
            }
        }
        temp = input;
        while (*input != c && *input != 0)
        {
            input++;
        }
        result.push_back(string(temp, input));
    }
    return result;
}
//Function to convert an int to a string.
string toString(int i)
{
    string result;
    std::ostringstream conv;
    conv << i;
    result = conv.str();
    return result;
}
//Function to send message through the given socket.
void sendMsg(int sock, string msgStr)
{
	size_t msgLen = strlen(msgStr.c_str());
	ssize_t numBytes = send(sock, msgStr.c_str(), msgLen, 0);
	if(numBytes != msgLen)
	{
		cout << "send() error" << endl;
		exit(-1);
	}
	return;
}


