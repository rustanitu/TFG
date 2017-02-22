#include "RendererCPU.h"

#include "../Viewer.h"

#include <math/MGeometry.h>

RendererCPU::RendererCPU()
  : m_pthread_on(false),
  m_pixels(NULL),
  m_aabb(NULL),
  m_camera(rt::RTCamera::CamProjection::PERSPECTIVE)
{
  float number = 256.f;

  m_aabb_min_x = m_aabb_min_y = m_aabb_min_z = -(number / 2.f);
  m_aabb_max_x = m_aabb_max_y = m_aabb_max_z = (number / 2.f);


  m_cam_init_st.eye = lqc::Vector3f(0.0f, 0.0f, number * 1.6f);
  m_cam_init_st.at = lqc::Vector3f(0.0f, 0.0f, 0.0f);
  m_cam_init_st.up = lqc::Vector3f(0.0f, 1.0f, 0.0f);

  m_num_of_threads = 2;

  m_scaletomaxanchorvalue = true;
  m_max_aabb_anchor_value = (number / 2.f);

}

RendererCPU::~RendererCPU()
{
  Destroy();
}

bool RendererCPU::Render(int Width, int Height)
{
  ((vr::Volume*)Viewer::Instance()->m_volume)->SetAnchors(m_aabbmin, m_aabbmax);
  rays_traced = 0;
  //m_voleval.m_statistics.Reset ();
  if (m_pthread_on) {
    int in_even[2] = { 0, 0 };
    int in_odd[2] = { 1, 0 };
    int in_even_r2[2] = { 0, 1 };
    int in_odd_r2[2] = { 1, 1 };
    RayTracerPThread rtt[4];

    if (m_num_of_threads == 4) {
      rtt[0].cpu_viewer = rtt[1].cpu_viewer = rtt[2].cpu_viewer = rtt[3].cpu_viewer = this;
      rtt[0].rtwidth = rtt[1].rtwidth = rtt[2].rtwidth = rtt[3].rtwidth = Width;
      rtt[0].rtheight = rtt[1].rtheight = rtt[2].rtheight = rtt[3].rtheight = Height;

      rtt[0].starter = in_even;
      rtt[1].starter = in_odd;
      rtt[2].starter = in_even_r2;
      rtt[3].starter = in_odd_r2;

      rtt[0].i_iterator = rtt[1].i_iterator = rtt[2].i_iterator = rtt[3].i_iterator = 2;
      rtt[0].j_iterator = rtt[1].j_iterator = rtt[2].j_iterator = rtt[3].j_iterator = 2;

      pthread_create(&m_t_even, NULL, &RayTracerPThreadCall, &rtt[0]);
      pthread_create(&m_t_odd, NULL, &RayTracerPThreadCall, &rtt[1]);
      pthread_create(&m_t_even_r2, NULL, &RayTracerPThreadCall, &rtt[2]);
      pthread_create(&m_t_odd_r2, NULL, &RayTracerPThreadCall, &rtt[3]);

      void *result;
      pthread_join(m_t_even, &result);
      pthread_join(m_t_odd, &result);
      pthread_join(m_t_even_r2, &result);
      pthread_join(m_t_odd_r2, &result);
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

      pthread_create(&m_t_even, NULL, &RayTracerPThreadCall, &rtt[0]);
      pthread_create(&m_t_odd, NULL, &RayTracerPThreadCall, &rtt[1]);

      void *result;
      pthread_join(m_t_even, &result);
      pthread_join(m_t_odd, &result);
    }
  }
  else {
    RayTracer(Width, Height);
  }

  Display();

  return true;
}

void RendererCPU::ResetCamera(int width, int height)
{
  m_cam_init_st.fovy = 60.0f;
  m_cam_init_st.nearp = 30.0f;
  m_cam_init_st.farp = 500.0f;
  m_cam_init_st.w = (float)width;
  m_cam_init_st.h = (float)height;

  m_camera.Init(&m_cam_init_st);
  //m_camera.UseOrthographic (3.5);
}

void RendererCPU::Destroy()
{
  if (m_pixels)
    delete m_pixels;
  m_pixels = NULL;

  if (m_aabb)
    delete m_aabb;
  m_aabb = NULL;
}

