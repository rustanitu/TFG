//Link http://www.cs.virginia.edu/~gfx/pubs/antimony/
//Credits to Daniel Dunbar & Greg Humphreys

#ifndef LQC_SAMPLING_POISSONDISK
#define LQC_SAMPLING_POISSONDISK

#include <cmath>
#include <vector>

#include <math/Vector2.h>
#include <math/MGeometry.h>
#include <lqc/Utils/Utils.h>

#include <lqc/lqcdefines.h>

#include <lqc/Sampling/RangeList.h>

namespace lqc
{
#ifndef PDGridMaxSamplePerCell
#define PDGridMaxSamplePerCell 9
#endif

  class PoissonDiskSampler
  {
  public:
    PoissonDiskSampler (float radius, bool isTiled, bool use_grid = true);
    ~PoissonDiskSampler ();

    std::vector<Vector2f> GetPoints ();

    virtual void Do () = 0;
    virtual bool Step () = 0;

    std::string SamplerType;

  protected:
    void GetGridXY (Vector2f v, int *gx_out, int *gy_out);
    Vector2f GetTiled (Vector2f v);

    Vector2f RandomPoint ();
    void AddPoint (Vector2f pt);
    bool PointInDomain (Vector2f p);
    void ResetPoints ();

    // populate m_neighbors with list of
    // all points within _radius_ of _pt_
    // and return number of such points
    int FindNeighbors (Vector2f pt, float distance);

    // return distance to closest neighbor within _radius_
    float FindClosestNeighbor (Vector2f pt, float distance);

    // find available angle ranges on boundary for candidate 
    // by subtracting occluded neighbor ranges from _rl_
    void FindNeighborRanges (int index, RangeList* rl);

    float m_radius;
    std::vector<Vector2f> m_points;

    
    //Se for tiled:
    //Um ponto fora da grade é realocado para
    // dentro da mesma
    bool m_is_tiled;

    //auxiliar variable
    std::vector<int> m_neighbors;

    bool m_use_grid;
    int (*m_grid)[PDGridMaxSamplePerCell];
    int m_gridSize;
    float m_gridCellSize;

  private:
  };

  //Dart Throwing method
  // O(n²) without grid
  class DartThrowing : public PoissonDiskSampler
  {
  public:
    DartThrowing (float radius, bool isTiled, int minMaxThrows, int maxThrowsMult);
    virtual void Do ();
    virtual bool Step ();
  private:
    int m_minMaxThrows, m_maxThrowsMult;
  };

  //Best Candidate method
  // O(n²) without grid
  class BestCandidate : public PoissonDiskSampler {
  public:
    BestCandidate (float radius, bool isTiled, int multiplier);
    virtual void Do ();
    virtual bool Step ();
  private:
    int m_multiplier, m_N;
    int m_i_stepaux;
  };

  //////////////////////////////////////////////
  // Daniel Dunbar, Greg Humphreys Algorithms //
  //////////////////////////////////////////////
  
  //Pure Sampler method [A1]
  class PureSampler : public PoissonDiskSampler {
  public:
    PureSampler (float radius);
    virtual void Do ();
    virtual bool Step ();
  };

  //Linear Pure Sampler method [A2]
  class LinearPureSampler : public PoissonDiskSampler {
  public:
    LinearPureSampler (float radius);
    virtual void Do ();
    virtual bool Step ();
  };

  //Boundary Sampler method [A3]
  class BoundarySampler : public PoissonDiskSampler {
  public:
    BoundarySampler (float radius, bool isTiled);
    ~BoundarySampler ();
    
    virtual void Do ();
    virtual bool Step ();

    Vector2f m_current_candidate;
  private:
    RangeList *m_rl;
    std::vector<int> m_candidates;
    int m_step;
  };

  //////////////////////////////////////////////
  // Daniel Dunbar, Greg Humphreys Algorithms //
  //////////////////////////////////////////////

  //Penrose Sampler method
  class PenroseSampler : public PoissonDiskSampler {
  public:
    //Não precisa de grid
    PenroseSampler (float radius);
    virtual void Do ();
    virtual bool Step ();
  };

  //Uniform Sampler method
  class UniformSampler : public PoissonDiskSampler {
  public:
    //Não precisa de grid
    UniformSampler (float radius);
    virtual void Do ();
    virtual bool Step ();
  };
}

#endif