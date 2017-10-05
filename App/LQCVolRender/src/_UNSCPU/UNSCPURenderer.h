/*!
\file CPURenderer.h
\brief CPU renderer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef UNSCPURENDERER_H
#define UNSCPURENDERER_H

#include <pthread.h>

#include <volrend/Volume.h>
#include <volrend/TransferFunction.h>
#include <raytracer/Camera/RTCamera.h>
#include <raytracer/Shapes/RTAABB.h>
#include <math/Vector3.h>

#include "../CPU/VolumeEvaluator.h"

class UNSCPURenderer
{
private:
  /*! Auxiliar struct used in Ray Tracer when pthreads is enable.*/
  typedef struct RayTracerPThread
  {
    int rtwidth, rtheight;
    int* starter;
    UNSCPURenderer* cpu_viewer;
    int i_iterator;
    int j_iterator;
  }RayTracerPThread;

public:
  /*! Constructor.*/
  UNSCPURenderer ();
  /*! Destructor.*/
  ~UNSCPURenderer ();

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

  lqc::Vector3f GetMinAnchor ();
  lqc::Vector3f GetMaxAnchor ();

  /*! Trace a ray.*/
  lqc::Vector4f Trace (rt::RTRay ray, int  depth, int i, int j);

  void SetModelVolumeAndTransferFunction (vr::Volume* ivolume, vr::TransferFunction* itransfer_function);

  bool m_scaletomaxanchorvalue;
  float m_max_aabb_anchor_value;
private:
  float m_aabb_min_x;
  float m_aabb_min_y;
  float m_aabb_min_z;
  float m_aabb_max_x;
  float m_aabb_max_y;
  float m_aabb_max_z;

  float m_camera_eye_initial_x;
  float m_camera_eye_initial_y;
  float m_camera_eye_initial_z;

  void Display ();
  void RayTracer (int width, int height);
  lqc::Vector4f Shade (rt::RTObject* object, lqc::Vector3f minpoint, lqc::Vector3f maxpoint, lqc::Vector3f normal, lqc::Vector3f eye, lqc::Vector3f eyedirection, int i, int j);
  static void* RayTracerPThreadCall (void* input);
  pthread_t m_t_even;
  pthread_t m_t_even_r2;
  pthread_t m_t_odd;
  pthread_t m_t_odd_r2;

  lqc::Vector3f m_aabbmin;
  lqc::Vector3f m_aabbmax;
};


#endif