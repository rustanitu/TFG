/*!
\file CPURenderer.h
\brief CPU renderer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef CPURENDERER_H
#define CPURENDERER_H

#include <pthread.h>

#include <atfg/Volume.h>
#include <atfg/TransferFunction.h>
#include <raytracer/Camera/RTCamera.h>
#include <raytracer/Shapes/RTAABB.h>
#include <math/Vector3.h>

#include "VolumeEvaluator.h"

class RendererCPU
{
private:
  /*! Auxiliar struct used in Ray Tracer when pthreads is enable.*/
  typedef struct RayTracerPThread
  {
    int rtwidth, rtheight;
    int* starter;
    RendererCPU* cpu_viewer;
    int i_iterator;
    int j_iterator;
  } RayTracerPThread;

public:
  /*! Constructor.*/
  RendererCPU ();
  /*! Destructor.*/
  ~RendererCPU ();

  /*! Render function.*/
  bool Render (int Width, int Height);

  /*! Reset the attributes of the camera.*/
  void ResetCamera (int width, int height);
  /*! Destroy all the objects of Renderer.*/
  void Destroy ();

  void ResetAABB (vr::Volume* volume);
  void SetAABB (vr::Volume* volume, float x0, float y0, float z0, float x1, float y1, float z1);

  /*! Volume Evaluator instance used in CPU rendering.*/
  VolumeEvaluator m_voleval;
 
  /*! Camera used in CPU rendering.*/
  rt::RTCamera m_camera;

  /*! AABB box used to intersect the rays with the volume.*/
  rt::RTAABB* m_aabb;

  /*! rgba values generated at Render Function.*/
  float* m_pixels;

  bool m_pthread_on;
  int m_num_of_threads;

  glm::vec3 GetMinAnchor ();
  glm::vec3 GetMaxAnchor ();

  /*! Trace a ray.*/
  glm::vec4 Trace (rt::RTRay ray, int  depth, int i, int j);

  void SetModelVolumeAndTransferFunction (vr::Volume* ivolume, vr::TransferFunction* itransfer_function);

  bool IsRenderErrorTest ();
  void PrintErrorTestResults ()
  {
    m_voleval.PrintErrorTestResults ();
  }

  void TraceToTestErrorRay (int x, int y);
  void PrintPixelsDifference (float pixels[], int pwidth, int pheight);

  void SaveInitRtCameraState (std::string filename);
  void LoadInitRtCameraState (std::string filename);

  void Display ();

  bool m_scaletomaxanchorvalue;
  float m_max_aabb_anchor_value;
  int rays_traced;
private:
  float m_aabb_min_x;
  float m_aabb_min_y;
  float m_aabb_min_z;
  float m_aabb_max_x;
  float m_aabb_max_y;
  float m_aabb_max_z;

  rt::Struct_RTCamera_init m_cam_init_st;

  void RayTracer (int width, int height);
  glm::vec4 Shade (rt::RTObject* object, glm::vec3 minpoint, glm::vec3 maxpoint, glm::vec3 normal, glm::vec3 eye, glm::vec3 eyedirection, int i, int j);
  static void* RayTracerPThreadCall (void* input);
  pthread_t m_t_even;
  pthread_t m_t_even_r2;
  pthread_t m_t_odd;
  pthread_t m_t_odd_r2;

  glm::vec3 m_aabbmin;
  glm::vec3 m_aabbmax;


};


#endif