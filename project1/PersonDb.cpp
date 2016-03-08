/*
Author: Mengwen Li (mli2)
*/

#include <iostream>
#include <algorithm>
#include "PersonDb.h"
#include "helper.h"

using namespace std;

PersonDb::PersonDb(){}

PersonDb::~PersonDb(){}

//Sort the person database according to last name.
void PersonDb::dbSort()
{
	vector<Person> idfPerson;
	vector<Person> unkPerson;
	//Seperate unk person with identified person first.
	for(int i = 0; i < personDb.size(); i++)
	{
		if(personDb.at(i).isUnidentified())
		{
			unkPerson.push_back(personDb.at(i));
		}
		else
		{
			idfPerson.push_back(personDb.at(i));
		}
	}
	//sort the identified person.
	if(!idfPerson.empty())
	{
		stable_sort(idfPerson.begin(), idfPerson.end(), compare);
	}
	//Combine two parts together.	
	for(int i = 0; i < idfPerson.size(); i++)
	{
		personDb.at(i) = idfPerson.at(i);
	}
	for(int i = idfPerson.size(); i < idfPerson.size() + unkPerson.size(); i++)
	{
		personDb.at(i) = unkPerson.at(i - idfPerson.size());
	}
	return;
}
//Add person to database.
bool PersonDb::add(string id, string first, string last, char gender, string loc)
{
	//Check for size.
	if (personDb.size() > 100)
	{
		return false;
	}
	//Check for duplicate.
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).isId(id))
		{
			//Send error message back!
			return false;
		}
	}
	//Convert name, loc to upper case.
	transform(first.begin(), first.end(), first.begin(), ::toupper);
	transform(last.begin(), last.end(), last.begin(), ::toupper);
	transform(loc.begin(), loc.end(), loc.begin(), ::toupper);
	Person p(id, first, last, gender, loc);
	personDb.push_back(p);
	//Do a dbSort.
	dbSort();
	return true;
}
//Update a person's information.
bool PersonDb::update(string id, string first, string last, char gender, string loc)
{
	transform(first.begin(), first.end(), first.begin(), ::toupper);
	transform(last.begin(), last.end(), last.begin(), ::toupper);
	transform(loc.begin(), loc.end(), loc.begin(), ::toupper);
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).isId(id))
		{
			//Erase this person and add it again with new information.
			personDb.erase(personDb.begin() + i);
			this->add(id, first, last, gender, loc);
			return true;
		}
	}
	return false;
}
//Remove a person from database accroding to id.
bool PersonDb::remove(string id, string* firstName, string* lastName)
{
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).isId(id))
		{
			*firstName = personDb.at(i).getFirstName();
			*lastName = personDb.at(i).getLastName();
			personDb.erase(personDb.begin() + i);
			return true;
		}
	}
	return false;
}
//Find a perso from database according to name.
void PersonDb::find(string first, string last, vector<string>* result)
{
	transform(first.begin(), first.end(), first.begin(), ::toupper);
	transform(last.begin(), last.end(), last.begin(), ::toupper);
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).isName(first, last))
		{
			(*result).push_back(personDb.at(i).toString());
		}
	}
	//If not found, return error message.
	if((*result).empty())
	{
		string errMsg = "No person called " + first + " " + last + " is found!"; 
		(*result).push_back(errMsg);
	}
	return;
}
//List with start and end.
void PersonDb::list(string start, string end, vector<string>* result)
{
	//Handle the case when start and end length is greater than 1.
	if(start.length() != 1 || end.length() != 1)
	{
		(*result).push_back("argument length is not 1!");
		return;
	}
	transform(start.begin(), start.end(), start.begin(), ::toupper);
	transform(end.begin(), end.end(), end.begin(), ::toupper);
	//Handle the case when end is greater than start.
	if (end.c_str()[0] < start.c_str()[0])
	{
		(*result).push_back("finish less then start");
		return;
	}
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).nameInRange(start, end) && !personDb.at(i).isUnidentified())
		{
			(*result).push_back(personDb.at(i).toString());
		}
	}
	//If no record found, send back error message.
	if ((*result).empty())
	{
		(*result).push_back("No entities satisfy requirements");
		return;
	}
	return;
}
//List with start.
void PersonDb::list(string start, vector<string>* result)
{
	if(start.length() != 1)
	{
		(*result).push_back("argument length is not 1!");
		return;
	}
	transform(start.begin(), start.end(), start.begin(), ::toupper);
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).nameInRange(start) && !personDb.at(i).isUnidentified())
		{
			(*result).push_back(personDb.at(i).toString());
		}
	}
	//If no record found, send back error message.
	if ((*result).empty())
	{
		(*result).push_back("No entities satisify requirements");
		return;
	}
	return;
}
//List that return all the records.
void PersonDb::list(vector<string>* result)
{
	for (int i = 0; i < personDb.size(); i++)
	{
		(*result).push_back(personDb.at(i).toString());
	}
	if((*result).empty())
	{
		(*result).push_back("Person Database is empty!");
		return;
	}
	return;
}
//Locate the persons with loc.
void PersonDb::locate(string loc, vector<string>* result)
{
	for (int i = 0; i < personDb.size(); i++)
	{
		if (personDb.at(i).inLoc(loc))
		{
			(*result).push_back(personDb.at(i).toString());
		}
	}
	//If no record found, send back error message.
	if((*result).size() == 0)
	{
		(*result).push_back("No match location found!");
		return;
	}
	return;
}
//Get the length of database.
int PersonDb::getLength()
{
	return personDb.size();
}
//Get the database.
vector<Person> PersonDb::getDatabase()
{
	return personDb;
}
//Print the record in the given pos.
void PersonDb::print(int pos)
{
	if (pos < 0 || pos > this->getLength())
	{
		cout << "Wrong input position" << endl;
		return;
	}
	cout << personDb.at(pos);
}

