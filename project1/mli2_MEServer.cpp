/*
Author: Mengwen Li (mli2)
*/

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include "Person.h"
#include "PersonDb.h"
#include "helper.h"

#define BUFSIZE 200

static const int MAXPENDING = 5;

using namespace std;

int handleTCPClient(int);
//Define variables to store number of scripts, number of commands, number of packets sent and the total number of packet server sent. 
int clntScript = 0;
int clntCmd = 0;
int servPkt = 0;
int totalPktCnt = 0;
//Create a person database.
PersonDb pdb;

int main(int argc, char* argv[])
{
    //Check for command line args.
    if(argc > 1)
    {
        cout << "./mli2_MEServer" << endl;
        exit(-1);
    }
    //Defines the server port.
    in_port_t servPort = 8000;
    //Create server socket.
    int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(servSock < 0)
    {
        cout << "socket() failed" << endl;
        exit(-1);
    }
    //Construct local address structure.
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);
    //Assigns local Internet protocol address to servSock.
    int bindRtn = bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if(bindRtn < 0)
    {
        cout << "bind() failed" << endl;
        exit(-1);
    }
    //Converts the unconnected servSock to passive socket. Specifies
    //the max number of connections the kernel should queue for this sock.
    int listenRtn = listen(servSock, MAXPENDING);
    if(listenRtn < 0)
    {
        cout << "listen() failed" << endl;
        exit(-1);
    }
    while(1)
    {
        //Initiate clntAddr.
        struct sockaddr_in clntAddr;
        memset(&clntAddr, 0, sizeof(clntAddr));
        socklen_t clntAddrLen = sizeof(clntAddr);
        //Dequeue the next connection from the queue of servSock.
        int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
        if(clntSock < 0)
        {
            cout << "accept() failed" << endl;
        }
        //Increase client script count.
        clntScript++;
        //Handle client.
        int rtn = handleTCPClient(clntSock);
        //Close client socket.
        close(clntSock);
        //If receive EOF.
        if(rtn == 2)
        {
            close(servSock);
            return 0;
        }   
    }
}

