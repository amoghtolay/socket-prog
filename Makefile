all:
	g++ -Wall client.cpp fileReadWrite.cpp -o client
	g++ -Wall serverAllModes.cpp fileReadWrite.cpp -o serverAllModes 

# Used for cleaning all user files - DONOT use in between testing
# Also deletes all executable files
clean:
	rm -f ./calendar/*
	rm -f ./client
	rm -f ./serverSingle serverAllModes serverMultiThread
	rm -f ./fileReadWrite
