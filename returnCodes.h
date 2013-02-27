/*
 * Return string macros are defined in this file. 
 * Try and reuse all macros as much as possible.
 */

// Macros for function addEvent()

#define emptyDelFail "Could not delete empty file of user\n"
#define fopenError "ERROR: Could not open the user's file\n"
#define wrongUsageAdd "Number of arguments is wrong. Usage is username add  date time1 time2 name\n"
#define dateErrorPast "ERROR: The time and date should be in the future and of valid format\n"
#define dateErrorDuration "ERROR: End time of event should be same or after start time\n"
#define clashError "This event clashes with another event.\n"
#define addSuccess "The event was successfully added. Query successful.\n"

#define noExistError "ERROR: User's file doesn't exist."
#define wrongUsageRemove "Number of arguments is wrong. Usage is username remove  date time1\n"
#define removeSuccess "Event successfully removed.\n"
#define removeFail "Event could not be removed. Please check query and try again.\n"

#define wrongUsageUpdate "Number of arguments is wrong. Usage is username update  date time1 time2 name\n"
#define updateSuccess "This event successfuly updated\n"
#define updateFailAdd "Could not update event as adding this event failed\n"
#define updateFailNoExists "Entry does not exist, so cannot be updated\n"
#define restoredEvent "Could not add event because of the above reason.\nThus, update failed and now restoring original event back\n"
#define restoredEventFail "Could not add event because of the above reason.\n Could not restore event due to some reason\n"
#define exactMatchingUpdate "This exact event already exists, so no update done.\n"

#define queryInvalid "Failed to modify your calendar: Invalid operation specified.\nOnly 'add', 'remove', 'update' , 'get' and 'getall' are supported\n"
#define queryPass "Your query was successfully completed.\n"
#define queryFail "Your operation could not be completed because of the error described above ^.\n Please check and try again.\n"

#define wrongUsageGet "ERROR: Invalid query. Usage is either of the two forms:\n1. username get eventDate\n2. username get eventDate eventStartTime\n"
#define getSuccess "The event(s) is(are) as shown above\n"
#define getFail "Your request to get the events has failed\n"
#define getNoEntry "There is no entry corresponding to this date and time\n"

#define totalLines " number of events are present for this user\n"
#define getLineSuccess "Event Number "
#define getLineWrongLine "The client sent a request for getting an event that doesn't exist\n"
#define wrongUsageGetLine "The client sent a wrong format of request to get the nth line.\n"
#define wrongUsageGetAll "Wrong usage of getall. Usage is username getall\n"
#define restoreSuccess "Restored original event\n"

/*
 * Defining folder name to store all user files
 */
#define FOLDER_NAME "./calendar/"
