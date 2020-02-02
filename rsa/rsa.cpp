#include <gmp.h>
#include <fstream>	// reading from urandom
#include <iostream>
#include <string>	// For easy packing

// Converts a string of bytes to an mpz number
void str_to_mpz(const std::string str, mpz_t n)
{
	// Convert the string to a hex string
	char hex[2*str.length()+1];
	// Clear
	for(int i = 0; i < sizeof(hex); ++i)
		hex[i] = '\0';
	// Go until null-terminated string
	for(int i = 0; i < str.length(); ++i)
	{
		// Each character in the string stores 2 hex digits
		char byte = str[i];
		// Separate out the bytes
		hex[2*i] = (unsigned char)byte >> 4;
		hex[2*i+1] = (unsigned char)byte % 16;
		// Map to hex digits
		hex[2*i] += (unsigned char)hex[2*i] < 10 ? '0' : ('a'-10);
		hex[2*i+1] += (unsigned char)hex[2*i+1] < 10 ? '0' : ('a'-10);
	}
	// Manually null-terminate
	hex[2*str.length()] = '\0';
	// Now set
	mpz_set_str(n, hex, 16);
}

// Converts an mpz number to a char*
std::string mpz_to_str(const mpz_t n)
{
	// Throw n into a hex string
	std::string hex(mpz_get_str(NULL, 16, n));
	if(hex.length() % 2)
		hex = "0" + hex;
	// Now work through hex to make a readable string
	std::string r = "";
	for(int i = 0; i < hex.length()/2; ++i)
	{
		// Extract nibbles
		char hi=hex[2*i], lo=hex[2*i+1];
		// Convert to bytes; gmp uses lower case
		lo -= lo <= '9' ? '0' : ('a'-10);
		hi -= hi <= '9' ? '0' : ('a'-10);
		// Now throw into str
		r += 16*hi + lo;
	}
	return r;
}

std::string pad(const std::string str)
{
	std::string r(str);
	char pad = 16 - (r.length() % 16);
	for(int i = 0; i < pad; ++i)
		r += pad;
	return r;
}

std::string unpad(const std::string str)
{
	std::string r = "";
	char pad = str[str.length()-1];
	for(int i = 0; i < str.length()-pad; ++i)
		r += str[i];
	return r;
}

// Some stupid helper I always seem to need
std::ostream& operator<<(std::ostream& o, const mpz_t z)
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
	mpz_set(phi, p); 		// p
	mpz_sub_ui(phi, phi, 1);	// p-1
	mpz_mul(phi, phi, q);		// pq-q
	mpz_sub(phi, phi, p);		// pq-q-p
	mpz_add_ui(phi, phi, 1);	// pq-q-p+1
	// Now here's the secret key
	mpz_invert(d, e, phi); // e is prime, so we've checked phi already
	mpz_clears(p, q, phi, NULL);
}

// Assumes that enc makes something smaller than N
// Otherwise the encryption breaks
std::string enc_rsa(const std::string mes, const mpz_t N, const mpz_t e)
{
	// Convert mes to an mpz number
	mpz_t m; mpz_init(m); str_to_mpz(mes, m);
	// Encrypt
	mpz_powm(m, m, e, N);
	// Throw the message into enc
	std::string r = mpz_to_str(m);
	// Now clear and return
	mpz_clear(m);
	return r;
}
