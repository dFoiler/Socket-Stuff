#include <iostream>
#include <cstdlib>	// std::exit
#include <string.h>	// memset, std::string, etc.
#include <unistd.h>	// close
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define DEFAULT_PORT "3730"
#define MAXDATASIZE 256

int setup_client(char* port)
{
	// Your usual setup
	struct addrinfo hints, *res; // Stuff we need
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP for life
	hints.ai_flags = AI_PASSIVE; // I'll be connecting on my machine
	
	// So what's the diagnosis?
	if(getaddrinfo(NULL, port, &hints, &res))
	{
		std::cout << "I died setting up a socket." << std::endl;
		std::exit(1);
	}
	
	// Now, make the socket
	int sock_desc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	// And connect!
	if(connect(sock_desc, res->ai_addr, res->ai_addrlen) == -1)
	{
		std::cout << "I died connecting." << std::endl;
		std::exit(1);
	}
	freeaddrinfo(res);
	return sock_desc;
}

// Runs the client sending program
// Returns true if to continue going
bool run_client(int sock_desc)
{
	std::string message;
	getline(std::cin, message);
	char to_send[MAXDATASIZE];
	int i;
	for(i = 0; i < message.length() && i < MAXDATASIZE-1; ++i)
		to_send[i] = message[i];
	to_send[i] = '\0';
	if(to_send[0]=='[' && to_send[1]=='q' && to_send[2]==']' && i==3)
		return false;
	send(sock_desc, to_send, MAXDATASIZE, 0);
	return true;
}

int main(int argc, char* argv[])
{
	// Set up the socket stuff
	if(argc != 2)
	{
		std::cout << "Usage: [port]" << std::endl;
		return 1;
	}
	int sock_desc = setup_client(argv[1]);
	// And that's it!
	// And let's send something, for kicks
	std::cout << "Tell me what to send; enter [q] to quit." << std::endl;
	bool go;
	do
	{
		go = run_client(sock_desc);
	} while(go);
	// And now we're done.
	close(sock_desc);
	return 0;
}
