#ifndef LQC_UTILS_LHISTOGRAM_H
#define LQC_UTILS_LHISTOGRAM_H

#include <vector>

namespace lqc
{
  class LHHistogram
  {
  public:
    LHHistogram ();
    LHHistogram (int width, int height);
    ~LHHistogram ();

    /*void Add (float lvalue, float hvalue);
    bool Remove (float lvalue, float hvalue);
    int GetIndex (float lvalue, float hvalue);
    
    void Resize (int widht, int height);

    void Clear ();
    int Size ();

    float GetHighestLValue ();
    float GetLowestLValue ();

    float GetHighestHValue ();
    float GetLowestHValue ();

    int GetMaxCountValue ();

    void GeneratePNG (char* filename, bool use_lqc_resource_path);
    
    void SaveFile (char* filename, bool use_lqc_resource_path= true);
    void LoadFile (char* filename, bool use_lqc_resource_path= true);*/
  private:
    int m_width;
    int m_height;

    class LHPoint
    {
    public:
      int m_hvalue;
      int m_lvalue;
      int m_count;
    };

    std::vector<LHPoint> m_lh_points;

  };
}

#endif