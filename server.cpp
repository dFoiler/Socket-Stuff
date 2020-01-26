#include <iostream>
#include <cstdlib>	// std::exit
#include <string.h>	// memset, std::string, etc.
#include <unistd.h>	// close
#include <errno.h>	// errno
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>	// IP to string stuff

#define BACKLOG 20
#define MAXDATASIZE 256

int setup_server(char* port)
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
	status = getaddrinfo(NULL, port, &hints, &res);
	if(status)
	{
		std::cout << "I died setting up a socket." << std::endl;
		std::exit(1);
	}
	// Feed address information into that socket
	int sockDesc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockDesc == -1) // Uh oh
	{
		std::cout << "I died making a socket." << std::endl;
		std::exit(1);
	}
	// Now bind
	if(bind(sockDesc, res->ai_addr, res->ai_addrlen) == -1)
	{
		std::cout << "I died binding." << std::endl;
		std::exit(1);
	}
	// Be a good programmer and free up stuff
	freeaddrinfo(res);
	return sockDesc;
}

// Return the sockaddr with IPv4 and IP6 support
void* get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family = AF_INET)
		return &(( (struct sockaddr_in*)sa ) -> sin_addr);
	return &(( (struct sockaddr_in6*)sa ) -> sin6_addr);
}

// Return the string associated with an address
std::string addr_to_str(struct sockaddr_storage *client_addr)
{
	// Use C commands to push this into a char array
	char addr_chars[INET6_ADDRSTRLEN];
	inet_ntop(client_addr->ss_family,
		get_in_addr((struct sockaddr*)client_addr),
		addr_chars, sizeof(addr_chars));
	// Manually convert to a string
	return std::string(addr_chars);
}

void run_client(int client_desc, struct sockaddr_storage *client_addr)
{
	// Let's sit around reveiving for a while
	char buf[MAXDATASIZE];
	while(1)
	{
		int bytes_recv = recv(client_desc, buf, MAXDATASIZE, 0);
		// Break if the connection is dead
		if(bytes_recv == 0) break;
		// Now output
		std::cout << addr_to_str(client_addr) << " says: " << buf << std::endl;
	}
	// We're done here
	close(client_desc);
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		std::cout << "Usage: [port]" << std::endl;
		return 1;
	}
	int sock_desc = setup_server(argv[1]);
	// Listen
	if(listen(sock_desc, BACKLOG) == -1)
	{
		std::cout << "I died listening." << std::endl;
		return 1;
	}
	std::cout << "Listening . . . " << std::endl;
	
	// And now we wait for someone to make us less lonesly inside
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	while(1)
	{
		// We got a biter
		int client_desc = accept(sock_desc, (struct sockaddr*)&client_addr, &addr_size);
		if(client_desc == -1)
		{
			std::cout << "I died accepting." << std::endl;
			continue;
		}
		std::string addr_str = addr_to_str(&client_addr);
		std::cout << "Connected to " << addr_str << std::endl;
		// Now open up a new thread
		if(!fork())
		{
			close(sock_desc); // The new thread shouldn't hog the port
			run_client(client_desc, &client_addr);
			std::cout << "Disconnected from " << addr_str << std::endl;
			return 0;
		}
		// And done here
		close(client_desc);
	}
	close(sock_desc);
	return 0;
}
