#ifndef LQC_UTILS_UTILS_H
#define LQC_UTILS_UTILS_H

#include <cstdlib>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <random>
#include <math/Vector2.h>
#include <math/Circle.h>
#include <limits>

namespace lqc
{
  template<typename T>
  bool isnan (T value)
  {
    return value != value;
  }

  template<typename T>
  bool isinf (T value)
  {
    return std::numeric_limits<T>::has_infinity &&
    value == std::numeric_limits<T>::infinity ();
  }

  /*
  * Does not include \n
  */
  void DebugPrint (char* toprint);

  void PrintlqcConsoleMessage (char* nameclass, char* message);

  std::vector<Vector2f> generateRandomPoints (int n, float range_x, float range_y);

  std::string intToString (int num);
  int stringToInt (std::string aString);

  /**
  * Greatest Common Divisor
  * Returns: the largest positive integer that divides the numbers without a remainder
  */
  int GCD (int a, int b);

  /**
  * Least common multiple
  * Returns: the smallest positive integer that is divisible by both a and b
  */
  int LCM (int a, int b);

  /**
  * Least Prime Factor
  * Ex: 6 = 2 * 3, returns 2
  */
  int LPF (int n);

  int Round (float number);

  double round (double number);
  double round (double val, int precision);

  bool isPerfectSquare (int value);

  bool bissexto (int ano);

  /**
   * Days and months starts in 1
   */
  bool isConsecutiveDay (int D1, int M1, int Y1, int D2, int M2, int Y2);

  // types: float, char, string and int
  // example: ShowVar ("fcsi", 32.4f, 'a', "Test string", 4);
  //                             f     c        s         i
  void ShowVars (char *szTypes, ...);
  /*
  int min (int n_size, ...);

  int max (int n_size, ...);
  */
  int simplerandom (int a, int b);

  int c11randomnumber (int a, int b);

  //todo
  //class AutoString
  class AutoString
  {
    AutoString ()
    {
      m_content = NULL;
    }

    AutoString (char* m_text)
    {
      m_content = new char[strlen (m_text)];
      //strcpy (m_content, m_text);
    }

    ~AutoString ()
    {
      if (m_content)
        delete[] m_content;
    }

  private:
    char* m_content;
  };

  float inline frand () { return (float)rand () / (float)(RAND_MAX); }; //returns from 0 to 1
  float inline frand (float Max) { return frand () * Max; }; //returns from 0 to Max
  float inline frand (float Min, float Max) { return (frand () * (Max - Min)) + Min; }; //returns from Min to Max

  int inline irand (int Max) { return (int)((((float)rand () / (float)(RAND_MAX + 1)) * (float)(Max + 1))); }; //returns from 0 to Max
  int inline irand (int Min, int Max) { return irand (Max - Min) + Min; }; //returns from Min to Max

  double inline drand () { return (double)rand () / (double)(RAND_MAX); }; //returns from 0 to 1
  double inline drand (double Max) { return (drand () * (float)Max); }; //returns from 0 to Max
  double inline drand (double Min, double Max) { return drand (Max - Min) + Min; }; //returns from Min to Max

  template<typename TYPE> inline TYPE trand (TYPE min, TYPE max) { return trand (max - min) + min; };
  template<typename TYPE> inline TYPE trand (TYPE max) { return (TYPE)frand ((float)max); };
  template<> inline int trand<int> (int max) { return irand (max); };
  template<> inline float trand<float> (float max) { return frand (max); };
  template<> inline double trand<double> (double max) { return drand (max); };

  /*class timer
  {
  public:
  static bool s_initialized = false;
  static bool s_highrescompatible;
  static LARGE_INTEGER s_startcounter;
  static LARGE_INTEGER s_tickspersecond;
  static double s_starttime;

  static void uso_timeinit ()
  {
  if (QueryPerformanceFrequency (&s_tickspersecond))
  {
  s_highrescompatible = true;
  QueryPerformanceCounter (&s_startcounter);
  struct _timeb t;
  _ftime (&t);
  s_starttime = ((double)t.time) + 0.001*t.millitm;
  }
  else
  s_highrescompatible = false;
  }

  double uso_gettime (void)
  {
  if (!s_initialized)
  {
  uso_timeinit ();
  s_initialized = true;
  }
  if (s_highrescompatible)
  {
  LARGE_INTEGER currentcounter;
  QueryPerformanceCounter (&currentcounter);
  double seconds = ((double)currentcounter.QuadPart - (double)s_startcounter.QuadPart) / (double)s_tickspersecond.QuadPart;
  return s_starttime + seconds;
  }
  else
  {
  struct _timeb t;
  _ftime (&t);
  return ((double)t.time) + 0.001*t.millitm;
  }
  }
  };*/
}

#endif