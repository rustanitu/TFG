//////////TIMER
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>
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
//////////TIMER