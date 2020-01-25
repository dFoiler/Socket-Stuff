#include <iostream>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3730"
#define BACKLOG 20

int main()
{
	// Here are the structs that matter
	struct addrinfo hints, *res;
	// Load in the information
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 and IPv6 are fine
	hints.ai_socktype = SOCK_STREAM; // TCP for life
	hints.ai_flags = AI_PASSIVE; // Use my IP please
	
	int status;
	// Now we get the information
	status = getaddrinfo(NULL, PORT, &hints, &res);
	if(status)
	{
		std::cout << "I exploded. Here are my dying words: " << status << std::endl;
		return 1;
	}
	// Feed address information into that socket
	int sockDesc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockDesc == -1) // Uh oh
	{
		std::cout << "I exploded. Here are my dying words:" << std::endl;
	}
	// Now bind
	status = bind(sockDesc, res->ai_addr, res->ai_addrlen);
	if(status == -1)
	{
		std::cout << "I died binding." << std::endl;
		return 1;
	}
	// Listen
	status = listen(sockDesc, BACKLOG);
	if(status == -1)
	{
		std::cout << "I died listening." << std::endl;
		return 1;
	}
	std::cout << "Listening . . . " << std::endl;
	
	// And now we wait for someone to make us less lonesly inside
	while(1)
	{
		struct sockaddr_storage client_addr;
		socklen_t addr_size = sizeof(client_addr);
		int newDesc = accept(sockDesc, (struct sockaddr*)&client_addr, &addr_size);
		if(newDesc == -1)
		{
			std::cout << "I died accepting." << std::endl;
			return 1;
		}
		std::cout << "Connected!" << std::endl;
		char* buf[256];
		int bytes_recv = recv(newDesc, buf, 255, 0);
		std::cout << "Received:" << buf << std::endl;
		// And done here
		close(newDesc);
	}
	// Be a good programmer and return safely
	freeaddrinfo(res);
	return 0;
}
