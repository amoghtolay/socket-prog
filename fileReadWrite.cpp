#include <iostream>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

#define FOLDER_NAME "./calendar/"

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
 * 2. Finish all essential queries and imcorporate to file
 * 3. Check extensively and then incorporate this code to client code
 * 	  which calls the server automatically
 * 4. Include error handling like putting char instead of int in date
 * 5. Change update func to avoid duplicate updates
 * 6. Remove all expired events from all users function - IMP
 ***********************************************************************
 */
// All helper functions here
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
string dateToEpoch ( string date, string hrmm )
{
	/*
	 * Convert time from the format given to seconds since epoch.
	 * Then return string of this value.
	 * Assume format as MMDDYY
	 */
	string mm = date.substr(0,2);
	string dd = date.substr(2,2);
	string yy = date.substr(4,2);
	
	string hr = hrmm.substr(0,2);
	string min = hrmm.substr(2,2);
	
	struct tm t = {0};  // Initalize to all 0's
	t.tm_year = atoi(yy.c_str())+100; // This is year-1900, so 112 = 2012
	t.tm_mon = atoi(mm.c_str())-1;
	t.tm_mday = atoi(dd.c_str());
	t.tm_hour = atoi(hr.c_str());
	t.tm_min = atoi(min.c_str());
	
	time_t timeSinceEpoch = mktime(&t);
	time_t curTime = time (NULL);
	string error = "ERROR";
	if ( timeSinceEpoch < curTime )
		return error;
	stringstream ss;
	ss << timeSinceEpoch;
	string ts = ss.str();
	return ts;
} 
bool isFileExists(string fileName)
{
    if (FILE *file = fopen( strdup(fileName.c_str()), "r"))
    {
        fclose(file);
        return true;
    }
    return false;
}
bool isClash ( string startTimestamp, string endTimestamp, string fileName )
{
	ifstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	if (!fpCal.is_open()){
		cout<<"ERROR: Could not open the user's file\n";
		return (1);
	}
	string oneLine;
	while ( getline (fpCal,oneLine) ){
		if ( oneLine.find(" ") == 0 )
			continue;
		vector<std::string> splitLine;
		splitLine = parse( oneLine );
		if ( splitLine[0] >= startTimestamp && splitLine[0] < endTimestamp )
			return true;
		if ( splitLine[1] > startTimestamp && splitLine[1] <= endTimestamp )
			return true;
	}
	return false;
}
int removeEmptyFile ( string fileName )
{
	if ( !isFileExists( fileName ) )
		return 0;
	ifstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	if (!fpCal.is_open()){
		cout<<"ERROR: Could not open the user's file\n";
		return (1);
	}
	string oneLine;
	bool empty = true;
	while ( getline (fpCal,oneLine) ){
		if ( oneLine.find(" ") != 0 )
			empty = false;
	}
	if (empty){
		string cmd = "rm " + fileName;
		system(cmd.c_str());
	}
	return 0;
}
int removeAllExpired ( )
{
	DIR *dirp = opendir(FOLDER_NAME);
	struct dirent *dp;
	bool isRemoved = false;
	while ( (dp = readdir(dirp) ) != NULL ){
		if ( !strcmp(".", dp->d_name) || !strcmp("..", dp->d_name) )
			continue;
		fstream fpCal;
		cout<<dp->d_name;
		stringstream ss;
		ss << dp->d_name;
		string fileName = FOLDER_NAME + ss.str();
		
		fpCal.open( strdup(fileName.c_str()), fstream::in | fstream::out );
		if (!fpCal.is_open()){
			cout<<"ERROR: Could not open the user's file HERE\n";
			return (1);
		}
		stringstream st;
		time_t timeNow = time (NULL);
		st << timeNow;
		string curTime = st.str();
		string oneLine;
		
		int pos = fpCal.tellg();
		while ( getline (fpCal,oneLine) ){
			cout<<pos<<"\n";
			if ( oneLine.find(" ") == 0 ){
				pos = fpCal.tellg();
				continue;
			}
			vector<std::string> splitLine;
			splitLine = parse( oneLine );
			if ( ( splitLine[1] >= curTime ) ){
				pos = fpCal.tellg();
				continue;
			}
			else if ( splitLine[1] < curTime ){
				oneLine.replace ( oneLine.begin(), oneLine.end(), oneLine.length(), ' ' );
				fpCal.seekg(pos);
				fpCal<<oneLine;
				isRemoved = true;
				cout<<"REMOVING\n";
			}
		}
	}
	
	(void)closedir(dirp);
	return (!isRemoved);
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
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		cout<<"Could not delete empty file of user\n";
		return 16;
	}
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()), ios::app );
	if (!fpCal.is_open()){
		cout<<"ERROR: Could not open the user's file\n";
		return (1);
	}
	/*
	 * operation[0] contains add. operation[1] contains date.
	 * operation[2] contains start time, operation[3] contains end time
	 * operation[4] contains event name
	 */
	if (operation.size() != 5 ){
		cout<<"Number of arguments is wrong. Usage is username add  date time1 time2 name\n";
		return (10);
	}
	if ( !(dateToEpoch( operation[1], operation[2] ).compare("ERROR") &&
		dateToEpoch( operation[1], operation[3] ).compare("ERROR")) ){
			cout<<"ERROR: The time and date should be in the future and of valid format\n";
			return (2);
	}
	string startTimestamp = dateToEpoch( operation[1], operation[2] );
	string endTimestamp = dateToEpoch( operation[1], operation[3] );
	string entryToWrite = startTimestamp + " " + endTimestamp + " " + operation[4] + "\n";
	if ( atoi(startTimestamp.c_str()) > atoi(endTimestamp.c_str()) ){
		cout<<"ERROR: End time of event should be same or after start time\n";
		return (3);
	}
	/*
	 * Now check for clashes with other events
	 */
	bool isClashing = isClash ( startTimestamp, endTimestamp, fileName );
	if ( isClashing ){
		cout<<"This event clashes with another event.\n";
		return (5);
	}
	fpCal<<entryToWrite;
	fpCal.close();
	return (0);
}
int removeEvent ( string fileName, vector<std::string> operation )
{
	if ( !isFileExists(fileName) ){
		cout<<"ERROR: User's file doesn't exist.";
		return (4);
	}
	fstream fpCal;
	fpCal.open( strdup(fileName.c_str()), fstream::in | fstream::out );
	string startTimestamp = dateToEpoch ( operation[1], operation[2] );
	if (operation.size() < 3){
		cout<<"Number of arguments is wrong. Usage is username remove  date time1\n";
		return (10);
	}
	string oneLine;
	int pos = fpCal.tellg();
	bool isRemoved = false;
	while ( getline (fpCal,oneLine) ){
		if ( oneLine.find(" ") == 0 ){
			pos = fpCal.tellg();
			continue;
		}
		vector<std::string> splitLine;
		splitLine = parse( oneLine );
		if ( ( splitLine[0].compare(startTimestamp) ) ){
			pos = fpCal.tellg();
			continue;
		}
		else if ( !( splitLine[0].compare(startTimestamp) ) ){
			oneLine.replace ( oneLine.begin(), oneLine.end(), oneLine.length(), ' ' );
			fpCal.seekg(pos);
			fpCal<<oneLine;
			isRemoved = true;
			break;
		}
	}
	fpCal.close();
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		cout<<"Could not delete empty file of user\n";
		return 16;
	}
	if (isRemoved){
		cout<<"Event removed.\n";
		return (0);
	}
	else
		return (6);
}
int getEvent ( string fileName, vector<std::string> operation )
{
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		cout<<"Could not delete empty file of user\n";
		return 16;
	}
	if ( !isFileExists(fileName) ){
		cout<<"ERROR: User's file doesn't exist.";
		return (4);
	}
	ifstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	
	fpCal.close();
	return (0);
}
int updateEvent ( string fileName, vector<std::string> operation )
{
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		cout<<"Could not delete empty file of user\n";
		return 16;
	}
	if ( !isFileExists(fileName) ){
		cout<<"ERROR: User's file doesn't exist.";
		return (4);
	}
	fstream fpCal;
	fpCal.open( strdup(fileName.c_str()), fstream::in | fstream::out );
	if (operation.size() != 5 ){
		cout<<"Number of arguments is wrong. Usage is username update  date time1 time2 name\n";
		return (10);
	}
	int resultRemove = removeEvent( fileName, operation );
	cout<<resultRemove;
	if (!resultRemove){
		int resultAdd = addEvent ( fileName, operation );
		if (resultAdd){
			cout<<"Could not update file as adding event failed\n";
			return (8);
		}
	}
	else {
		cout<<"Entry does not exist, so cannot be updated\n";
		return (7);
	}
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
	string fileName = FOLDER_NAME + ( "calendar_" + username );
	
	removeAllExpired();
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
