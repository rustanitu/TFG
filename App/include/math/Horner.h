#ifndef LQC_MATH_CIRCLE_H
#define LQC_MATH_CIRCLE_H

#include <iostream>
#include <vector>

namespace lqc
{
  double Horner(std::vector<double> v, double x)
  {
    double s = 0;

    for (std::vector<double>::const_reverse_iterator i = v.rbegin(); i != v.rend(); i++)
      s = s*x + *i;
    return s;
  }

  int HornerMain()
  {
    double c[] = { -19, 7, -4, 6 };
    std::vector<double> v(c, c + sizeof(c) / sizeof(double));
    std::cout << Horner(v, 3.0) << std::endl;
    return 0;
  }

  template<typename BidirIter>
  double BidirHorner(BidirIter begin, BidirIter end, double x)
  {
    double result = 0;
    while (end != begin)
      result = result*x + *--end;
    return result;
  }

  int BidirMain()
  {
    double c[] = { -19, 7, -4, 6 };
    std::cout << BidirHorner(c, c + 4, 3) << std::endl;
  }

  /*
  Step 1: Set p = a[n] and q = 0
  Step 2 : Do steps 3 and 4 for i from n - 1 to 0, decreasing by 1
    Step 3 : set q = p + x0 * q
    Step 4 : set p = a[i] + x0 * p
  Step 5 : The value of P(x_0) is p and the value of P'(x_0) is q
  */
  std::vector<double> HornerDual(std::vector<double> v, double x0)
  {
    std::vector<double> r;
    double p = v[v.size()-1];
    double q = 0.0;

    for (int i = v.size() - 2 ; i >= 0 ; i--)
    {
      q = p + x0 * q;
      p = v[i] + x0 * p;
    }
    r.push_back(p);
    r.push_back(q);
    return r;
  }
}

#endif