/***********************************************************************
 * Author: Amogh Tolay, Networks assignment 2, Sem 6
 * This is a basic calendar server implementation.
 * The server runs in 3 modes:
 * 1. Iterative (only one client supported)
 * 2. Multi-threaded (multiple clients at once supported)
 * 3. Selection of client based on select()
 * This file is used for file handling on the server side. All users
 * data (all events) are stored in files and this is the code that
 * handles all types of file handling, including adding, removing,
 * updating and getting events after parsing the query received from
 * the client
 **********************************************************************/

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
#include "returnCodes.h"

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
bool isDateTimeValid ( string date, string hrmm )
{
	/*
	 * Checking for leap year and also for 30 or 31 days in a month
	 * and time hrs and mins constraint
	 */
	string mm = date.substr(0,2);
	string dd = date.substr(2,2);
	string yy = date.substr(4,2);
	
	string hr = hrmm.substr(0,2);
	string min = hrmm.substr(2,2);
	
	int yyInt = atoi(yy.c_str());
	int mmInt = atoi(mm.c_str());
	int ddInt = atoi(dd.c_str());
	int hrInt = atoi(hr.c_str());
	int minInt = atoi(min.c_str());
	
	if ( hrInt < 0 || minInt < 0 || yyInt <0 || mmInt < 0 || ddInt < 0 )
		return false;
	// Check leap year
	if ( yyInt % 4 == 0 && mmInt == 2 && ddInt <= 29 && hrInt <= 23 && minInt < 60 )
		return true;
	if ( ( mmInt == 1 || mmInt == 3 || mmInt == 5 || mmInt == 7 || mmInt == 8 || mmInt == 10 || mmInt == 12 ) && ddInt <= 31 && hrInt <= 23 && minInt < 60 )
		return true;
	if ( ( mmInt == 4 || mmInt == 6 || mmInt == 9 || mmInt == 11 ) && ddInt <= 30 && hrInt <= 23 && minInt < 60 )
		return true;
	if ( mmInt == 2 && ddInt <= 28 && hrInt <= 23 && minInt < 60 )
		return true;
	return false;
}
string dateToEpoch ( string date, string hrmm )
{
	/*
	 * Convert time from the format given to seconds since epoch.
	 * Then return string of this value.
	 * Assume format as MMDDYY
	 */
	string error;
	bool checkDateRet = isDateTimeValid ( date, hrmm );
	if ( !checkDateRet ){
		error = dateErrorPast;
		return error;
	}
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
	
	if ( timeSinceEpoch < curTime ){
		error = dateErrorPast;
		return error;
	}
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
		//cout<<"ERROR: Could not open the user's file\n";
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
		if ( splitLine[0] < startTimestamp && splitLine[1] >= endTimestamp )
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
		//cout<<"ERROR: Could not open the user's file\n";
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
		stringstream ss;
		ss << dp->d_name;
		string fileName = FOLDER_NAME + ss.str();
		
		fpCal.open( strdup(fileName.c_str()), fstream::in | fstream::out );
		if (!fpCal.is_open()){
			//cout<<"ERROR: Could not open the user's file HERE\n";
			return (1);
		}
		stringstream st;
		time_t timeNow = time (NULL);
		st << timeNow;
		string curTime = st.str();
		string oneLine;
		
		int pos = fpCal.tellg();
		while ( getline (fpCal,oneLine) ){
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
string epochToDate ( string epochStr )
{
	time_t epochTime = atoi(epochStr.c_str());
	char buffer [80];
	strftime(buffer, 80, "%Y-%m-%d %H:%M", localtime(&epochTime));
	
	stringstream ss;
	ss << buffer;
	string returnDate = ss.str();

	return returnDate;
} 
string addLine ( string fileName, string eventObtained )
{
	string returnString = "";
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()), ios::app );
	if (!fpCal.is_open()){
		returnString = returnString + fopenError;
		return returnString;
	}
	fpCal<<eventObtained;
	fpCal.close();
	return restoreSuccess;
}
/*
 * The following 4 functions, add, remove, update or get data from the
 * calendar. These are the only functions which ever deal with the file
 * as such. No other function ever opens the file, so any synch
 * constructs have to be implemented here only.
 * All formats are same. First arg is filename and second is the
 * remaining query after filename.
 */

