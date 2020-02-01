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

// Mod inv; returns a^-1 (mod b)
long long mod_inv(long long a, long long b)
{
	int mod = b;
	// Casual magic box algorithm
	int x0=0, x1=1, y0=1, y1=0;
	while(a)
	{
		int q = b / a;
		int a_buf = a; a = b % a; b = a_buf;
		int y_buf = y0; y0 = y1; y1 = y_buf - q * y1;
		int x_buf = x0; x0 = x1; x1 = x_buf - q * x1;
	}
	// Adjust for sign
	while(x0 < 0) x0 += mod;
	return x0;
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

long long gen_prime(long long lower)
{
	long long p = lower + !(lower%2);
	while(!is_prime(p))
		p += 2;
	return p;
}
