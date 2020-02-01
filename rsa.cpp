#include <gmp.h>
#include <fstream>	// reading from urandom
#include <iostream>

// Converts a string of bytes to an mpz number
void str_to_mpz(char* str, int len, mpz_t n)
{
	// Convert the string to a hex string
	char hex[len*2+1];
	for(int i = 0; i < len; ++i)
	{
		// Each character in the string stores 2 hex digits
		char byte = str[i];
		// Separate out the bytes
		hex[2*i] = (unsigned int)byte >> 4;
		hex[2*i+1] = (unsigned int)byte % 16;
		// Map to hex digits
		hex[2*i] += hex[2*i] < 10 ? '0' : ('a'-10);
		hex[2*i+1] += hex[2*i+1] < 10 ? '0' : ('a'-10);
	}
	// Null-terminate manually
	hex[2*len] = '\0';
	// Now set
	mpz_set_str(n, hex, 16);
}

// Converts an mpz number to a string
// Assumes there's enough space in str to store
void mpz_to_str(mpz_t n, char* str)
{
	// Throw n into a hex string
	char hex[2*sizeof(str)+1];
	// Clear hex
	for(int i = 0; i < sizeof(hex); ++i)
		hex[i] = '\0';
	mpz_get_str(hex, 16, n);
	// Now work through hex to make a readable string
	for(int i = 0; i < sizeof(str); ++i)
	{
		// Extract nibbles
		char hi=hex[2*i], lo=hex[2*i+1];
		// Convert to bytes; gmp uses lower case
		lo -= lo <= '9' ? '0' : ('a'-10);
		hi -= hi <= '9' ? '0' : ('a'-10);
		// Now throw into str
		str[i] = 16*hi + lo;
	}
}

// Some stupid helper I always seem to need
std::ostream& operator<<(std::ostream& o, mpz_t z)
{
	return o << mpz_get_str(NULL, 10, z);
}

// Reads from urandom
void set_rand(mpz_t r, int bits)
{
	// Read random bytes into buf
	char buf[bits/8+1];
	std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
	urandom.read(buf, sizeof(buf));
	// Now convert buf to a hex number
	char num[sizeof(buf)+1];
	for(int b = 0; b < sizeof(buf); ++b)
	{
		num[b] = (unsigned int)buf[b] % 16;
		if(num[b] < 10)
			num[b] += '0';
		else
			num[b] += 'a'-10;
	}
	// Null-terminate the string manually
	num[sizeof(buf)] = '\0';
	mpz_set_str(r, num, 16);
}

// Sets up RSA cryptosystem
void setup_rsa(int bits, mpz_t N, mpz_t e, mpz_t d)
{
	// Generate secret keys
	mpz_t p, q, phi; mpz_inits(p, q, phi, NULL);
	set_rand(p, bits/2-1); set_rand(q, bits/2-1);
	bool go = true;
	while(go)
	{
		mpz_nextprime(p, p); mpz_nextprime(q, q);
		// Make sure that phi(pq) is coprime with 65537
		go = (mpz_cdiv_ui(p, 65537)-1) * (mpz_cdiv_ui(q, 65537)-1);
		go = !go;
	}
	// Now initialize variables
	mpz_mul(N, p, q);
	mpz_set_ui(e, 65537);
	// Compute phi slowly
	mpz_set(phi, p); // p
	mpz_sub_ui(phi, phi, 1); // p-1
	mpz_mul(phi, phi, q); // pq-q
	mpz_sub(phi, phi, p); // pq-q-p
	mpz_add_ui(phi, phi, 1); // pq-q-p+1
	// Now here's the secret key
	mpz_invert(d, e, phi); // e is prime, so we've checked phi already
	mpz_clears(p, q, phi, NULL);
}

// Assumes that enc makes something smaller than N
// Otherwise the encryption breaks
void enc_rsa(char* enc, char* mes, mpz_t N, mpz_t e)
{
	// Convert char* to 
}
