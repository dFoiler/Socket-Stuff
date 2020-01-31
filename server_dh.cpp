#include <iostream>
#include <cstdlib>	// std::exit
#include <string.h>	// memset, std::string, etc.
#include <unistd.h>	// close
#include <errno.h>	// errno
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>	// IP to string stuff

#include "crypto.cpp"	// My crypto stuff

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
		std::cout << "socket setup error: " << strerror(errno) << std::endl;
		std::exit(1);
	}
	// Feed address information into that socket
	int sockDesc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockDesc == -1) // Uh oh
	{
		std::cout << "socket make error: " << strerror(errno) << std::endl;
		std::exit(1);
	}
	// Now bind
	if(bind(sockDesc, res->ai_addr, res->ai_addrlen) == -1)
	{
		std::cout << "bind error: " << strerror(errno) << std::endl;
		close(sockDesc); // Won't be needing that anymore
		std::exit(1);
	}
	// Be a good programmer and free up stuff
	freeaddrinfo(res);
	return sockDesc;
}

// Return the sockaddr with IPv4 and IP6 support
void* get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
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

// Runs DH key exchange with a client; assumes client behaves
long long run_dh(int client_desc, long long lower=10000000)
{
	long long p = lower;
	// Look for safe primes until we hit a 7 mod 8 one
	while((p = gen_safe_prime(p+1)) % 8 != 7);
	long long g = 2;
	// Now transmit both the prime and the generator across
	send(client_desc, &p, sizeof(&p), 0);
	send(client_desc, &g, sizeof(&g), 0);
	std::cout << "Sent prime (" << p << ") and generator (" << g << ")." << std::endl;
	// Generate what to send next
	long long a = (long long)(get_rand() % (p-1));
	long long ga = pow_mod(g, a, p);
	// Send ga and receive gb
	send(client_desc, &ga, sizeof(ga), 0);
	long long gb;
	recv(client_desc, &gb, sizeof(gb), 0);
	std::cout << "Sent g^a (" << ga << ") and received g^b (" << gb << ")." << std::endl;
	// Return g^(ab)
	return pow_mod(gb, a, p);
}

void run_client(int client_desc, struct sockaddr_storage *client_addr)
{
	long long secret = run_dh(client_desc, get_rand() % (1 << 30));
	std::cout << addr_to_str(client_addr) << "'s secret: " << secret << std::endl;
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
		std::exit(1);
	}
	int sock_desc = setup_server(argv[1]);
	// Listen
	if(listen(sock_desc, BACKLOG) == -1)
	{
		std::cout << "listen error: " << strerror(errno) << std::endl;
		std::exit(1);
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
			std::cout << "accept error: " << strerror(errno) << std::endl;
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
