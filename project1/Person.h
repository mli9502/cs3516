/*
Author: Mengwen Li (mli2)
*/
#include <string>
#include <iostream>

#ifndef PERSON_H_
#define PERSON_H_

using namespace std;

class Person
{
private:
	//9 digit
	string idNum;
	//20 char
	string firstName;
	//25 char
	string lastName;
	//char, M or F
	char gender;
	//30 char
	string loc;
public:
	Person();
	Person(string _idNum, string _firstName, string _lastName, char _gender, string _loc);
	//Check whether the given loc is the person's loc.
	bool inLoc(string location);
	//Check whether the given id is the person's id.
	bool isId(string id);
	//Check whether the given name is the person's name.
	bool isName(string first, string last);
	//Check whether the name is in the given range. two args.
	bool nameInRange(string start, string end);
	//Check whether the name is in the given range. pne arg.
	bool nameInRange(string start);
	//Check whether the person is unidentified.
	bool isUnidentified();
	//Get the first name.
	string getFirstName();
	//Get the last name.
	string getLastName();
	//Convert the person to string.
	string toString();
	//Operator override.
	friend ostream& operator << (ostream&, const Person);
	~Person();
};

#endif
