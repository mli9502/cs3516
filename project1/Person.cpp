/*
Author: Mengwen Li (mli2)
*/

#include <iostream>
#include <algorithm>
#include "Person.h"
//Constructor for person.
Person::Person()
{
	idNum = "";
	firstName = "";
	lastName = "";
	gender = 0;
	loc = "";
}
//Constructor for person.
Person::Person(string idNum, string firstName, string lastName, char gender, string loc)
{
	this->idNum = idNum;
	this->firstName = firstName;
	this->lastName = lastName;
	this->gender = gender;
	this->loc = loc;
}

Person::~Person() {}
//Get the first name.
string Person::getFirstName()
{
	return Person::firstName;
}
//Get the last name.
string Person::getLastName()
{
	return Person::lastName;
}
//Check whether the person is found in give location.
bool Person::inLoc(string location)
{
	string inLoc = location;
	string tmpLoc = Person::loc;
	transform(location.begin(), location.end(), inLoc.begin(), ::tolower);
	transform(loc.begin(), loc.end(), tmpLoc.begin(), ::tolower);
	int res = inLoc.compare(tmpLoc);
	if (res == 0)
	{
		return true;
	}
	return false;
}
//Check whether the person's id is the same with the given id.
bool Person::isId(string id)
{
	//cout << "input id is " << id << endl;
	//cout << "Person::idNum is " << Person::idNum << endl;
	int res = id.compare(Person::idNum);
	if (res == 0)
	{
		return true;
	}
	return false;
}
//Check whether the person's name is the given name.
bool Person::isName(string first, string last)
{
	int resFirst = Person::firstName.compare(first);
	int resLast = Person::lastName.compare(last);
	if (resFirst == 0 && resLast == 0)
	{
		return true;
	}
	return false;
}
//Check whether the person's name is in range, two args.
bool Person::nameInRange(string start, string end)
{
	//If no name, don't count.
	if ((this->lastName).compare("UNK") == 0)
	{
		return false;
	}

	char firstChar = lastName.c_str()[0];
	char startChar = start.c_str()[0];
	char endChar = end.c_str()[0];

	if (firstChar >= startChar && firstChar <= endChar)
	{
		return true;
	}
	return false;
}
//Check whether the person's name is in range, one arg.
bool Person::nameInRange(string start)
{
	//If no name, don't count.
	if (this->isUnidentified())
	{
		return false;
	}

	char startChar = start.c_str()[0];
	char firstChar = lastName.c_str()[0];
	if (firstChar == startChar)
	{
		return true;
	}
	return false;
}
//Check whether the person is unidentified or not.
bool Person::isUnidentified()
{
	if ((this->lastName).compare("UNK") == 0 &&
		(this->firstName).compare("UNK") == 0)
	{
		return true;
	}
	return false;
}
//Convert the person to string.
string Person::toString()
{
	string result = "";
	string genderStr;
	genderStr.push_back(gender);
	return result + idNum + " " + firstName + " " + 
			lastName + " " + genderStr + " " + loc;
}
//overload << to output person.
ostream& operator << (ostream& output, const Person p)
{
	output << "ID: " << p.idNum << "\t" << "First Name: " << p.firstName << "\t" <<
		"Last Name: " << p.lastName << "\t" << "Gender: " << p.gender << "\t" <<
		"Location: " << p.loc << endl;
	return output;
}