void RendererCPU::ResetAABB(vr::Volume* volume)
{
  m_aabbmin = lqc::Vector3f(m_aabb_min_x, m_aabb_min_y, m_aabb_min_z);
  m_aabbmax = lqc::Vector3f(m_aabb_max_x, m_aabb_max_y, m_aabb_max_z);

  if (m_aabb)
    delete m_aabb;
  m_aabb = new rt::RTAABB(GetMinAnchor(), GetMaxAnchor());
  volume->SetAnchors(GetMinAnchor(), GetMaxAnchor());
}

void RendererCPU::SetAABB(vr::Volume* volume, float x0, float y0, float z0, float x1, float y1, float z1)
{
  m_aabb_min_x = x0;
  m_aabb_min_y = y0;
  m_aabb_min_z = z0;
  m_aabb_max_x = x1;
  m_aabb_max_y = y1;
  m_aabb_max_z = z1;
  ResetAABB(volume);
}

lqc::Vector3f RendererCPU::GetMinAnchor()
{
  return m_aabbmin;
}

lqc::Vector3f RendererCPU::GetMaxAnchor()
{
  return m_aabbmax;
}

lqc::Vector4f RendererCPU::Trace(rt::RTRay ray, int  depth, int i, int j)
{
  lqc::Vector4f color = lqc::Vector4f(0.0f, 0.0f, 0.0f, 0.0f);

  float tmin, tmax;
  lqc::Vector3f normal;
  if (m_aabb->Hit(ray, tmin, tmax, normal)) {
    if (tmax < tmin) {
      float ttmin = tmin;
      tmin = tmax;
      tmax = ttmin;
    }
    lqc::Vector3f minpoint = ray.GetPosition() + ray.GetDirection() * tmin;
    lqc::Vector3f maxpoint = ray.GetPosition() + ray.GetDirection() * tmax;
    color = Shade(m_aabb, minpoint, maxpoint, normal, ray.GetPosition(), ray.GetDirection(), i, j);
    rays_traced += 1;
  }
  return color;
}

void RendererCPU::SetModelVolumeAndTransferFunction(vr::Volume* ivolume, vr::TransferFunction* itransfer_function)
{
  m_voleval.SetModelAndTransferFunction(ivolume, itransfer_function);
}

bool RendererCPU::IsRenderErrorTest()
{
  return m_voleval.IsCurrentErrorTest();
}

void RendererCPU::TraceToTestErrorRay(int x, int y)
{
  int w = Viewer::Instance()->m_CurrentWidth, h = Viewer::Instance()->m_CurrentHeight;
  int curmethod;
  lqc::Vector4f pixel_color;

  curmethod = m_voleval.m_current_method;

  m_voleval.m_current_method = VolumeEvaluator::E_TESTS;
  pixel_color = Trace(rt::RTRay(m_camera.GetPositionCastRay((float)x, (float)y)
    , m_camera.CastRay((float)x, (float)y)), 1, x, y);

  m_pixels[x * 4 + y*w * 4] = 1.0f;
  m_pixels[x * 4 + y*w * 4 + 1] = 0.0f;
  m_pixels[x * 4 + y*w * 4 + 2] = 0.0f;
  m_pixels[x * 4 + y*w * 4 + 3] = 1.0f;

  Display();

  m_voleval.m_current_method = curmethod;
}

void RendererCPU::PrintPixelsDifference(float pixels[], int pwidth, int pheight)
{
  for (int x = 0; x < pwidth; x++) {
    for (int y = 0; y < pheight; y++) {
      int i = x * 4 + y*pwidth * 4;
      m_pixels[i] = abs(m_pixels[i] - pixels[i]);
      m_pixels[i + 1] = abs(m_pixels[i + 1] - pixels[i + 1]);
      m_pixels[i + 2] = abs(m_pixels[i + 2] - pixels[i + 2]);
      m_pixels[i + 3] = abs(m_pixels[i + 3] - pixels[i + 3]);
    }
  }
  Display();
}

void RendererCPU::Display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glDrawPixels(Viewer::Instance()->m_CurrentWidth, Viewer::Instance()->m_CurrentHeight, GL_RGBA, GL_FLOAT, m_pixels);
}

