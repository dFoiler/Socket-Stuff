long long pow_mod(int a, long long power, long long mod)
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
	long long toTest[12] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
	// Else we begin by throwing out all evens
	if(!(n & 1)) return false;
	// Begin MR test
	long long d = (n-1) >> 1;
	long long r = 1;
	while (!(d & 1))
	{
		d >>= 1;
		++r;
	}
	for(long long p : toTest)
	{
		long long x = pow_mod(p, d, n);
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
	// Start at the next odd
	long long p = lower + ! (lower&1);
	while(!is_prime(p) && !is_prime((p-1) >> 1))
		p += 2;
	return p;
}