string addEvent ( string fileName, vector<std::string> operation )
{
	string returnString = "";
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		returnString = returnString + emptyDelFail;
		return returnString;
	}
	ofstream fpCal;
	fpCal.open( strdup(fileName.c_str()), ios::app );
	if (!fpCal.is_open()){
		returnString = returnString + fopenError;
		return returnString;
	}
	/*
	 * operation[0] contains add. operation[1] contains date.
	 * operation[2] contains start time, operation[3] contains end time
	 * operation[4] contains event name
	 */
	if (operation.size() != 5 ){
		returnString = returnString + wrongUsageAdd;
		return returnString;
	}
	if ( !(dateToEpoch( operation[1], operation[2] ).compare(dateErrorPast) &&
		dateToEpoch( operation[1], operation[3] ).compare(dateErrorPast)) ){
			returnString = returnString + dateErrorPast;
			return (returnString);
	}
	string startTimestamp = dateToEpoch( operation[1], operation[2] );
	string endTimestamp = dateToEpoch( operation[1], operation[3] );
	string entryToWrite = startTimestamp + " " + endTimestamp + " " + operation[4] + "\n";
	if ( atoi(startTimestamp.c_str()) > atoi(endTimestamp.c_str()) ){
		returnString = returnString + dateErrorDuration;
		return (returnString);
	}
	/*
	 * Now check for clashes with other events
	 */
	bool isClashing = isClash ( startTimestamp, endTimestamp, fileName );
	if ( isClashing ){
		returnString = returnString + clashError;
		return (returnString);
	}
	fpCal<<entryToWrite;
	fpCal.close();
	returnString = returnString + addSuccess;
	return (returnString);
}
string removeEvent ( string fileName, vector<std::string> operation )
{
	string returnString = "";
	if ( !isFileExists(fileName) ){
		returnString = returnString + noExistError;
		return (returnString);
	}
	fstream fpCal;
	fpCal.open( strdup(fileName.c_str()), fstream::in | fstream::out );
	string startTimestamp = dateToEpoch ( operation[1], operation[2] );
	if (operation.size() < 3){
		returnString = returnString + wrongUsageRemove;
		return (returnString);
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
		returnString = returnString + emptyDelFail;
		return returnString;
	}
	if (isRemoved){
		returnString = returnString + removeSuccess;
		return (returnString);
	}
	else{
		returnString = returnString + removeFail;
		return (returnString);
	}
}
string getEvent ( string fileName, vector<std::string> operation )
{
	string returnString = "";
	bool entryExists = false;
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		returnString = returnString + emptyDelFail;
		return returnString;
	}
	if ( !isFileExists(fileName) ){
		returnString = returnString + noExistError;
		return (returnString);
	}
	ifstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	/*
	 * Now check which of the two formats is the get query - 
	 * 1. username get <eventTime>
	 * 2. username get <eventName>
	 */
	if ( operation.size() == 3 ){
		// Only one event has to be returned
		string startTimestamp = dateToEpoch( operation[1], operation[2] );
		string oneLine;
		while ( getline (fpCal,oneLine) ){
			if ( oneLine.find(" ") == 0 )
				continue;
			vector<std::string> splitLine;
			splitLine = parse( oneLine );
			if ( splitLine[0] == startTimestamp ){
				// Return this entire line after parsing
				returnString = returnString + epochToDate ( splitLine[0] ) + " to " + epochToDate ( splitLine[1] ) + "\t" + splitLine[2] + "\n";
				entryExists = true;
				break;
			}
		}
	}
	if ( operation.size() == 2 ){
		string startTimestamp = dateToEpoch( operation[1], "0000" );
		string endTimestamp = dateToEpoch ( operation[1], "2359" );
		string oneLine;
		while ( getline (fpCal,oneLine) ){
			if ( oneLine.find(" ") == 0 )
				continue;
			vector<std::string> splitLine;
			splitLine = parse( oneLine );
			if ( splitLine[0] >= startTimestamp && splitLine[0] <= endTimestamp ){
				// Return this entire line after parsing
				returnString = returnString + epochToDate ( splitLine[0] ) + " to " + epochToDate ( splitLine[1] ) + "\t" + splitLine[2] + "\n";
				entryExists = true;
			}
		}
	}
	else if ( operation.size() > 4 || operation.size() < 2 ){
		returnString = returnString + wrongUsageGet;
		return returnString;
	}
	fpCal.close();
	if (entryExists)
		returnString = returnString + getSuccess;
	else
		returnString = returnString + getNoEntry;
	return (returnString);
}
string updateEvent ( string fileName, vector<std::string> operation )
{
	string returnString = "";
	int delResult = removeEmptyFile ( fileName );
	if (delResult){
		returnString = returnString + emptyDelFail;
		return returnString;
	}
	if ( !isFileExists(fileName) ){
		returnString = returnString + noExistError;
		return (returnString);
	}
	fstream fpCal;
	fpCal.open( strdup(fileName.c_str()), fstream::in | fstream::out );
	if (operation.size() != 5 ){
		returnString = returnString + wrongUsageUpdate;
		return (returnString);
	}
	/*
	 * Below code is to get event so that if the addition fails after
	 * removing the event, we can add the original event back into the
	 * calendar.
	 */
	string eventToUpdate;
	string startTimestamp = dateToEpoch ( operation[1], operation[2] );
	string endTimestamp = dateToEpoch ( operation[1], operation[3] );
	string updatedEventName = operation[4];
	
	bool isExists = false;
	bool exactMatch = false;
	while ( getline (fpCal,eventToUpdate) ){
		if ( eventToUpdate.find(" ") == 0 )
			continue;
		vector<std::string> splitLine;
		splitLine = parse( eventToUpdate );
		if ( !( splitLine[0].compare(startTimestamp) ) ){
			isExists = true;
			eventToUpdate = eventToUpdate + "\n";
			if ( !(splitLine[1].compare(endTimestamp)) && !(splitLine[2].compare(updatedEventName)) )
				exactMatch = true; 
			break;
		}
	}
	if ( exactMatch ){
		returnString = returnString + exactMatchingUpdate;
		return returnString;
	}
	string resultRemove = removeEvent( fileName, operation );
	if (resultRemove == removeSuccess){
		string resultAdd = addEvent ( fileName, operation );
		if (resultAdd == addSuccess){
			returnString = updateSuccess;
			return (returnString);
		}
		else if ( isExists ){
			string resultRestore = addLine ( fileName, eventToUpdate);
			if ( resultRestore == restoreSuccess )
				returnString = resultAdd + restoredEvent;
			else
				returnString = resultRemove + resultAdd + restoredEventFail;
		}
	}
	else {
		returnString = returnString + updateFailNoExists;
		return (returnString);
	}
	fpCal.close();
	return (returnString);
}
string findNumLines ( string fileName )
{
	string returnString = "";
	ifstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	if (!fpCal.is_open()){
		returnString = returnString + fopenError;
		return returnString;
	}

	/*
	 * Now count the number of lines here
	 */
	int numLines = 0;
	string oneLine;
	while ( getline (fpCal,oneLine) ){
		if ( oneLine.find(" ") == 0 )
			continue;
		numLines++;
	}
	fpCal.close();
	stringstream ss;
	ss<<numLines;
	returnString = ss.str() + totalLines;
	return ( returnString );
}
string getLine ( string fileName, vector<std::string> operation )
{
	string returnString = "";
	if (operation.size() != 2 ){
		returnString = returnString + wrongUsageGetLine;
		return returnString;
	}

	ifstream fpCal;
	fpCal.open( strdup(fileName.c_str()) );
	if (!fpCal.is_open()){
		returnString = returnString + fopenError;
		return returnString;
	}
	int numLines = atoi(findNumLines ( fileName ).c_str());
	int lineNum = atoi(operation[1].c_str());
	if (numLines < lineNum ){
		returnString = returnString + getLineWrongLine;
		return (returnString);
	}
	int i = 1;
	string oneLine;
	while ( i<=lineNum && getline (fpCal,oneLine) ){
		if ( oneLine.find(" ") == 0 )
			continue;
		if ( i == lineNum ){
			vector<std::string> splitLine;
			splitLine = parse( oneLine );
			returnString = returnString + epochToDate ( splitLine[0] ) + " to " + epochToDate ( splitLine[1] ) + "\t" + splitLine[2];
			break;
		}
		i++;
	}
	fpCal.close();
	returnString = getLineSuccess + operation[1] + ":\t " + returnString + "\n";
	return returnString;
}

