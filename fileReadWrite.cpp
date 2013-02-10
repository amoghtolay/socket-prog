#include <iostream>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <vector>

using namespace std;
/*
 * Algorithm:
 * 1. Take username from username command
 * 2. If calendar_<username> exists, then open the file
 * 3. If doesn't exist, and command is add, then create file and add
 * 	  the event appropriately.
 * 	  Else, just give error that no events for the user
 * 4. Otherwise open file, and depending on the command, do appropriate
 * 	  stuff
 * 5. Do basic error detection, and clash handling
 * 
 * FILE FORMATTING:
 */
/***********************************************************************
 * TODO:
 * 1. Finalise format of file
 * 2. Finish all essential queries and imcorporate to file
 * 3. Check extensively and then incorporate this code to client code
 * 	  which calls the server automatically
 ***********************************************************************
 */

std::vector<std::string> parse( string query )
{
	vector<std::string> entities;
	istringstream iss(query);
    while (iss)
    {
		string sub;
		iss >> sub;
        entities.push_back( sub );
    }
    entities.pop_back();
    return entities;
}
/*
 * The following 4 functions, add, remove, update or get data from the
 * calendar. These are the only functions which ever deal with the file
 * as such. No other function ever opens the file, so any synch
 * constructs have to be implemented here only.
 * All formats are same. First arg is filename and second is the
 * remaining query after filename.
 */

int addEvent ( string fileName, vector<std::string> operation )
{
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );

	fpCal.close();
	return (0);
}
int removeEvent ( string fileName, vector<std::string> operation )
{
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );

	fpCal.close();
	return (0);
}
int getEvent ( string fileName, vector<std::string> operation )
{
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );

	fpCal.close();
	return (0);
}
int updateEvent ( string fileName, vector<std::string> operation )
{
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	
	fpCal.close();
	return (0);
}

/*
 * This is the function that takes in the entire query and splits the
 * query and appropriately calls required operator function
 */
void execQuery( string query )
{
	vector<std::string> splitQuery;
	splitQuery = parse( query );
	string username = splitQuery.front();
	// Remove the username from the query, now just the operator 
	// and its operands remain
	splitQuery.erase (splitQuery.begin());
	string fileName = "calendar_"+username;
	
	string operation = splitQuery.front();
	int result = -1;
	if ( !operation.compare("add") )
		result = addEvent( fileName, splitQuery );
	if ( !operation.compare("remove") )
		result = removeEvent( fileName, splitQuery );
	if ( !operation.compare("update") )
		result = updateEvent( fileName, splitQuery );
	if ( !operation.compare("get"))
		result = getEvent( fileName, splitQuery );
	
	if ( result == -1 ){
		cout<<"Failed to modify your calendar: Invalid operation specified.\nOnly 'add', 'remove', 'update' and 'get' are supported";
		result = 1;
	}
	if (!result)
		cout<<"Operation completed successfully\n";
	else
		cout<<"\nOperation FAILED. Recheck query and try again.\n";
}

int main()
{
	string query;
	getline( cin, query );
	execQuery( query );
	return (EXIT_SUCCESS);
}
