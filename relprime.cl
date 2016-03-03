long hcf(long x, long y)
{
  long t;

  while (y != 0) {
    t = x % y;
    x = y;
    y = t;
  }
  return x;
}

// relprime x y = hcf x y == 1
__kernel void relprime(
	__global int* result,
	const unsigned int n
	)
{
	int i = get_global_id(0);
	if(n >= i+1)
		result[i] = hcf(n, i+1);
}
