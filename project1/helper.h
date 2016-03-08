/*
Author: Mengwen Li (mli2)
*/
#pragma once

#include "Person.h"
#include <vector>
#include <string>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef HELPER_H_
#define HELPER_H_
//Function to compare two persons according to last name.
bool compare(Person p1, Person p2);
//Function to split string with given char.
vector<string> split(const char*, char);
//Function to convert int to string.
string toString(int);
//Function to send message through sock.
void sendMsg(int sock, string msgStr);

#endif

