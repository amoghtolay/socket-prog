#include <cstring>
#include <vector>

using namespace std;

/*
 * Main functions that can be accessed from outside. These are the query
 * functions to be used
 */
int addEvent ( string fileName, vector<std::string> operation );
int removeEvent ( string fileName, vector<std::string> operation );
int getEvent ( string fileName, vector<std::string> operation );
int updateEvent ( string fileName, vector<std::string> operation );
void execQuery( string query );
