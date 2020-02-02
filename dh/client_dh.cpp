#include <iostream>
#include <cstdlib>	// std::exit
#include <string.h>	// memset, std::string, etc.
#include <unistd.h>	// close
#include <errno.h>	// errno
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "crypto.cpp"

#define DEFAULT_PORT "3730"
#define MAXDATASIZE 256

int setup_client(char* ip, char* port)
{
	// Your usual setup
	struct addrinfo hints, *res; // Stuff we need
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP for life
	
	// So what's the diagnosis?
	if(getaddrinfo(ip, port, &hints, &res))
	{
		std::cout << "socket setup error: " << strerror(errno) << std::endl;
		std::exit(1);
	}
	
	// Now, make the socket
	int sock_desc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sock_desc == -1)
	{
		std::cout << "socket make error: " << strerror(errno) << std::endl;
		std::exit(1);
	}
	// And connect!
	if(connect(sock_desc, res->ai_addr, res->ai_addrlen) == -1)
	{
		std::cout << "connect error: " << strerror(errno) << std::endl;
		std::exit(1);
	}
	freeaddrinfo(res);
	return sock_desc;
}

long long run_dh(int server_desc)
{
	// The prime is sent and then the generator
	long long p, g;
	recv(server_desc, &p, sizeof(p), 0);
	recv(server_desc, &g, sizeof(g), 0);
	std::cout << "Received prime (" << p << ") and generator (" << g << ")." << std::endl;
	// Generate what to send next
	long long b = (long long)(get_rand() % (p-1));
	long long gb = pow_mod(g, b, p);
	// Send gb and receive ga
	send(server_desc, &gb, sizeof(gb), 0);
	long long ga;
	recv(server_desc, &ga, sizeof(ga), 0);
	std::cout << "Send g^b (" << gb << ") and recieved g^a (" << ga << ")." << std::endl;
	// Return g^(ab)
	return pow_mod(ga, b, p);
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
	if(argc != 3)
	{
		std::cout << "Usage: [node] [port]" << std::endl;
		return 1;
	}
	int sock_desc = setup_client(argv[1], argv[2]);
	// And that's it!
	// And let's send something, for kicks
	long long secret = run_dh(sock_desc);
	std::cout << "Received secret (" << secret << ")." << std::endl;
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
