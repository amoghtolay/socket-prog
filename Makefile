all: serverSingle serverMultiThread client

serverSingle:
	g++ -Wall serverSingle.cpp fileReadWrite.cpp -o serverSingle
serverMultiThread:
	g++ -Wall serverMultiThread.cpp fileReadWrite.cpp -o serverMultiThread
client:
	g++ -Wall client.cpp fileReadWrite.cpp -o client

# Used for cleaning all user files - DONOT use in between testing
clean:
	rm ./calendar/*
