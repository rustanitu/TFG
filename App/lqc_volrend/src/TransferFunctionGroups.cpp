#include <volrend/TransferFunctionGroups.h>

#include <fstream>
#include <cstdlib>
#include <algorithm>

namespace vr
{
  TransferFunctionGroups::TransferFunctionGroups ()
  {}

  TransferFunctionGroups::~TransferFunctionGroups ()
  {
    m_groups.clear ();
  }

  const char* TransferFunctionGroups::GetNameClass ()
  {
    return "TransferFunctionGroups";
  }

  lqc::Vector4d TransferFunctionGroups::Get (double value)
  {
    lqc::Vector4d vf = lqc::Vector4d (0);
    assert (value >= m_minvalue && value <= m_maxvalue);

    int index = (int)value;
    if (value != 0) return lqc::Vector4d (1,0,0,1);
    /* for (int i = 0; index >= m_groups[i].value && i < (int)m_groups.size (); i++)
    {
      if (index == m_groups[i].value)
      {
        vf = m_groups[i].color;
        break;
      }
    }*/
    return vf;
  }

  gl::GLTexture1D* TransferFunctionGroups::GenerateTexture_RGBA ()
  {
    return NULL;
  }
  
  bool compById (const TransferFunctionGroups::TFGroups_ControlPoint& a, const TransferFunctionGroups::TFGroups_ControlPoint& b)
  {
    return a.value < b.value;
  }

  bool TransferFunctionGroups::AddGroup (int value, lqc::Vector4d color)
  {
    m_groups.push_back (TFGroups_ControlPoint (value, color));
    std::sort (m_groups.begin (), m_groups.end (), compById);
    return true;
  }
}
