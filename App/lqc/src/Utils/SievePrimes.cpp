#include <lqc/Utils/SievePrimes.h>

namespace lqc
{
  SievePrimes::SievePrimes (const int sieve_size)
  {
    build ();
  }

  void SievePrimes::build ()
  {
    int i, j, max = SIEVE_SIZE + 1;
    bs.reset ();
    bs.flip ();
    bs.set (0, false);
    bs.set (1, true);
    for (i = 2; i <= max; i++)
    {
      if (bs.test (i) == true)
      {
        primes.push_back (i);
        for (j = i*i; j <= max; j += i)
        {
          bs.set (j, false);
        }
      }
    }
  }

  bool SievePrimes::isPrime (int p)
  {
    if (p < SIEVE_SIZE)
      return bs.test (p);
    for (int i = 0; i < (int)primes.size () - 1; i++)
    {
      if (p % primes[i] == 0)
        return false;
    }
    return true;
  }
}