void RendererCPU::RayTracer(int rtwidth, int rtheight)
{
  for (int j = 0; j < rtheight; j++) {
    for (int i = 0; i < rtwidth; i++) {
      lqc::Vector4f pixel_color = Trace(rt::RTRay(m_camera.GetPositionCastRay((float)i, (float)j)
        , m_camera.CastRay((float)i, (float)j)), 1, i, j);
      m_pixels[i * 4 + j*rtwidth * 4] = pixel_color.x;
      m_pixels[i * 4 + j*rtwidth * 4 + 1] = pixel_color.y;
      m_pixels[i * 4 + j*rtwidth * 4 + 2] = pixel_color.z;
      m_pixels[i * 4 + j*rtwidth * 4 + 3] = pixel_color.w;
    }
  }
}

lqc::Vector4f RendererCPU::Shade(rt::RTObject* object, lqc::Vector3f minpoint, lqc::Vector3f maxpoint, lqc::Vector3f normal, lqc::Vector3f eye, lqc::Vector3f eyedirection, int i, int j)
{
  lqc::Vector4f color;

  if (minpoint != maxpoint) {
    float StepSize = (lqc::Distance(minpoint, maxpoint) / m_voleval.m_samples);
    lqc::Vector3f Step = lqc::Vector3f::Normalize(eyedirection)*StepSize;
    lqc::Vector3f pos = minpoint;
    glm::dvec4 dst;
    m_voleval.EvaluateIntegral(&dst, m_voleval.m_volume, minpoint, maxpoint, NULL);

    //m_inner_integral_minor_step = MIN (m_inner_integral_minor_step, m_voleval.m_inner_minstep);
    //m_outer_integral_minor_step = MIN (m_outer_integral_minor_step, m_voleval.m_outer_minstep);

    color = lqc::Vector4f((float)dst.x, (float)dst.y, (float)dst.z, (float)dst.w);
  }

  return color;
}

void* RendererCPU::RayTracerPThreadCall(void* input)
{
  RayTracerPThread* rt = (RayTracerPThread*)input;
  for (int j = rt->starter[1]; j < rt->rtheight; j += rt->j_iterator) {
    for (int i = rt->starter[0]; i < rt->rtwidth; i += rt->i_iterator) {
      lqc::Vector4f pixel_color = rt->cpu_viewer->Trace(rt::RTRay(rt->cpu_viewer->m_camera.GetPositionCastRay((float)i, (float)j)
        , rt->cpu_viewer->m_camera.CastRay((float)i, (float)j)), 1, i, j);
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4] = pixel_color.x;
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4 + 1] = pixel_color.y;
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4 + 2] = pixel_color.z;
      rt->cpu_viewer->m_pixels[i * 4 + j*rt->rtwidth * 4 + 3] = pixel_color.w;
    }
  }
  return NULL;
}


void RendererCPU::SaveInitRtCameraState(std::string filename)
{
  std::ofstream state_file;
  state_file.open(filename);
  if (state_file.is_open()) {
    state_file << "CPU\n";
    state_file << 0 << "\n";

    state_file << m_camera.m_eye.x << " "
      << m_camera.m_eye.y << " "
      << m_camera.m_eye.z << "\n";

    state_file << m_camera.m_z_e.x << " "
      << m_camera.m_z_e.y << " "
      << m_camera.m_z_e.z << "\n";

    state_file << m_camera.m_y_e.x << " "
      << m_camera.m_y_e.y << " "
      << m_camera.m_y_e.z << "\n";

    state_file << m_camera.m_x_e.x << " "
      << m_camera.m_x_e.y << " "
      << m_camera.m_x_e.z << "\n";

    state_file.close();
  }
}

void RendererCPU::LoadInitRtCameraState(std::string filename)
{
  std::ifstream state_file;
  state_file.open(filename);
  if (state_file.is_open()) {
    std::string method_name;
    std::getline(state_file, method_name);
    int view_method;
    state_file >> view_method;

    state_file >> m_camera.m_eye.x
      >> m_camera.m_eye.y
      >> m_camera.m_eye.z;

    state_file >> m_camera.m_z_e.x
      >> m_camera.m_z_e.y
      >> m_camera.m_z_e.z;

    state_file >> m_camera.m_y_e.x
      >> m_camera.m_y_e.y
      >> m_camera.m_y_e.z;

    state_file >> m_camera.m_x_e.x
      >> m_camera.m_x_e.y
      >> m_camera.m_x_e.z;

    state_file.close();
  }
}