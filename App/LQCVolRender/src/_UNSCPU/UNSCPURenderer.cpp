#include "UNSCPURenderer.h"

#include "../Viewer.h"

#include <math/MGeometry.h>

UNSCPURenderer::UNSCPURenderer ()
: m_pthread_on (false),
m_pixels (NULL),
m_aabb (NULL)
{
  m_aabb_min_x = m_aabb_min_y = m_aabb_min_z = -16.0f;
  m_aabb_max_x = m_aabb_max_y = m_aabb_max_z =  16.0f;

  m_camera_eye_initial_x = 0.0f;
  m_camera_eye_initial_y = 0.0f;
  m_camera_eye_initial_z = 50.0f;

  m_num_of_threads = 2;

  m_scaletomaxanchorvalue = true;
  m_max_aabb_anchor_value = 32.f;

}

UNSCPURenderer::~UNSCPURenderer ()
{
  Destroy ();
}

bool UNSCPURenderer::Render (int Width, int Height)
{
  Viewer::Instance ()->m_volume->SetAnchors (m_aabbmin, m_aabbmax);
  m_voleval.m_statistics.Reset ();
  if (m_pthread_on)
  {
    int in_even[2] = { 0, 0 };
    int in_odd[2] = { 1, 0 };
    int in_even_r2[2] = { 0, 1 };
    int in_odd_r2[2] = { 1, 1 };
    RayTracerPThread rtt[4];

    if (m_num_of_threads == 4)
    {
      rtt[0].cpu_viewer = rtt[1].cpu_viewer = rtt[2].cpu_viewer = rtt[3].cpu_viewer = this;
      rtt[0].rtwidth = rtt[1].rtwidth = rtt[2].rtwidth = rtt[3].rtwidth = Width;
      rtt[0].rtheight = rtt[1].rtheight = rtt[2].rtheight = rtt[3].rtheight = Height;

      rtt[0].starter = in_even;
      rtt[1].starter = in_odd;
      rtt[2].starter = in_even_r2;
      rtt[3].starter = in_odd_r2;

      rtt[0].i_iterator = rtt[1].i_iterator = rtt[2].i_iterator = rtt[3].i_iterator = 2;
      rtt[0].j_iterator = rtt[1].j_iterator = rtt[2].j_iterator = rtt[3].j_iterator = 2;

      pthread_create (&m_t_even, NULL, &RayTracerPThreadCall, &rtt[0]);
      pthread_create (&m_t_odd, NULL, &RayTracerPThreadCall, &rtt[1]);
      pthread_create (&m_t_even_r2, NULL, &RayTracerPThreadCall, &rtt[2]);
      pthread_create (&m_t_odd_r2, NULL, &RayTracerPThreadCall, &rtt[3]);

      void *result;
      pthread_join (m_t_even, &result);
      pthread_join (m_t_odd, &result);
      pthread_join (m_t_even_r2, &result);
      pthread_join (m_t_odd_r2, &result);
    }
    else //m_num_of_threads == 2
    {
      rtt[0].cpu_viewer = rtt[1].cpu_viewer = this;
      rtt[0].rtwidth = rtt[1].rtwidth = Width;
      rtt[0].rtheight = rtt[1].rtheight = Height;

      rtt[0].starter = in_even;
      rtt[1].starter = in_odd;

      rtt[0].i_iterator = rtt[1].i_iterator = rtt[2].i_iterator = rtt[3].i_iterator = 2;
      rtt[0].j_iterator = rtt[1].j_iterator = rtt[2].j_iterator = rtt[3].j_iterator = 1;

      pthread_create (&m_t_even, NULL, &RayTracerPThreadCall, &rtt[0]);
      pthread_create (&m_t_odd, NULL, &RayTracerPThreadCall, &rtt[1]);

      void *result;
      pthread_join (m_t_even, &result);
      pthread_join (m_t_odd, &result);
    }
  }
  else
  {
    RayTracer (Width, Height);
  }

  Display ();

  return true;
}

void UNSCPURenderer::ResetCamera (int width, int height)
{
  rt::Struct_RTCamera_init st;
  st.eye = lqc::Vector3f (m_camera_eye_initial_x, m_camera_eye_initial_y, m_camera_eye_initial_z);
  st.at = lqc::Vector3f (0.0f, 0.0f, 0.0f);
  st.up = lqc::Vector3f (0.0f, 1.0f, 0.0f);
  
  st.fovy = 90.0f;
  st.nearp = 30.0f;
  st.farp = 30.0f;
  st.w = (float)width;
  st.h = (float)height;

  m_camera.Init (&st);
}

void UNSCPURenderer::Destroy ()
{
  if (m_pixels)
    delete m_pixels;
  m_pixels = NULL;
  
  if (m_aabb)
    delete m_aabb;
  m_aabb = NULL;
}

