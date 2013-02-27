#include <cstring>
#include <vector>

using namespace std;

/*
 * Main functions that can be accessed from outside. These are the query
 * functions to be used
 */
string addEvent ( string fileName, vector<std::string> operation );
string removeEvent ( string fileName, vector<std::string> operation );
string getEvent ( string fileName, vector<std::string> operation );
string updateEvent ( string fileName, vector<std::string> operation );
string execQuery( string query );
string findNumLines ( string fileName );
string getLine ( string fileName, vector<std::string> operation );

