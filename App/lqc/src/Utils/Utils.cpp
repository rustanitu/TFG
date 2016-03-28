#include <lqc/Utils/Utils.h>

#include <cstdarg>

namespace lqc
{
  void DebugPrint (char* toprint)
  {
    #if _DEBUG
    printf ("%s", toprint);
    #endif
  }

  void PrintlqcConsoleMessage (char* nameclass, char* message)
  {
    printf("lqc: %s\n  - %s\n", nameclass, message);
  }

  std::vector<Vector2f> generateRandomPoints(int n, float range_x, float range_y)
  {
    std::vector<Vector2f> ret;
    int rx = (int)range_x; int ry = (int)range_y;
    float range2_x = range_x/2.0f;
    float range2_y = range_y/2.0f;
    for(int i = 0 ; i < n ; i++)
    {
      ret.push_back(Vector2f(rand()%rx - range2_x, rand()%ry - range2_y));
    }
    return ret;
  }

  std::string intToString(int num)
  {
    std::stringstream s;
    s << num;
    return s.str();
  }

  int stringToInt(std::string aString)
  {
    std::istringstream s(aString);
    int num;
    s >> num;
    return num;
  }

  int GCD (int a, int b)
  { 
    while (b > 0)
    { 
      a = a % b; 
      a ^= b;    b ^= a;    a ^= b;  
    }  
    return a; 
  }

  int LCM (int a, int b)
  {
    return abs(a*b)/GCD(a,b);
  }

  int LPF (int n)
  {
    // Don't bother to use abs () <stdlib.h>
    if (n < 0)
      n = -n;
    // For n < 2; it is just n
    if (n < 2)
      return n;

    // otherwise, for an even number it is 2
    if (n % 2 == 0)
      return 2;

    // for other odd numbers, search for a divisor
    // until SQRT (n) [conceptual] n / div >= div is
    // equivalent to SQRT (n) >= div
    for (int div = 3; n / div >= div; div += 2)
    {
      // if div is a divisor, just return it.
      if (n % div == 0)
        return div;
    }

    // There is no divisor (n is prime), return it.
    return n;
  }

  int Round (float number)
  {
    return (int)(number+0.5f);
  }

  double round(double number)
  {
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
  }

  double round(double val, int precision)
  {
    std::stringstream s;
    s << std::setprecision(precision) << std::setiosflags(std::ios_base::fixed) << val;
    s >> val;
    return val;
  }

  bool isPerfectSquare(int value)
  {
    int vsqrt = (int)round(sqrt(value));
    return (value >= 0 && value == vsqrt * vsqrt);
  }

  bool bissexto(int ano)
  {
    char bissexto = 0;
    if ( ( (!(ano % 4)) && (ano % 100) ) || (!(ano % 400)) )
      return true;
    return false;
  }

  bool isConsecutiveDay(int D1, int M1, int Y1, int D2, int M2, int Y2)
  {
    int days[13] = { -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(Y1 == Y2)
    {
      if(M1 == M2)
      {
        if(D1 == D2-1)
        {
          return true;
        }
      }else if(M1 == M2-1){
        if(M1 == 2)
        {
          if(bissexto(Y1))
          {
            if(D1 == 29 && D2 == 1)
            {
              return true;
            }
          }else{
            if(D1 == days[M1] && D2 == 1)
            {
              return true;
            }
          }
        }else{
          if(D1 == days[M1] && D2 == 1)
          {
            return true;
          }
        }
      }
    }else if(Y1 == Y2-1)
    {
      if(M1 == 12 && M2 == 1)//Fim e começo de ano
      {
        if(D1 == days[12] && D2 == 1)
        {
          return true;
        }
      }
    }
    return false;
  }

  //  http://msdn.microsoft.com/en-us/library/fxhdxye9.aspx
  //  ShowVar takes a format string of the form
  //   "ifcs", where each character specifies the
  //   type of the argument in that position.
  //
  //  i = int
  //  f = float
  //  c = char
  //  s = string (char *)
  //
  //  Following the format specification is a variable 
  //  list of arguments. Each argument corresponds to 
  //  a format character in the format string to which 
  // the szTypes parameter points 
  void ShowVar (char *szTypes, ...) {
    va_list vl;
    int i;

    //  szTypes is the last argument specified; you must access 
    //  all others using the variable-argument macros.
    va_start (vl, szTypes);

    // Step through the list.
    for (i = 0; szTypes[i] != '\0'; ++i) {
      union Printable_t {
        int     i;
        float   f;
        char    c;
        char   *s;
      } Printable;

      switch (szTypes[i]) {   // Type to expect.
      case 'i':
        Printable.i = va_arg (vl, int);
        printf_s ("%i\n", Printable.i);
        break;

      case 'f':
        Printable.f = va_arg (vl, float);
        printf_s ("%f\n", Printable.f);
        break;

      case 'c':
        Printable.c = va_arg (vl, char);
        printf_s ("%c\n", Printable.c);
        break;

      case 's':
        Printable.s = va_arg (vl, char *);
        printf_s ("%s\n", Printable.s);
        break;

      default:
        break;
      }
    }
    va_end (vl);
  }
  

 /* int min (int n_size, ...)
  {
    va_list vl;
    int i;

    return 0;
  }

  int max (int n_size, ...)
  {
    va_list vl;
    int i;

     return 0;
  }*/

  int simplerandom (int a, int b)
  {
    srand (time (NULL));
    return a + rand () % (b - a);
  }

  int c11randomnumber (int a, int b)
  {
    std::mt19937 rng;
    rng.seed (std::random_device ()());
    std::uniform_int_distribution<std::mt19937::result_type> r (a, b);

    return r (rng);
  }
}