//Function to handle client.
int handleTCPClient(int clntSocket)
{
    //variable to record loggin status.
    bool loggedIn = false;
    char buffer[BUFSIZE];
    vector<string> bufToken;
    string clntName;
    //Receive command from client.
    ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
    buffer[numBytesRcvd] = '\0';
    if(numBytesRcvd < 0)
    {
        cout << "recv() failed" << endl;
        exit(-1);
    }
    clntCmd++;
    while(numBytesRcvd > 0)
    {
        //Split the received string.
        string bufStr(buffer);
        bufToken = split(bufStr.c_str(), ' ');
        //login
        if(bufToken.at(0) == "login")
        {
            loggedIn = true;
            clntName = bufToken.at(1);
            string loginMsg = "Hello " + bufToken.at(1) + "!";
            //Send the message back to client.
            sendMsg(clntSocket, loginMsg);
            //Increase the packet count.
            servPkt++;
        }
        //add
        else if(bufToken.at(0) == "add" && loggedIn)
        {
            char gender = bufToken.at(4).at(0);
            bool addRtn = pdb.add(bufToken.at(1), bufToken.at(2), bufToken.at(3), gender, bufToken.at(5));
            //If add successfully, send back the original message, if not, send back error message.
            if(addRtn)
            {
                sendMsg(clntSocket, bufStr);
            }
            else
            {
                sendMsg(clntSocket, "Contains duplicate id number in database");
            }
            //Increase packet count.
            servPkt++;
        }
        //update
        else if(bufToken.at(0) == "update" && loggedIn)
        {
            char gender = bufToken.at(4).at(0);
            //Call update method in PersonDb.
            bool updateRtn = pdb.update(bufToken.at(1), bufToken.at(2), bufToken.at(3), gender, bufToken.at(5));
            //If success, send "success", if not, send error message.
            if(updateRtn)
            {
                sendMsg(clntSocket, "success");
            }
            else
            {
                sendMsg(clntSocket, "Can't find specified person!");
            }
            //Increase packet count.
            servPkt++;
        }
        //remove
        else if(bufToken.at(0) == "remove" && loggedIn)
        {
            string firstName, lastName;
            //Call remove method in PersonDb.
            bool delRtn = pdb.remove(bufToken.at(1), &firstName, &lastName);
            //If success, send back first name, last name, if not, send back error message.
            if(delRtn)
            {
                sendMsg(clntSocket, firstName + " " + lastName + " is removed");
            }
            else
            {
                sendMsg(clntSocket, "remove error: given ID number not found!");
            }
            //Increase packet count.
            servPkt++;
        }
        //find
        else if(bufToken.at(0) == "find" && loggedIn)
        {
            vector<string> result;
            //Call find method in PersonDb.
            pdb.find(bufToken.at(1), bufToken.at(2), &result);
            //Send back the returned result.
            for(int i=0; i<result.size(); i++)
            {
                //Send the result message.
                sendMsg(clntSocket, result.at(i));
                //Increase packet count.
                servPkt++;
                //Receive the confirm message.
                ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
                buffer[numBytesRcvd] = '\0';
                string confMsg(buffer);
                //If confirm message is not received, break.
                if(confMsg != "confirm")
                {
                    cout << "Not receiving confirm!" << endl;
                    break;
                }
            }
            //Send a packet indicating that the communication has ended.
            sendMsg(clntSocket, "end");
            //Increase packet count.
            servPkt++;
        }
        //list
        else if(bufToken.at(0) == "list" && loggedIn)
        {
            vector<string> result;
            //Call the list method in PersonDb according to the number of arguments.
            if(bufToken.size() == 1)
            {
                pdb.list(&result);
            }
            else if(bufToken.size() == 2)
            {
                pdb.list(bufToken.at(1), &result);
            }
            else
            {
                pdb.list(bufToken.at(1), bufToken.at(2), &result);
            }
            //Send the result back.
            for(int i=0; i<result.size(); i++)
            {
                sendMsg(clntSocket, result.at(i));
                //Increase packet count.
                servPkt++;
                //Receive the confirm message.
                ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
                buffer[numBytesRcvd] = '\0';
                string confMsg(buffer);
                if(confMsg != "confirm")
                {
                    cout << "Not receiving confirm!" << endl;
                    break;
                }
            }
            //Send the terminate message.
            sendMsg(clntSocket, "end");
            //Increase packet count.
            servPkt++;
        }
        //locate
        else if(bufToken.at(0) == "locate" && loggedIn)
        {
            vector<string> result;
            //Call locate method in PersonDb.
            pdb.locate(bufToken.at(1), &result);
            //Send the result back.
            for(int i=0; i<result.size(); i++)
            {
                sendMsg(clntSocket, result.at(i));
                //Increase packet count.
                servPkt++;
                //Receive the confirm message.
                ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
                buffer[numBytesRcvd] = '\0';
                string confMsg(buffer);
                if(confMsg != "confirm")
                {
                    cout << "Not receiving confirm!" << endl;
                    break;
                }
            }
            //Send the terminate message.
            sendMsg(clntSocket, "end");
            //Increase packet count.
            servPkt++;
        }
        //quit
        else if(bufToken.at(0) == "quit" && loggedIn)
        {
            string quitStr = "The TCP connection for client " + clntName + " will be closed." + "\n" +
                                "The total number of commends " + clntName + " sent is " + toString(clntCmd);
            //Set the command count to 0.
            clntCmd = 0;
            //Send message back to client.
            sendMsg(clntSocket, quitStr);
            servPkt++;
            //Output the number of packets sent to current client.
            cout << "The total number of packets sent to client is " << servPkt << endl;
            //Add the packet count to total packet count.
            totalPktCnt += servPkt;
            if(bufToken.size() == 2 && bufToken.at(1) == "EOF")
            {
                //Write file and send back the total number of scripts processed and total num of packets server sent.
                sendMsg(clntSocket,  "Total number of packets server sent is " + toString(totalPktCnt) + "\nClient script count is " + toString(clntScript));
                ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
                buffer[numBytesRcvd] = '\0';
                string confMsg(buffer);
                if(confMsg != "confirm")
                {
                    cout << "Not receiving confirm!" << endl;
                    break;
                }
                //Write the database to MEDatabase.txt
                ofstream outFile;
                outFile.open("MEDatabase.txt");
                vector<Person> finalDb = pdb.getDatabase();
                for(int i=0; i<finalDb.size(); i++)
                {
                    outFile << finalDb.at(i).toString() << endl;
                }
                outFile.close();
                servPkt = 0;
                return 2;
            }
            
            //Reset the packet count to 0.
            servPkt = 0;
            return 1;
        } 
        //Receive again.
        numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
        clntCmd++;
        buffer[numBytesRcvd] = '\0';
        if(numBytesRcvd < 0)
        {
            cout << "recv() failed" << endl;
            exit(-1);
        }
    }
}
