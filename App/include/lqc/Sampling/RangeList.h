//Link http://www.cs.virginia.edu/~gfx/pubs/antimony/
//Credits to Daniel Dunbar & Greg Humphreys

#ifndef LQC_SAMPLING_RANGELIST
#define LQC_SAMPLING_RANGELIST

#include <vector>

namespace lqc
{
  typedef struct _RangeEntry {
    float min, max;
  } RangeEntry;

  class RangeList {
  public:
    RangeEntry *ranges;
    int numRanges, rangesSize;

  public:
    RangeList (float min, float max);
    ~RangeList ();

    void reset (float min, float max);

    void print ();

    void subtract (float min, float max);

  private:
    void deleteRange (int pos);
    void insertRange (int pos, float min, float max);
  };
}

#endif