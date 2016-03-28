#ifndef LQC_UTILS_SIEVEPRIMES_H
#define LQC_UTILS_SIEVEPRIMES_H

#include <bitset>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <iostream>

namespace lqc {
  class SievePrimes
  {
  public:
    SievePrimes (const int sieve_size = 1000);
    bool isPrime (int p);
  protected:
    static const int SIEVE_SIZE = 1000;
    void build ();
  private:
    std::vector<int> primes;
    std::bitset<SIEVE_SIZE + 2> bs;
  };
}

#endif