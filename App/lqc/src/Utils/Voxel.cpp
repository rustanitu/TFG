#include <lqc/Utils/Voxel.h>

#include <math/Vector3.h>

namespace lqc
{
  Voxel::Voxel ()
  {
    m_scalar_values[0] = 0.0f; m_scalar_values[1] = 0.0f; m_scalar_values[2] = 0.0f; m_scalar_values[3] = 0.0f;
    m_scalar_values[4] = 0.0f; m_scalar_values[5] = 0.0f; m_scalar_values[6] = 0.0f; m_scalar_values[7] = 0.0f;

    m_vertex[0] = Vector3f (0.0f, 0.0f, 1.0f);
    m_vertex[1] = Vector3f (1.0f, 0.0f, 1.0f);
    m_vertex[2] = Vector3f (0.0f, 1.0f, 1.0f);
    m_vertex[3] = Vector3f (1.0f, 1.0f, 1.0f);
    m_vertex[4] = Vector3f (0.0f, 0.0f, 0.0f);
    m_vertex[5] = Vector3f (1.0f, 0.0f, 0.0f);
    m_vertex[6] = Vector3f (0.0f, 1.0f, 0.0f);
    m_vertex[7] = Vector3f (1.0f, 1.0f, 0.0f);
  }

  Voxel::Voxel (Vector3f vmin, Vector3f vmax)
  {
    m_vertex[0] = Vector3f (vmin.x, vmin.y, vmax.z);
    m_vertex[1] = Vector3f (vmax.x, vmin.y, vmax.z);
    m_vertex[2] = Vector3f (vmin.x, vmax.y, vmax.z);
    m_vertex[3] = Vector3f (vmax.x, vmax.y, vmax.z);
    m_vertex[4] = Vector3f (vmin.x, vmin.y, vmin.z);
    m_vertex[5] = Vector3f (vmax.x, vmin.y, vmin.z);
    m_vertex[6] = Vector3f (vmin.x, vmax.y, vmin.z);
    m_vertex[7] = Vector3f (vmax.x, vmax.y, vmin.z);
  }

  Voxel::~Voxel ()
  {
  }

  float Voxel::GetScalar (Vector3f pos)
  {
    return -1.0f;
  }
}