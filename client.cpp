#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "3730"

int main()
{
	// Your usual setup
	struct addrinfo hints, *res; // Stuff we need
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP for life
	hints.ai_flags = AI_PASSIVE; // I'll be connecting on my machine
	
	// So what's the diagnosis?
	int status;
	status = getaddrinfo(NULL, PORT, &hints, &res);
	if(status)
	{
		std::cout << "I died." << std::endl;
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
	while(1)
	{
		std::cout << "Tell me what to send." << std::endl;
		std::string message;
		std::getline(std::cin, message);
		char to_send[256];
		for(int i = 0; i < message.length(); ++i)
			to_send[i] = message[i];
		int bytes_sent = send(sockDesc, to_send, sizeof(to_send), 0);
		std::cout << "SENT: " << message << std::endl;
	}
	// And now we're done.
	close(sockDesc);
	freeaddrinfo(res);
	return 0;
}
