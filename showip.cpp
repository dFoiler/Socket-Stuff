#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// This is a showcase of getaddrinfo and its documentation:
//  getaddrinfo(const char *node,     		// connection
//		const char *service,  		// how are we connecting? (http/s/port?)
//		const struct addrinfo *hints,	// specify my conditions (IPv4/6/etc.)
//		struct addrinfo **res);		// a linked list of addrinfos

int main(int argc, char* argv[])
{
	// Declare the structs responsible for storing our address
	struct addrinfo hints, *res; // p is placeholder
	
	// Manually set the hints to 0 so that we don't worry about it
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Let's not care IPv4/6
	hints.ai_socktype = SOCK_STREAM; // TCP for life
	
	// Now for running the connection!
	int status; // How'd I do chief?
	// Alright, now we make a fake connection to the service
	if(argc > 1)
	{
		std::cout << "Showing IP of " << argv[1] << " . . . " << std::endl;
		status = getaddrinfo(argv[1], NULL, &hints, &res);
	}
	else
	{
		std::cout << "Showing my IP . . . " << std::endl;
		hints.ai_flags = AI_PASSIVE; // Use my IP
		// Pretend we're going to bind to a port, say 1025
		status = getaddrinfo(NULL, "1025", &hints, &res);
	}
	if(status)
	{
		// Dangit, something went wrong.
		std::cout << "I exploded. Here are my dying words: " << status << std::endl;
		return 1;
	}
	
	// Ok, now we've got everything we want in res. Lezgo.
	for(struct addrinfo *p = res; p; p = p->ai_next)
	{
		// It's time to get some addresses
		void* addr; std::string ipver;
		if(p->ai_family == AF_INET) // IPv4 check
		{
			// Get the address
			struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
			addr = &(ipv4->sin_addr);
			// Set the version
			ipver = "IPv4";
		}
		else // IPv6 default
		{
			// Get the address
			struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			// Set the version
			ipver = "IPv6";
		}
		
		// Now use a sweet swwet convenience command to get the job done
		char ipstr[INET6_ADDRSTRLEN]; // Gimme some space
		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
		std::cout << ipver  << " : " << ipstr << std::endl;
	}
	
	// Those darn memory leaks ruin your day
	freeaddrinfo(res);
	return 0;
}