void UNSCPURenderer::ResetAABB (vr::Volume* volume)
{
  m_aabbmin = lqc::Vector3f (m_aabb_min_x, m_aabb_min_y, m_aabb_min_z);
  m_aabbmax = lqc::Vector3f (m_aabb_max_x, m_aabb_max_y, m_aabb_max_z);

  if (m_aabb)
    delete m_aabb;
  m_aabb = new rt::RTAABB (GetMinAnchor (), GetMaxAnchor ());
  volume->SetAnchors (GetMinAnchor (), GetMaxAnchor ());
}

void UNSCPURenderer::SetAABB (vr::Volume* volume, float x0, float y0, float z0, float x1, float y1, float z1)
{
  m_aabb_min_x = x0;
  m_aabb_min_y = y0;
  m_aabb_min_z = z0;
  m_aabb_max_x = x1;
  m_aabb_max_y = y1;
  m_aabb_max_z = z1;
  ResetAABB (volume);
}

lqc::Vector3f UNSCPURenderer::GetMinAnchor ()
{
  return m_aabbmin;
}

lqc::Vector3f UNSCPURenderer::GetMaxAnchor ()
{
  return m_aabbmax;
}

lqc::Vector4f UNSCPURenderer::Trace (rt::RTRay ray, int  depth, int i, int j)
{
  lqc::Vector4f color = lqc::Vector4f (0.0f, 0.0f, 0.0f, 0.0f);

  float tmin, tmax;
  lqc::Vector3f normal;
  if (m_aabb->Hit (ray, tmin, tmax, normal))
  {
    if (tmax < tmin)
    {
      float ttmin = tmin;
      tmin = tmax;
      tmax = ttmin;
    }
    lqc::Vector3f minpoint = ray.GetPosition () + ray.GetDirection () * tmin;
    lqc::Vector3f maxpoint = ray.GetPosition () + ray.GetDirection () * tmax;
    color = Shade (m_aabb, minpoint, maxpoint, normal, ray.GetPosition (), ray.GetDirection (), i, j);
  }
  return color;
}

void UNSCPURenderer::SetModelVolumeAndTransferFunction (vr::Volume* ivolume, vr::TransferFunction* itransfer_function)
{
  m_voleval.SetModelAndTransferFunction (ivolume, itransfer_function);
}

void UNSCPURenderer::Display ()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDrawPixels (Viewer::Instance ()->m_CurrentWidth, Viewer::Instance ()->m_CurrentHeight, GL_RGBA, GL_FLOAT, m_pixels);
}

void UNSCPURenderer::RayTracer (int rtwidth, int rtheight)
{
  for (int j = 0; j < rtheight; j++)
  {
    for (int i = 0; i < rtwidth; i++)
    {
      lqc::Vector4f pixel_color = Trace (rt::RTRay (m_camera.GetPosition (), m_camera.CastRay ((float)i, (float)j)), 1, i, j);
      m_pixels[i * 4 + j*rtwidth * 4] = pixel_color.x;
      m_pixels[i * 4 + j*rtwidth * 4 + 1] = pixel_color.y;
      m_pixels[i * 4 + j*rtwidth * 4 + 2] = pixel_color.z;
      m_pixels[i * 4 + j*rtwidth * 4 + 3] = pixel_color.w;
    }
  }
}

lqc::Vector4f UNSCPURenderer::Shade (rt::RTObject* object, lqc::Vector3f minpoint, lqc::Vector3f maxpoint, lqc::Vector3f normal, lqc::Vector3f eye, lqc::Vector3f eyedirection, int i, int j)
{
  lqc::Vector4f color;

  if (minpoint != maxpoint)
  {
    float StepSize = (lqc::Distance (minpoint, maxpoint) / m_voleval.m_samples);
    lqc::Vector3f Step = lqc::Vector3f::Normalize (eyedirection)*StepSize;
    lqc::Vector3f pos = minpoint;
    lqc::Vector4d dst;
    m_voleval.EvaluateIntegral (&dst, m_voleval.m_volume, minpoint, maxpoint);

    color = lqc::Vector4f ((float)dst.x, (float)dst.y, (float)dst.z, (float)dst.w);
  }

  return color;
}

void* UNSCPURenderer::RayTracerPThreadCall (void* input)
{
  RayTracerPThread* rt = (RayTracerPThread*)input;
  for (int j = rt->starter[1]; j < rt->rtheight; j += rt->j_iterator)
  {
    for (int i = rt->starter[0]; i < rt->rtwidth; i += rt->i_iterator)
    {
      lqc::Vector4f pixel_color = rt->cpu_viewer->Trace (rt::RTRay (rt->cpu_viewer->m_camera.GetPosition(), rt->cpu_viewer->m_camera.CastRay ((float)i, (float)j)), 1, i, j);
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4] = pixel_color.x;
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4 + 1] = pixel_color.y;
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4 + 2] = pixel_color.z;
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4 + 3] = pixel_color.w;
    }
  }
  return NULL;
}
