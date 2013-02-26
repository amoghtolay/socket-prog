all:
	g++ -Wall serverSingle.cpp fileReadWrite.cpp -o serverSingle
	g++ -Wall serverMultiThread.cpp fileReadWrite.cpp -o serverMultiThread
	g++ -Wall client.cpp fileReadWrite.cpp -o client
	g++ -Wall serverAllModes.cpp fileReadWrite.cpp -o serverAllModes 
# Used for cleaning all user files - DONOT use in between testing
clean:
	rm ./calendar/*
