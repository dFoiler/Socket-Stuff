#include <fstream>

unsigned long long get_rand()
{
	unsigned long long r; size_t size = sizeof(r);
	// Open up the portal to Narnia
	std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
	// To read char bytes into our long, we do sketchy stuff
	urandom.read(reinterpret_cast<char*>(&r), size);
	return r;
}

long long pow_mod(long long a, long long power, long long mod)
{
	long long r = 1;
	while(power)
	{
		if(power & 1)
			r = (r * a) % mod;
		a = (a*a) % mod;
		power = power >> 1;
	}
	return r;
}

// MR good to 2^64
bool is_prime(long long n)
{
	if(n < 0) return is_prime(-n);
	// Test small primes
	long long to_test[12] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
	// Else we begin by throwing out all evens
	if(!(n & 1)) return false;
	// Begin MR test
	long long d = (n-1) >> 1;
	long long r = 1;
	while (!(d % 2))
	{
		d >>= 1;
		++r;
	}
	for(int i = 0; i < 7; ++i)
	{
		long long x = pow_mod(to_test[i], d, n);
		if(x == 1 || x == n-1)
			continue;
		bool composite = true;
		for(long long i = 0; i < r-1; ++i)
		{
			x = (x*x) % n;
			if(x == n-1)
			{
				composite = false;
				break;
			}
		}
		if(composite)
			return false;
	}
	return true;
}

long long gen_safe_prime(long long lower)
{
	// We automatically need to be 11 mod 12
	long long p = lower + 11 - (lower % 12);
	while(!is_prime(p) || !is_prime( (p-1) >> 1 ))
		p += 12;
	return p;
}
