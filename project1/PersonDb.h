/*
Author: Mengwen Li (mli2)
*/
#include <vector>
#include "Person.h"

#ifndef PERSONDB_H_
#define PERSONDB_H_

using namespace std;

class PersonDb {
public:
	PersonDb();
	~PersonDb();
	//Sort the person database according to last name.
	void dbSort();
	//Add person to database.
	bool add(string id, string first, string last, char gender, string loc);
	//Update a person's information.
	bool update(string id, string first, string last, char gender, string loc);
	//Remove a person from database accroding to id.
	bool remove(string id, string* firstName, string* lastName);
	//Find a perso from database according to name.
	void find(string first, string last, vector<string>* result);
	//List with start and end.
	void list(string start, string finish, vector<string>* result);
	//List with start.
	void list(string start, vector<string>* result);
	//List that return all the records.
	void list(vector<string>* result);
	//Locate the persons with loc.
	void locate(string loc, vector<string>* result);
	//Get the length of database.
	int getLength();
	//Get the database.
	vector<Person> getDatabase();
	//Print the record in the given pos.
	void print(int pos);

private:
	vector<Person> personDb;
};

#endif

