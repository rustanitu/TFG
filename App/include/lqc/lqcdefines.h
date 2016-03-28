#ifndef LQC_DEFINES_H
#define	LQC_DEFINES_H

namespace lqc
{
#ifndef DEGREE_TO_RADS
  #define DEGREE_TO_RADS 3.141592654 / 180.0;
#endif

#ifndef uchar
  #define uchar unsigned char
#endif

#ifndef MIN
  #define MIN(a,b) (a < b) ? (a) : (b)
#endif

#ifndef MAX
  #define MAX(a,b) (a > b) ? (a) : (b)
#endif

#ifndef RESOURCE_LIBLQC_PATH
  #define RESOURCE_LIBLQC_PATH "../Resources/"
#endif

#ifndef THRESHOLD_INFINITE_LOOP
  #define THRESHOLD_INFINITE_LOOP 99999
#endif

#ifndef KEPSILON
  #define KEPSILON 0.001
#endif

#ifndef MEPSILON
  #define MEPSILON 0.000001
#endif

//square
//if SQR(n) is equal to n*n
//SQR(1+1) expand to (1+1*1+1) == 3
//need to change to ((x)*(x))
#ifndef SQR
  #define SQR(n) ((n)*(n))
#endif

#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#ifndef PI
  #define PI 3.1415926535897932384626433832795028841971693993751058
#endif

#ifndef PIOVER180
#define PIOVER180 PI/180.0
#endif

#ifndef LQC_PI
  #define LQC_PI 3.1415926535897932384626433832795028841971693993751058
#endif

#ifndef E
  #define E 2.7182818284590452353602874713526624977572470937
#endif

#ifndef TERA10 //T
  #define TERA10 1000000000000
#endif

#ifndef GIGA10 //G
  #define GIGA10 1000000000
#endif

#ifndef MEGA10 //M
  #define MEGA10 1000000
#endif

#ifndef KILO10 //k
  #define KILO10 1000
#endif

#ifndef HECTO10 //h
  #define HECTO10 100
#endif

#ifndef DECA10 //da
  #define DECA10 10
#endif

#ifndef DECI10 //d
  #define DECI10 0.1
#endif
  
#ifndef CENTI10 //c
  #define CENTI10 0.01
#endif

#ifndef MILLI10 //m
  #define MILLI10 0.001
#endif

#ifndef MICRO10 //Mu (letter)
  #define MICRO10 0.000001
#endif

#ifndef NANO10 //n
  #define NANO10 0.000000001
#endif

#ifndef PICO10 //p
  #define PICO10 0.000000000001
#endif
}
#endif