/*
 * This is the function that takes in the entire query and splits the
 * query and appropriately calls required operator function
 */
string execQuery( string query )
{
	cout<<"Executing "<<query<<"\n";
	vector<std::string> splitQuery;
	splitQuery = parse( query );
	string username = splitQuery.front();
	// Remove the username from the query, now just the operator 
	// and its operands remain
	splitQuery.erase (splitQuery.begin());
	string fileName = FOLDER_NAME + ( "calendar_" + username );
	
	removeAllExpired();
	string operation = splitQuery.front();
	string result = "-1";
	if ( !operation.compare("add") )
		result = addEvent( fileName, splitQuery );
	if ( !operation.compare("remove") )
		result = removeEvent( fileName, splitQuery );
	if ( !operation.compare("update") )
		result = updateEvent( fileName, splitQuery );
	if ( !operation.compare("get"))
		result = getEvent( fileName, splitQuery );
	if ( !operation.compare("getall"))
		result = findNumLines( fileName );
	if ( !operation.compare("getLine") )
		result = getLine ( fileName, splitQuery );
	
	if ( result == "-1" ){
		result = queryInvalid;
		return result;
	}
	if ( string::npos != result.find(addSuccess) || string::npos != result.find(removeSuccess) || string::npos != result.find(updateSuccess) || string::npos != result.find(getSuccess)
							|| string::npos != result.find(totalLines) || string::npos != result.find(getLineSuccess))
		result = result + queryPass;
	else
		result = result + queryFail;
	return (result);
}
