#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "3730"
#define MAXDATASIZE 256

int main()
{
	// Your usual setup
	struct addrinfo hints, *res; // Stuff we need
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP for life
	hints.ai_flags = AI_PASSIVE; // I'll be connecting on my machine
	
	// So what's the diagnosis?
	if(getaddrinfo(NULL, PORT, &hints, &res))
	{
		std::cout << "I died setting up a socket." << std::endl;
		return 1;
	}
	
	// Now, make the socket
	int sockDesc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	// And connect!
	if(connect(sockDesc, res->ai_addr, res->ai_addrlen) == -1)
	{
		std::cout << "I died connecting." << std::endl;
		return 1;
	}
	std::cout << "Connected!" << std::endl;
	// And let's send something, for kicks
	std::cout << "Tell me what to send; enter [q] to quit." << std::endl;
	std::string message;
	while(1)
	{
		getline(std::cin, message);
		char to_send[MAXDATASIZE];
		int i;
		for(i = 0; i < message.length() && i < MAXDATASIZE-1; ++i)
			to_send[i] = message[i];
		to_send[i] = '\0';
		if(to_send[0]=='[' && to_send[1]=='q' && to_send[2]==']' && i==3)
			break;
		int bytes_sent = send(sockDesc, to_send, MAXDATASIZE, 0);
	}
	// And now we're done.
	close(sockDesc);
	freeaddrinfo(res);
	return 0;
}
